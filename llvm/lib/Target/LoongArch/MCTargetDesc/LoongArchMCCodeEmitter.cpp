//=- LoongArchMCCodeEmitter.cpp - Convert LoongArch code to machine code --===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the LoongArchMCCodeEmitter class.
//
//===----------------------------------------------------------------------===//

#include "LoongArchFixupKinds.h"
#include "MCTargetDesc/LoongArchBaseInfo.h"
#include "MCTargetDesc/LoongArchMCExpr.h"
#include "MCTargetDesc/LoongArchMCTargetDesc.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/MC/MCInstBuilder.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/EndianStream.h"

using namespace llvm;

#define DEBUG_TYPE "mccodeemitter"

namespace {
class LoongArchMCCodeEmitter : public MCCodeEmitter {
  LoongArchMCCodeEmitter(const LoongArchMCCodeEmitter &) = delete;
  void operator=(const LoongArchMCCodeEmitter &) = delete;
  MCContext &Ctx;
  MCInstrInfo const &MCII;

public:
  LoongArchMCCodeEmitter(MCContext &ctx, MCInstrInfo const &MCII)
      : Ctx(ctx), MCII(MCII) {}

  ~LoongArchMCCodeEmitter() override {}

  void encodeInstruction(const MCInst &MI, SmallVectorImpl<char> &CB,
                         SmallVectorImpl<MCFixup> &Fixups,
                         const MCSubtargetInfo &STI) const override;

  template <unsigned Opc>
  void expandToVectorLDI(const MCInst &MI, SmallVectorImpl<char> &CB,
                         SmallVectorImpl<MCFixup> &Fixups,
                         const MCSubtargetInfo &STI) const;

  void expandAddTPRel(const MCInst &MI, SmallVectorImpl<char> &CB,
                      SmallVectorImpl<MCFixup> &Fixups,
                      const MCSubtargetInfo &STI) const;

  /// TableGen'erated function for getting the binary encoding for an
  /// instruction.
  uint64_t getBinaryCodeForInstr(const MCInst &MI,
                                 SmallVectorImpl<MCFixup> &Fixups,
                                 const MCSubtargetInfo &STI) const;

  /// Return binary encoding of operand. If the machine operand requires
  /// relocation, record the relocation and return zero.
  unsigned getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const;

  /// Return binary encoding of an immediate operand specified by OpNo.
  /// The value returned is the value of the immediate minus 1.
  /// Note that this function is dedicated to specific immediate types,
  /// e.g. uimm2_plus1.
  unsigned getImmOpValueSub1(const MCInst &MI, unsigned OpNo,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const;

  /// Return binary encoding of an immediate operand specified by OpNo.
  /// The value returned is the value of the immediate shifted right
  //  arithmetically by N.
  /// Note that this function is dedicated to specific immediate types,
  /// e.g. simm14_lsl2, simm16_lsl2, simm21_lsl2 and simm26_lsl2.
  template <unsigned N>
  unsigned getImmOpValueAsr(const MCInst &MI, unsigned OpNo,
                            SmallVectorImpl<MCFixup> &Fixups,
                            const MCSubtargetInfo &STI) const {
    const MCOperand &MO = MI.getOperand(OpNo);
    if (MO.isImm()) {
      unsigned Res = MI.getOperand(OpNo).getImm();
      assert((Res & ((1U << N) - 1U)) == 0 && "lowest N bits are non-zero");
      return Res >> N;
    }
    return getExprOpValue(MI, MO, Fixups, STI);
  }

  unsigned getExprOpValue(const MCInst &MI, const MCOperand &MO,
                          SmallVectorImpl<MCFixup> &Fixups,
                          const MCSubtargetInfo &STI) const;
};
} // end namespace

unsigned
LoongArchMCCodeEmitter::getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                                          SmallVectorImpl<MCFixup> &Fixups,
                                          const MCSubtargetInfo &STI) const {
  if (MO.isReg())
    return Ctx.getRegisterInfo()->getEncodingValue(MO.getReg());

  if (MO.isImm())
    return static_cast<unsigned>(MO.getImm());

  // MO must be an Expr.
  assert(MO.isExpr());
  return getExprOpValue(MI, MO, Fixups, STI);
}

