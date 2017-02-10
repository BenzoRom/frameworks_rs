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

#include "KernelSignature.h"

#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
std::string TypeToString(const Type *Ty) {
  assert(Ty);
  if (Ty->isVoidTy())
    return "void";

  if (auto *IT = dyn_cast<IntegerType>(Ty)) {
    if (IT->getBitWidth() == 32)
      return "int";
    else if (IT->getBitWidth() == 8)
      return "uchar";
  }

  if (Ty->isFloatTy())
    return "float";

  if (auto *VT = dyn_cast<VectorType>(Ty)) {
    auto *ET = VT->getElementType();
    if (auto *IT = dyn_cast<IntegerType>(ET)) {
      if (IT->getBitWidth() == 32)
        return "int4";
      else if (IT->getBitWidth() == 8)
        return "uchar4";
    }
    if (ET->isFloatTy())
      return "float4";
  }

  std::string badNameString;
  raw_string_ostream badNameStream(badNameString);
  badNameStream << '[';
  Ty->print(badNameStream);
  badNameStream << ']';
  return badNameStream.str();
}
} // namespace

namespace rs2spirv {

const std::string KernelSignature::wrapperPrefix = "%__rsov_";

KernelSignature::KernelSignature(const FunctionType *FT,
                                 const std::string Fname, Coords CK)
    : returnType(TypeToString(FT->getReturnType())), name(Fname),
      coordsKind(CK) {
  for (const auto *ArgT : FT->params()) {
    argumentTypes.push_back(TypeToString(ArgT));
  }
  // Drop special arguments
  // TODO: handle all special argument cases.
  argumentTypes.resize(argumentTypes.size()-size_t(CK));
}

void KernelSignature::dump() const {
  dbgs() << returnType << ' ' << name << '(' << argumentTypes[0];
  const auto CoordsNum = size_t(coordsKind);
  for (size_t i = 0; i != CoordsNum; ++i)
    dbgs() << ", " << CoordsNames[i];

  dbgs() << ")\n";
}

} // namespace rs2spirv
