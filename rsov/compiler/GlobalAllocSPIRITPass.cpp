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

#include "GlobalAllocSPIRITPass.h"

#include "spirit.h"
#include "transformer.h"

namespace android {
namespace spirit {

// Replacing calls to lowered accessors, e.g., __rsov_rsAllocationGetDimX
// which was created from rsAllocationGetDimX by replacing the allocation
// with an ID in an earlier LLVM pass (see GlobalAllocationPass.cpp),
// to access the global allocation metadata.
//
// For example, the source code may look like:
//
// rs_allocation g;
// ...
//    uint32_t foo = rsAllocationGetDimX(g);
//
// After the  GlobalAllocPass, it would look like the LLVM IR
// equivalent of:
//
//    uint32_t foo = __rsov_rsAllocationGetDimX(0);
//
// After that pass, g is removed, and references in intrinsics
// to g would be replaced with an assigned unique id (0 here), and
// rsAllocationGetDimX() would be replaced by __rsov_rsAllocationGetDimX()
// where the only difference is the argument being replaced by the unique
// ID. __rsov_rsAllocationGetDimX() does not really exist - it is used
// as a marker for this pass to work on.
//
// After this GAAccessTransformer pass, it would look like (in SPIRIT):
//
//   uint32_t foo = Metadata[0].size_x;
//
// where the OpFunctionCall to __rsov_rsAllocationGetDim() is replaced by
// an OpAccessChain and OpLoad from the metadata buffer.

class GAAccessorTransformer : public Transformer {
public:
  GAAccessorTransformer(Builder *b, Module *m, VariableInst *metadata)
      : mBuilder(b), mModule(m), mMetadata(metadata) {}

  Instruction *transform(FunctionCallInst *call) {
    FunctionInst *func =
        static_cast<FunctionInst *>(call->mOperand1.mInstruction);
    const char *name = mModule->lookupNameByInstruction(func);
    if (!name) {
      return call;
    }

    Instruction *inst = nullptr;
    // Maps name into a SPIR-V instruction
    // TODO: generalize it to support more accessors
    if (!strcmp(name, "__rsov_rsAllocationGetDimX")) {
      TypeIntInst *UInt32Ty = mModule->getUnsignedIntType(32);
      // TODO: hardcoded layout
      auto ConstZero = mModule->getConstant(UInt32Ty, 0U);
      auto ConstOne = mModule->getConstant(UInt32Ty, 1U);

      // TODO: Use constant memory later
      auto resultPtrType =
          mModule->getPointerType(StorageClass::Uniform, UInt32Ty);
      AccessChainInst *LoadPtr = mBuilder->MakeAccessChain(
          resultPtrType, mMetadata, {ConstZero, ConstZero, ConstOne});
      insert(LoadPtr);

      inst = mBuilder->MakeLoad(UInt32Ty, LoadPtr);
      inst->setId(call->getId());
    } else {
      inst = call;
    }
    return inst;
  }

private:
  Builder *mBuilder;
  Module *mModule;
  VariableInst *mMetadata;
};

} // namespace spirit
} // namespace android

namespace rs2spirv {

// android::spirit::Module *
std::vector<uint32_t>
TranslateGAAccessors(android::spirit::Builder &b, android::spirit::Module *m,
                     android::spirit::VariableInst *metadata, int *error) {
  android::spirit::GAAccessorTransformer trans(&b, m, metadata);
  *error = 0;
  return trans.transformSerialize(m);
}

} // namespace rs2spirv