unsigned
LoongArchMCCodeEmitter::getImmOpValueSub1(const MCInst &MI, unsigned OpNo,
                                          SmallVectorImpl<MCFixup> &Fixups,
                                          const MCSubtargetInfo &STI) const {
  return MI.getOperand(OpNo).getImm() - 1;
}

class LoongArchInstAddressHintMCExpr : public MCExpr {
public:
  size_t InstAddress;
  bool StubActivated;
  const MCSymbol *TargetSymbol;
  int RegisterIndex;
  LoongArchInstAddressHintMCExpr(size_t Address, bool StubActivated = false)
      : MCExpr(MCExpr::ExprKind::Constant, SMLoc()), InstAddress(Address),
        StubActivated(StubActivated), TargetSymbol(nullptr), RegisterIndex(0) {}
  void activateHintStub(const LoongArchMCExpr *LaExpr, int TmpRegIndex) {
    StubActivated = true;
    const auto *SubExpr = LaExpr->getSubExpr();
    auto &Symbol = ((const MCSymbolRefExpr *)SubExpr)->getSymbol();
    TargetSymbol = &Symbol;
    RegisterIndex = TmpRegIndex;
  }
  bool matchStub(const LoongArchMCExpr *LaExpr, int TmpRegIndex) const {
    if (!StubActivated)
      return false;
    const auto *const SubExpr = LaExpr->getSubExpr();
    auto &Symbol = ((const MCSymbolRefExpr *)SubExpr)->getSymbol();
    return TargetSymbol == &Symbol && RegisterIndex == TmpRegIndex;
  }
};

unsigned emitRelocSequenceHi(bool UseGot, MCContext &Ctx, const MCExpr *Expr,
                             const MCInst &MI, const MCOperand &MO,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI,
                             llvm::LoongArch::Fixups SymbolFixupType =
                                 LoongArch::fixup_loongarch_sop_push_gprel) {
  auto *HintFixup = Fixups.end() - 1;
  if (HintFixup->getKind() == (MCFixupKind)LoongArch::fixup_loongarch_relax) {
    auto *HintExpr = reinterpret_cast<LoongArchInstAddressHintMCExpr *>(const_cast<MCExpr *>(HintFixup->getValue()));
    HintExpr->activateHintStub((const LoongArchMCExpr *)Expr,
                               MI.getOperand(0).getReg().id());
  } else {
    report_fatal_error("Address hint is missing");
  }
  if (UseGot) {
    auto *GotBase = Ctx.getOrCreateSymbol("_GLOBAL_OFFSET_TABLE_");

    const auto *GotRefExpr = MCSymbolRefExpr::create(
        (const MCSymbol *)GotBase,
        (MCSymbolRefExpr::VariantKind)LoongArchMCExpr::VK_LoongArch_ABS_LO12,
        Ctx, MI.getLoc());

    Fixups.push_back(MCFixup::create(
        0,
        MCBinaryExpr::createAdd(GotRefExpr, MCConstantExpr::create(0x800, Ctx),
                                Ctx),
        MCFixupKind(LoongArch::fixup_loongarch_sop_push_pcrel), MI.getLoc()));

    Fixups.push_back(
        MCFixup::create(0, Expr, MCFixupKind(SymbolFixupType), MI.getLoc()));

  } else {
    Fixups.push_back(MCFixup::create(
        0, MCConstantExpr::create(0x800, Ctx),
        MCFixupKind(LoongArch::fixup_loongarch_sop_push_abs), MI.getLoc()));

    Fixups.push_back(MCFixup::create(
        0, Expr, MCFixupKind(LoongArch::fixup_loongarch_sop_push_pcrel),
        MI.getLoc()));
  }

  Fixups.push_back(MCFixup::create(
      0, MCConstantExpr::create(0x0, Ctx),
      MCFixupKind(LoongArch::fixup_loongarch_sop_add), MI.getLoc()));

  Fixups.push_back(MCFixup::create(
      0, MCConstantExpr::create(0xC, Ctx),
      MCFixupKind(LoongArch::fixup_loongarch_sop_push_abs), MI.getLoc()));

  Fixups.push_back(MCFixup::create(
      0, MCConstantExpr::create(0x0, Ctx),
      MCFixupKind(LoongArch::fixup_loongarch_sop_sr), MI.getLoc()));

  Fixups.push_back(MCFixup::create(
      0, MCConstantExpr::create(0x0, Ctx),
      MCFixupKind(LoongArch::fixup_loongarch_sop_pop32_s_5_20), MI.getLoc()));
  return 0;
}

