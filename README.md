# The LLVM Compiler Infrastructure - With LA32R ISA support

This repository contains the source code for LLVM with extra support for LoongArch32 reduced (LA32R) ISA. LA32R is a simplified 32bit RISC ISA based on standard Loongson ISA for educational purposes. More details can be found in [LA32R Spec](https://www.loongson.cn/uploads/images/2023041918122813624.%E9%BE%99%E8%8A%AF%E6%9E%B6%E6%9E%8432%E4%BD%8D%E7%B2%BE%E7%AE%80%E7%89%88%E5%8F%82%E8%80%83%E6%89%8B%E5%86%8C_r1p03.pdf).

Compared to the standard version of the Loongson instruction set, LA32R only retains the basic integer, floating-point, and privileged instruction sets. Although the standard version of LLVM supports both 32-bit and 64 bit Loongson instruction sets, it includes parts that are not supported by LA32R.

The NCSCCS competition organizer provides a GCC compiler suitable for LA32R. However, due to personal reasons, I don't like GCC😅, so I tried to adapt LLVM to the LA32R instruction set. 

Usage: Simply specify target triple "-target loongarch32r-pc-gnusf". Currently only supports ilp32s ABI.

```
clang -target loongarch32r-pc-gnusf file.c <...>
```

---

This repository contains the source code for LLVM, a toolkit for the
construction of highly optimized compilers, optimizers, and run-time
environments.

The LLVM project has multiple components. The core of the project is
itself called "LLVM". This contains all of the tools, libraries, and header
files needed to process intermediate representations and convert them into
object files. Tools include an assembler, disassembler, bitcode analyzer, and
bitcode optimizer.

C-like languages use the [Clang](https://clang.llvm.org/) frontend. This
component compiles C, C++, Objective-C, and Objective-C++ code into LLVM bitcode
-- and from there into object files, using LLVM.

Other components include:
the [libc++ C++ standard library](https://libcxx.llvm.org),
the [LLD linker](https://lld.llvm.org), and more.

