//===-- Passes.td - flang OpenMP pass definition -----------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef FORTRAN_OPTIMIZER_OPENMP_PASSES
#define FORTRAN_OPTIMIZER_OPENMP_PASSES

include "mlir/Pass/PassBase.td"

def MapInfoFinalizationPass
    : Pass<"omp-map-info-finalization", "mlir::ModuleOp"> {
  let summary = "expands OpenMP MapInfo operations containing descriptors";
  let description = [{
    Expands MapInfo operations containing descriptor types into multiple
    MapInfo's for each pointer element in the descriptor that requires
    explicit individual mapping by the OpenMP runtime.
  }];
  let dependentDialects = ["mlir::omp::OpenMPDialect"];
}

def MapsForPrivatizedSymbolsPass
    : Pass<"omp-maps-for-privatized-symbols", "mlir::func::FuncOp"> {
  let summary = "Creates MapInfoOp instances for privatized symbols when needed";
  let description = [{
    Adds omp.map.info operations for privatized symbols on omp.target ops
    In certain situations, such as when an allocatable is privatized, its
    descriptor is needed in the alloc region of the privatizer. This results
    in the use of the descriptor inside the target region. As such, the
    descriptor then needs to be mapped. This pass adds such MapInfoOp operations.
  }];
  let dependentDialects = ["mlir::omp::OpenMPDialect"];
}

def MarkDeclareTargetPass
    : Pass<"omp-mark-declare-target", "mlir::ModuleOp"> {
  let summary = "Marks all functions called by an OpenMP declare target function as declare target";
  let dependentDialects = ["mlir::omp::OpenMPDialect"];
}

def FunctionFilteringPass : Pass<"omp-function-filtering"> {
  let summary = "Filters out functions intended for the host when compiling "
                "for the target device.";
  let dependentDialects = [
    "mlir::func::FuncDialect",
    "fir::FIROpsDialect"
  ];
}

#endif //FORTRAN_OPTIMIZER_OPENMP_PASSES