unsigned emitRelocSequenceLo(bool UseGot, MCContext &Ctx, const MCExpr *Expr,
                             const MCInst &MI, const MCOperand &MO,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI,
                             llvm::LoongArch::Fixups SymbolFixupType =
                                 LoongArch::fixup_loongarch_sop_push_gprel) {
  auto *HintFixup = Fixups.end() - 1;
  size_t LoAddress = 0;
  if (HintFixup->getKind() == (MCFixupKind)LoongArch::fixup_loongarch_relax) {
    auto *HintExpr = ((const LoongArchInstAddressHintMCExpr *)HintFixup->getValue());
    LoAddress = HintExpr->InstAddress;
  } else {
    report_fatal_error("Lo Address hint is missing");
  }

  const size_t InvalidAddress = (size_t)-1;
  size_t PcadduAddress = InvalidAddress;
  for (int i = Fixups.size() - 1; i >= 0; i--) {
    auto &CurrentFixup = Fixups[i];
    if (CurrentFixup.getKind() == (MCFixupKind)LoongArch::fixup_loongarch_relax) {
      auto *HintExpr = (const LoongArchInstAddressHintMCExpr *)CurrentFixup.getValue();
      if (HintExpr->matchStub((const LoongArchMCExpr *)Expr,
                              MI.getOperand(1).getReg().id())) {
        PcadduAddress = HintExpr->InstAddress;
        break;
      }
    }
  }
  if (PcadduAddress == InvalidAddress) {
    report_fatal_error("Unable to match lo12 inst with hi20 inst");
  }
  auto InstOffset = LoAddress - PcadduAddress;
  if (UseGot) {
    auto *GotBase = Ctx.getOrCreateSymbol("_GLOBAL_OFFSET_TABLE_");

    const auto *GotRefExpr = MCSymbolRefExpr::create(
        (const MCSymbol *)GotBase,
        (MCSymbolRefExpr::VariantKind)LoongArchMCExpr::VK_LoongArch_ABS_LO12,
        Ctx, MI.getLoc());

    Fixups.push_back(MCFixup::create(
        0,
        MCBinaryExpr::createAdd(
            GotRefExpr, MCConstantExpr::create(0x800 + InstOffset, Ctx), Ctx),
        MCFixupKind(LoongArch::fixup_loongarch_sop_push_pcrel), MI.getLoc()));

    Fixups.push_back(
        MCFixup::create(0, Expr, MCFixupKind(SymbolFixupType), MI.getLoc()));

  } else {
    Fixups.push_back(MCFixup::create(
        0, MCConstantExpr::create(0x800 + InstOffset, Ctx),
        MCFixupKind(LoongArch::fixup_loongarch_sop_push_abs), MI.getLoc()));
    Fixups.push_back(MCFixup::create(
        0, Expr, MCFixupKind(LoongArch::fixup_loongarch_sop_push_pcrel),
        MI.getLoc()));
  }

  Fixups.push_back(MCFixup::create(
      0, MCConstantExpr::create(0x0, Ctx),
      MCFixupKind(LoongArch::fixup_loongarch_sop_add), MI.getLoc()));

  Fixups.push_back(MCFixup::create(
      0, MCConstantExpr::create(0xFFF, Ctx),
      MCFixupKind(LoongArch::fixup_loongarch_sop_push_abs), MI.getLoc()));

  Fixups.push_back(MCFixup::create(
      0, MCConstantExpr::create(0x0, Ctx),
      MCFixupKind(LoongArch::fixup_loongarch_sop_and), MI.getLoc()));

  Fixups.push_back(MCFixup::create(
      0, MCConstantExpr::create(0x800, Ctx),
      MCFixupKind(LoongArch::fixup_loongarch_sop_push_abs), MI.getLoc()));

  Fixups.push_back(MCFixup::create(
      0, MCConstantExpr::create(0x0, Ctx),
      MCFixupKind(LoongArch::fixup_loongarch_sop_sub), MI.getLoc()));

  Fixups.push_back(MCFixup::create(
      0, MCConstantExpr::create(0x0, Ctx),
      MCFixupKind(LoongArch::fixup_loongarch_sop_pop32_s_10_12), MI.getLoc()));
  return 0;
}

