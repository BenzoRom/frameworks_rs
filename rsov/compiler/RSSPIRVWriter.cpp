/*
 * Copyright 2016, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "RSSPIRVWriter.h"

#include "SPIRVModule.h"
#include "bcinfo/MetadataExtractor.h"

#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/Triple.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/SPIRV.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO.h"

#include "GlobalMergePass.h"
#include "InlinePreparationPass.h"
#include "ReflectionPass.h"
#include "RemoveNonkernelsPass.h"

#include <fstream>
#include <sstream>

#define DEBUG_TYPE "rs2spirv-writer"

using namespace llvm;
using namespace SPIRV;

namespace llvm {
FunctionPass *createPromoteMemoryToRegisterPass();
}

namespace rs2spirv {

static cl::opt<std::string> WrapperOutputFile("wo",
                                              cl::desc("Wrapper output file"),
                                              cl::value_desc("filename.spt"));

static void HandleTargetTriple(Module &M) {
  Triple TT(M.getTargetTriple());
  auto Arch = TT.getArch();

  StringRef NewTriple;
  switch (Arch) {
  default:
    llvm_unreachable("Unrecognized architecture");
    break;
  case Triple::arm:
    NewTriple = "spir-unknown-unknown";
    break;
  case Triple::aarch64:
    NewTriple = "spir64-unknown-unknown";
    break;
  case Triple::spir:
  case Triple::spir64:
    DEBUG(dbgs() << "!!! Already a spir triple !!!\n");
  }

  DEBUG(dbgs() << "New triple:\t" << NewTriple << "\n");
  M.setTargetTriple(NewTriple);
}

void addPassesForRS2SPIRV(llvm::legacy::PassManager &PassMgr,
                          bcinfo::MetadataExtractor &Extractor) {
  PassMgr.add(createInlinePreparationPass(Extractor));
  PassMgr.add(createAlwaysInlinerPass());
  PassMgr.add(createRemoveNonkernelsPass(Extractor));
  // Delete unreachable globals.
  PassMgr.add(createGlobalDCEPass());
  // Remove dead debug info.
  PassMgr.add(createStripDeadDebugInfoPass());
  // Remove dead func decls.
  PassMgr.add(createStripDeadPrototypesPass());
  PassMgr.add(createGlobalMergePass());
  PassMgr.add(createPromoteMemoryToRegisterPass());
  PassMgr.add(createTransOCLMD());
  // TODO: investigate removal of OCLTypeToSPIRV pass.
  PassMgr.add(createOCLTypeToSPIRV());
  PassMgr.add(createSPIRVRegularizeLLVM());
  PassMgr.add(createSPIRVLowerConstExpr());
  PassMgr.add(createSPIRVLowerBool());
}

bool WriteSPIRV(Module *M, llvm::raw_ostream &OS, std::string &ErrMsg) {
  std::unique_ptr<SPIRVModule> BM(SPIRVModule::createSPIRVModule());

  HandleTargetTriple(*M);

  bcinfo::MetadataExtractor ME(M);
  if (!ME.extract()) {
    errs() << "Could not extract metadata\n";
    return false;
  }
  DEBUG(dbgs() << "Metadata extracted\n");

  llvm::legacy::PassManager PassMgr;
  addPassesForRS2SPIRV(PassMgr, ME);

  std::ofstream WrapperF;
  if (!WrapperOutputFile.empty()) {
    WrapperF.open(WrapperOutputFile, std::ios::trunc);
    if (!WrapperF.good()) {
      errs() << "Could not create/open file:\t" << WrapperOutputFile << "\n";
      return false;
    }
    DEBUG(dbgs() << "Wrapper output:\t" << WrapperOutputFile << "\n");
    PassMgr.add(createReflectionPass(WrapperF, ME));
  }

  PassMgr.add(createLLVMToSPIRV(BM.get()));
  PassMgr.run(*M);
  DEBUG(M->dump());

  if (BM->getError(ErrMsg) != SPIRVEC_Success)
    return false;

  OS << *BM;

  return true;
}

} // namespace rs2spirv
