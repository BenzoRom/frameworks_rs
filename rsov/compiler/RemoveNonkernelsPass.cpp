/*
 * Copyright 2017, The Android Open Source Project
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

#include "RemoveNonkernelsPass.h"

#include "llvm/ADT/StringSet.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"

#include "bcinfo/MetadataExtractor.h"

#define DEBUG_TYPE "rs2spirv-remove"

using namespace llvm;

namespace rs2spirv {

namespace {

class RemoveNonkernelsPass : public ModulePass {
  bcinfo::MetadataExtractor &ME;

public:
  static char ID;
  explicit RemoveNonkernelsPass(bcinfo::MetadataExtractor &Extractor)
      : ModulePass(ID), ME(Extractor) {}

  const char *getPassName() const override { return "RemoveNonkernelsPass"; }

  bool runOnModule(Module &M) override {
    DEBUG(dbgs() << "RemoveNonkernelsPass\n");
    DEBUG(M.dump());

    const size_t RSKernelNum = ME.getExportForEachSignatureCount();
    const char **RSKernelNames = ME.getExportForEachNameList();
    if (RSKernelNum == 0)
      DEBUG(dbgs() << "RemoveNonkernelsPass detected no kernel\n");

    StringSet<> KNames;
    for (size_t i = 0; i < RSKernelNum; ++i)
      KNames.insert(RSKernelNames[i]);

    std::vector<Function *> Functions;
    for (auto &F : M.functions()) {
      Functions.push_back(&F);
    }

    for (auto &F : Functions) {
      if (F->isDeclaration())
        continue;

      const StringRef FName = F->getName();

      if (KNames.count(FName) != 0)
        continue; // Skip kernels.

      F->replaceAllUsesWith(UndefValue::get((Type *)F->getType()));
      F->eraseFromParent();

      DEBUG(dbgs() << "Removed:\t" << FName << '\n');
    }

    // Return true, as the pass modifies module.
    DEBUG(M.dump());
    DEBUG(dbgs() << "Done removal\n");

    return true;
  }
};
} // namespace

char RemoveNonkernelsPass::ID = 0;

ModulePass *createRemoveNonkernelsPass(bcinfo::MetadataExtractor &ME) {
  return new RemoveNonkernelsPass(ME);
}

} // namespace rs2spirv