unsigned
LoongArchMCCodeEmitter::getExprOpValue(const MCInst &MI, const MCOperand &MO,
                                       SmallVectorImpl<MCFixup> &Fixups,
                                       const MCSubtargetInfo &STI) const {
  assert(MO.isExpr() && "getExprOpValue expects only expressions");
  bool RelaxCandidate = false;
  bool EnableRelax = STI.hasFeature(LoongArch::FeatureRelax);
  const MCExpr *Expr = MO.getExpr();
  MCExpr::ExprKind Kind = Expr->getKind();
  LoongArch::Fixups FixupKind = LoongArch::fixup_loongarch_invalid;
  if (Kind == MCExpr::Target) {
    const LoongArchMCExpr *LAExpr = cast<LoongArchMCExpr>(Expr);

    RelaxCandidate = LAExpr->getRelaxHint();
    switch (LAExpr->getKind()) {
    case LoongArchMCExpr::VK_LoongArch_None:
    case LoongArchMCExpr::VK_LoongArch_Invalid:
      llvm_unreachable("Unhandled fixup kind!");
    case LoongArchMCExpr::VK_LoongArch_TLS_LE_ADD_R:
      llvm_unreachable("VK_LoongArch_TLS_LE_ADD_R should not represent an "
                       "instruction operand");
    case LoongArchMCExpr::VK_LoongArch_B16:
      FixupKind = LoongArch::fixup_loongarch_b16;
      break;
    case LoongArchMCExpr::VK_LoongArch_B21:
      FixupKind = LoongArch::fixup_loongarch_b21;
      break;
    case LoongArchMCExpr::VK_LoongArch_B26:
    case LoongArchMCExpr::VK_LoongArch_CALL:
    case LoongArchMCExpr::VK_LoongArch_CALL_PLT:
      FixupKind = LoongArch::fixup_loongarch_b26;
      break;
    case LoongArchMCExpr::VK_LoongArch_ABS_HI20:
      FixupKind = LoongArch::fixup_loongarch_abs_hi20;
      break;
    case LoongArchMCExpr::VK_LoongArch_ABS_LO12:
      FixupKind = LoongArch::fixup_loongarch_abs_lo12;
      break;
    case LoongArchMCExpr::VK_LoongArch_ABS64_LO20:
      FixupKind = LoongArch::fixup_loongarch_abs64_lo20;
      break;
    case LoongArchMCExpr::VK_LoongArch_ABS64_HI12:
      FixupKind = LoongArch::fixup_loongarch_abs64_hi12;
      break;
    case LoongArchMCExpr::VK_LoongArch_PCALA_HI20: {

      FixupKind = LoongArch::fixup_loongarch_pcala_hi20;
      if (STI.getTargetTriple().isLoongArch32Reduced())
        return emitRelocSequenceHi(false, Ctx, Expr, MI, MO, Fixups, STI);
      break;
    }
    case LoongArchMCExpr::VK_LoongArch_PCALA_LO12: {

      FixupKind = LoongArch::fixup_loongarch_pcala_lo12;
      if (STI.getTargetTriple().isLoongArch32Reduced()) {
        return emitRelocSequenceLo(false, Ctx, Expr, MI, MO, Fixups, STI);
      }

      break;
    }
    case LoongArchMCExpr::VK_LoongArch_PCALA64_LO20:
      FixupKind = LoongArch::fixup_loongarch_pcala64_lo20;
      break;
    case LoongArchMCExpr::VK_LoongArch_PCALA64_HI12:
      FixupKind = LoongArch::fixup_loongarch_pcala64_hi12;
      break;
    case LoongArchMCExpr::VK_LoongArch_GOT_PC_HI20: {
      FixupKind = LoongArch::fixup_loongarch_got_pc_hi20;

      if (STI.getTargetTriple().isLoongArch32Reduced()) {
        FixupKind = LoongArch::fixup_loongarch_pcala64_hi12;
        return emitRelocSequenceHi(true, Ctx, Expr, MI, MO, Fixups, STI);
      }

      break;
    }
    case LoongArchMCExpr::VK_LoongArch_GOT_PC_LO12: {
      FixupKind = LoongArch::fixup_loongarch_got_pc_lo12;
      if (STI.getTargetTriple().isLoongArch32Reduced()) {
        FixupKind = LoongArch::fixup_loongarch_got_pc_lo12;
        return emitRelocSequenceLo(true, Ctx, Expr, MI, MO, Fixups, STI);
      }
      break;
    }
    case LoongArchMCExpr::VK_LoongArch_GOT64_PC_LO20:
      FixupKind = LoongArch::fixup_loongarch_got64_pc_lo20;
      break;
    case LoongArchMCExpr::VK_LoongArch_GOT64_PC_HI12:
      FixupKind = LoongArch::fixup_loongarch_got64_pc_hi12;
      break;
    case LoongArchMCExpr::VK_LoongArch_GOT_HI20:
      FixupKind = LoongArch::fixup_loongarch_got_hi20;
      break;
    case LoongArchMCExpr::VK_LoongArch_GOT_LO12:
      FixupKind = LoongArch::fixup_loongarch_got_lo12;
      break;
    case LoongArchMCExpr::VK_LoongArch_GOT64_LO20:
      FixupKind = LoongArch::fixup_loongarch_got64_lo20;
      break;
    case LoongArchMCExpr::VK_LoongArch_GOT64_HI12:
      FixupKind = LoongArch::fixup_loongarch_got64_hi12;
      break;
    // TLS LE uses lui.w and don't need to fix pcaddu12 offset
    case LoongArchMCExpr::VK_LoongArch_TLS_LE_HI20:
      FixupKind = LoongArch::fixup_loongarch_tls_le_hi20;
      break;
    case LoongArchMCExpr::VK_LoongArch_TLS_LE_LO12:
      FixupKind = LoongArch::fixup_loongarch_tls_le_lo12;
      break;
    case LoongArchMCExpr::VK_LoongArch_TLS_LE64_LO20:
      FixupKind = LoongArch::fixup_loongarch_tls_le64_lo20;
      break;
    case LoongArchMCExpr::VK_LoongArch_TLS_LE64_HI12:
      FixupKind = LoongArch::fixup_loongarch_tls_le64_hi12;
      break;
    case LoongArchMCExpr::VK_LoongArch_TLS_IE_PC_HI20: {

      FixupKind = LoongArch::fixup_loongarch_tls_ie_pc_hi20;
      if (STI.getTargetTriple().isLoongArch32Reduced()) {
        Fixups.push_back(
            MCFixup::create(0, Expr, MCFixupKind(FixupKind), MI.getLoc()));
        return emitRelocSequenceHi(true, Ctx, Expr, MI, MO, Fixups, STI,
                                   LoongArch::fixup_loongarch_sop_push_tls_ie);
      }
      break;
    }
    case LoongArchMCExpr::VK_LoongArch_TLS_IE_PC_LO12: {

      FixupKind = LoongArch::fixup_loongarch_tls_ie_pc_lo12;
      if (STI.getTargetTriple().isLoongArch32Reduced()) {
        Fixups.push_back(
            MCFixup::create(0, Expr, MCFixupKind(FixupKind), MI.getLoc()));

        return emitRelocSequenceLo(true, Ctx, Expr, MI, MO, Fixups, STI,
                                   LoongArch::fixup_loongarch_sop_push_tls_ie);
      }
      break;
    }
    case LoongArchMCExpr::VK_LoongArch_TLS_IE64_PC_LO20:
      FixupKind = LoongArch::fixup_loongarch_tls_ie64_pc_lo20;
      break;
    case LoongArchMCExpr::VK_LoongArch_TLS_IE64_PC_HI12:
      FixupKind = LoongArch::fixup_loongarch_tls_ie64_pc_hi12;
      break;
    case LoongArchMCExpr::VK_LoongArch_TLS_IE_HI20:
      FixupKind = LoongArch::fixup_loongarch_tls_ie_hi20;
      break;
    case LoongArchMCExpr::VK_LoongArch_TLS_IE_LO12:
      FixupKind = LoongArch::fixup_loongarch_tls_ie_lo12;
      break;
    case LoongArchMCExpr::VK_LoongArch_TLS_IE64_LO20:
      FixupKind = LoongArch::fixup_loongarch_tls_ie64_lo20;
      break;
    case LoongArchMCExpr::VK_LoongArch_TLS_IE64_HI12:
      FixupKind = LoongArch::fixup_loongarch_tls_ie64_hi12;
      break;
    case LoongArchMCExpr::VK_LoongArch_TLS_LD_PC_HI20: {

      FixupKind = LoongArch::fixup_loongarch_tls_ld_pc_hi20;
      if (STI.getTargetTriple().isLoongArch32Reduced()) {
        Fixups.push_back(
            MCFixup::create(0, Expr, MCFixupKind(FixupKind), MI.getLoc()));
        return emitRelocSequenceHi(
            true, Ctx, Expr, MI, MO, Fixups, STI,
            LoongArch::fixup_loongarch_sop_push_tls_gd); // LD and GD share tls
                                                         // offset
      }

      break;
    }
    case LoongArchMCExpr::VK_LoongArch_TLS_LD_HI20:
      FixupKind = LoongArch::fixup_loongarch_tls_ld_hi20;
      break;
    case LoongArchMCExpr::VK_LoongArch_TLS_GD_PC_HI20: {

      FixupKind = LoongArch::fixup_loongarch_tls_gd_pc_hi20;
      if (STI.getTargetTriple().isLoongArch32Reduced()) {
        Fixups.push_back(
            MCFixup::create(0, Expr, MCFixupKind(FixupKind), MI.getLoc()));
        return emitRelocSequenceHi(true, Ctx, Expr, MI, MO, Fixups, STI,
                                   LoongArch::fixup_loongarch_sop_push_tls_gd);
      }

      break;
    }
    case LoongArchMCExpr::VK_LoongArch_TLS_GD_HI20:
      FixupKind = LoongArch::fixup_loongarch_tls_gd_hi20;
      break;
    case LoongArchMCExpr::VK_LoongArch_CALL36:
      FixupKind = LoongArch::fixup_loongarch_call36;
      break;
    case LoongArchMCExpr::VK_LoongArch_TLS_DESC_PC_HI20:
      FixupKind = LoongArch::fixup_loongarch_tls_desc_pc_hi20;
      break;
    case LoongArchMCExpr::VK_LoongArch_TLS_DESC_PC_LO12:
      FixupKind = LoongArch::fixup_loongarch_tls_desc_pc_lo12;
      break;
    case LoongArchMCExpr::VK_LoongArch_TLS_DESC64_PC_LO20:
      FixupKind = LoongArch::fixup_loongarch_tls_desc64_pc_lo20;
      break;
    case LoongArchMCExpr::VK_LoongArch_TLS_DESC64_PC_HI12:
      FixupKind = LoongArch::fixup_loongarch_tls_desc64_pc_hi12;
      break;
    case LoongArchMCExpr::VK_LoongArch_TLS_DESC_HI20:
      FixupKind = LoongArch::fixup_loongarch_tls_desc_hi20;
      break;
    case LoongArchMCExpr::VK_LoongArch_TLS_DESC_LO12:
      FixupKind = LoongArch::fixup_loongarch_tls_desc_lo12;
      break;
    case LoongArchMCExpr::VK_LoongArch_TLS_DESC64_LO20:
      FixupKind = LoongArch::fixup_loongarch_tls_desc64_lo20;
      break;
    case LoongArchMCExpr::VK_LoongArch_TLS_DESC64_HI12:
      FixupKind = LoongArch::fixup_loongarch_tls_desc64_hi12;
      break;
    case LoongArchMCExpr::VK_LoongArch_TLS_DESC_LD:
      FixupKind = LoongArch::fixup_loongarch_tls_desc_ld;
      break;
    case LoongArchMCExpr::VK_LoongArch_TLS_DESC_CALL:
      FixupKind = LoongArch::fixup_loongarch_tls_desc_call;
      break;
    case LoongArchMCExpr::VK_LoongArch_TLS_LE_HI20_R:
      FixupKind = LoongArch::fixup_loongarch_tls_le_hi20_r;
      break;
    case LoongArchMCExpr::VK_LoongArch_TLS_LE_LO12_R:
      FixupKind = LoongArch::fixup_loongarch_tls_le_lo12_r;
      break;
    case LoongArchMCExpr::VK_LoongArch_PCREL20_S2:
      FixupKind = LoongArch::fixup_loongarch_pcrel20_s2;
      break;
    case LoongArchMCExpr::VK_LoongArch_TLS_LD_PCREL20_S2:
      FixupKind = LoongArch::fixup_loongarch_tls_ld_pcrel20_s2;
      break;
    case LoongArchMCExpr::VK_LoongArch_TLS_GD_PCREL20_S2:
      FixupKind = LoongArch::fixup_loongarch_tls_gd_pcrel20_s2;
      break;
    case LoongArchMCExpr::VK_LoongArch_TLS_DESC_PCREL20_S2:
      FixupKind = LoongArch::fixup_loongarch_tls_desc_pcrel20_s2;
      break;
    }
  } else if (Kind == MCExpr::SymbolRef &&
             cast<MCSymbolRefExpr>(Expr)->getKind() ==
                 MCSymbolRefExpr::VK_None) {
    switch (MI.getOpcode()) {
    default:
      break;
    case LoongArch::BEQ:
    case LoongArch::BNE:
    case LoongArch::BLT:
    case LoongArch::BGE:
    case LoongArch::BLTU:
    case LoongArch::BGEU:
      FixupKind = LoongArch::fixup_loongarch_b16;
      break;
    case LoongArch::BEQZ:
    case LoongArch::BNEZ:
    case LoongArch::BCEQZ:
    case LoongArch::BCNEZ:
      FixupKind = LoongArch::fixup_loongarch_b21;
      break;
    case LoongArch::B:
    case LoongArch::BL:
      FixupKind = LoongArch::fixup_loongarch_b26;
      break;
    }
  }

  assert(FixupKind != LoongArch::fixup_loongarch_invalid &&
         "Unhandled expression!");
  Fixups.push_back(
      MCFixup::create(0, Expr, MCFixupKind(FixupKind), MI.getLoc()));

  // Emit an R_LARCH_RELAX if linker relaxation is enabled and LAExpr has relax
  // hint.
  if (EnableRelax && RelaxCandidate) {
    const MCConstantExpr *Dummy = MCConstantExpr::create(0, Ctx);
    Fixups.push_back(MCFixup::create(
        0, Dummy, MCFixupKind(LoongArch::fixup_loongarch_relax), MI.getLoc()));
  }

  return 0;
}

template <unsigned Opc>
void LoongArchMCCodeEmitter::expandToVectorLDI(
    const MCInst &MI, SmallVectorImpl<char> &CB,
    SmallVectorImpl<MCFixup> &Fixups, const MCSubtargetInfo &STI) const {
  int64_t Imm = MI.getOperand(1).getImm() & 0x3FF;
  switch (MI.getOpcode()) {
  case LoongArch::PseudoVREPLI_B:
  case LoongArch::PseudoXVREPLI_B:
    break;
  case LoongArch::PseudoVREPLI_H:
  case LoongArch::PseudoXVREPLI_H:
    Imm |= 0x400;
    break;
  case LoongArch::PseudoVREPLI_W:
  case LoongArch::PseudoXVREPLI_W:
    Imm |= 0x800;
    break;
  case LoongArch::PseudoVREPLI_D:
  case LoongArch::PseudoXVREPLI_D:
    Imm |= 0xC00;
    break;
  }
  MCInst TmpInst = MCInstBuilder(Opc).addOperand(MI.getOperand(0)).addImm(Imm);
  uint32_t Binary = getBinaryCodeForInstr(TmpInst, Fixups, STI);
  support::endian::write(CB, Binary, llvm::endianness::little);
}

void LoongArchMCCodeEmitter::expandAddTPRel(const MCInst &MI,
                                            SmallVectorImpl<char> &CB,
                                            SmallVectorImpl<MCFixup> &Fixups,
                                            const MCSubtargetInfo &STI) const {
  MCOperand Rd = MI.getOperand(0);
  MCOperand Rj = MI.getOperand(1);
  MCOperand Rk = MI.getOperand(2);
  MCOperand Symbol = MI.getOperand(3);
  assert(Symbol.isExpr() &&
         "Expected expression as third input to TP-relative add");

  const LoongArchMCExpr *Expr = dyn_cast<LoongArchMCExpr>(Symbol.getExpr());
  assert(Expr &&
         Expr->getKind() == LoongArchMCExpr::VK_LoongArch_TLS_LE_ADD_R &&
         "Expected %le_add_r relocation on TP-relative symbol");

  // Emit the correct %le_add_r relocation for the symbol.
  // TODO: Emit R_LARCH_RELAX for %le_add_r where the relax feature is enabled.
  Fixups.push_back(MCFixup::create(
      0, Expr, MCFixupKind(LoongArch::fixup_loongarch_tls_le_add_r),
      MI.getLoc()));

  // Emit a normal ADD instruction with the given operands.
  unsigned ADD = MI.getOpcode() == LoongArch::PseudoAddTPRel_D
                     ? LoongArch::ADD_D
                     : LoongArch::ADD_W;
  MCInst TmpInst =
      MCInstBuilder(ADD).addOperand(Rd).addOperand(Rj).addOperand(Rk);
  uint32_t Binary = getBinaryCodeForInstr(TmpInst, Fixups, STI);
  support::endian::write(CB, Binary, llvm::endianness::little);
}

void LoongArchMCCodeEmitter::encodeInstruction(
    const MCInst &MI, SmallVectorImpl<char> &CB,
    SmallVectorImpl<MCFixup> &Fixups, const MCSubtargetInfo &STI) const {

  const MCInstrDesc &Desc = MCII.get(MI.getOpcode());
  // Get byte count of instruction.
  unsigned Size = Desc.getSize();

  switch (MI.getOpcode()) {
  default:
    break;
  case LoongArch::PseudoVREPLI_B:
  case LoongArch::PseudoVREPLI_H:
  case LoongArch::PseudoVREPLI_W:
  case LoongArch::PseudoVREPLI_D:
    return expandToVectorLDI<LoongArch::VLDI>(MI, CB, Fixups, STI);
  case LoongArch::PseudoXVREPLI_B:
  case LoongArch::PseudoXVREPLI_H:
  case LoongArch::PseudoXVREPLI_W:
  case LoongArch::PseudoXVREPLI_D:
    return expandToVectorLDI<LoongArch::XVLDI>(MI, CB, Fixups, STI);
  case LoongArch::PseudoAddTPRel_W:
  case LoongArch::PseudoAddTPRel_D:
    return expandAddTPRel(MI, CB, Fixups, STI);
  }

  switch (Size) {
  default:
    llvm_unreachable("Unhandled encodeInstruction length!");
  case 4: {
    auto *InstHint = new LoongArchInstAddressHintMCExpr(CB.size(), false);
    auto HintIndex = Fixups.size();
    Fixups.push_back(MCFixup::create(
        0, InstHint, (MCFixupKind)LoongArch::fixup_loongarch_relax));

    uint32_t Bits = getBinaryCodeForInstr(MI, Fixups, STI);
    support::endian::write(CB, Bits, llvm::endianness::little);

    if (!InstHint->StubActivated) {
      Fixups.erase(Fixups.begin() + HintIndex);
      delete InstHint;
    }
    break;
  }
  }
}

MCCodeEmitter *llvm::createLoongArchMCCodeEmitter(const MCInstrInfo &MCII,
                                                  MCContext &Ctx) {
  return new LoongArchMCCodeEmitter(Ctx, MCII);
}

#include "LoongArchGenMCCodeEmitter.inc"
