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

#include "LinkerModule.h"

#include "KernelSignature.h"

#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include <fstream>
#include <sstream>

#define DEBUG_TYPE "rs2spirv-module"

using namespace llvm;

namespace rs2spirv {

bool SPIRVLine::hasCode() const {
  StringRef S(Line);
  S = S.trim();
  if (S.empty())
    return false;
  if (S[0] == ';')
    return false;

  return true;
}

static Optional<StringRef> GetFirstId(StringRef S, size_t StartPos,
                                      size_t &EndPos) {
  size_t Pos = S.find('%', StartPos);
  if (Pos == StringRef::npos) {
    return None;
  }

  const auto PosB = Pos;
  while (++Pos < S.size() && isspace(S[Pos]) == 0)
    ;

  EndPos = Pos;
  return StringRef(S.data() + PosB, EndPos - PosB);
}

void SPIRVLine::getIdentifiers(SmallVectorImpl<StringRef> &Out,
                               size_t StartPos) const {
  const StringRef S(Line);

  size_t Pos = StartPos;
  Optional<StringRef> Res;
  while ((Res = GetFirstId(S, Pos, Pos)))
    Out.push_back(*Res);
}

Optional<StringRef> SPIRVLine::getLHSIdentifier() const {
  size_t EndPos;
  const auto Id = GetFirstId(Line, 0, EndPos);
  if (!Id)
    return None;

  if (!contains("="))
    return None;

  return *Id;
}

Optional<StringRef> SPIRVLine::getRHS() const {
  const auto EqPos = Line.find('=', 0);
  if (EqPos == std::string::npos)
    return None;

  return StringRef(Line.c_str() + EqPos + 1).trim();
}

void SPIRVLine::getRHSIdentifiers(SmallVectorImpl<StringRef> &Out) const {
  const auto RHS = getRHS();
  if (!RHS)
    return;

  size_t Pos = 0;
  Optional<StringRef> Res;
  while ((Res = GetFirstId(*RHS, Pos, Pos)))
    Out.push_back(*Res);
}

bool SPIRVLine::replaceStr(StringRef Original, StringRef New) {
  const size_t Pos = StringRef(Line).find(Original);
  if (Pos == StringRef::npos)
    return false;

  Line.replace(Pos, Original.size(), New.str());
  return true;
}

bool SPIRVLine::replaceId(StringRef Original, StringRef New) {
  size_t Pos = StringRef(Line).find(Original, 0);
  if (Pos == StringRef::npos)
    return false;

  const auto OneAfter = Pos + Original.size();
  if (OneAfter < Line.size() && isspace(Line[OneAfter]) == 0) {
    Pos = StringRef(Line).find(Original, OneAfter);
    if (Pos == StringRef::npos)
      return false;
  }

  Line.replace(Pos, Original.size(), New.str());
  return true;
}

bool SPIRVLine::contains(StringRef S) const {
  return StringRef(Line).find(S, 0) != StringRef::npos;
}

void SPIRVLine::markAsEmpty() { Line = "; <<empty>>"; }

Block &Block::operator=(const Block &B) {
  assert(Kind == B.Kind);
  assert(Name == B.Name);
  Lines = B.Lines;

  return *this;
}

bool Block::addLine(SPIRVLine L, bool trim) {
  if (trim)
    L.trim();

  Lines.emplace_back(std::move(L));
  return true;
}

SPIRVLine &Block::getLastLine() {
  assert(!Lines.empty());
  return Lines.back();
}

const SPIRVLine &Block::getLastLine() const {
  assert(!Lines.empty());
  return Lines.back();
}

void Block::appendToStream(std::ostream &OS) const {
  for (const auto &L : Lines)
    OS << L.str() << '\n';
}

void Block::dump() const {
  dbgs() << "\n" << Name << "Block: {\n\n";
  for (const auto &L : Lines) {
    if (L.hasCode())
      dbgs() << '\t';
    dbgs() << L.str() << '\n';
  }
  dbgs() << "\n} (" << Name << "Block)\n\n";
}

void Block::replaceAllIds(StringRef Old, StringRef New) {
  for (auto &L : Lines)
    while (L.replaceId(Old, New))
      ;
}

bool Block::hasCode() const {
  return std::any_of(Lines.begin(), Lines.end(),
                     [](const SPIRVLine &L) { return L.hasCode(); });
}

size_t Block::getIdCount(StringRef Id) const {
  size_t Res = 0;
  for (const auto &L : Lines) {
    SmallVector<StringRef, 4> Ids;
    L.getIdentifiers(Ids);
    Res += std::count(Ids.begin(), Ids.end(), Id);
  }

  return Res;
}

void Block::removeNonCodeLines() {
  Lines.erase(std::remove_if(Lines.begin(), Lines.end(),
                             [](const SPIRVLine &L) { return !L.hasCode(); }),
              Lines.end());
}

bool HeaderBlock::getRSKernelNames(SmallVectorImpl<std::string> &Out) const {
  for (const auto &L : Lines)
    if (L.contains("OpString")) {
      const Optional<StringRef> Name = L.getLHSIdentifier();
      if (Name && *Name == "%RS_KERNELS") {
        auto LStr = L.str();
        LStr.erase(std::remove(LStr.begin(), LStr.end(), '"'), LStr.end());

        llvm::SmallVector<llvm::StringRef, 2> KernelNames;
        SPIRVLine(LStr).getRHSIdentifiers(KernelNames);

        Out.clear();
        // Returning StringRef to a destructed string is bad.
        // Need to duplicate the contents before returning.
        for (auto n : KernelNames) {
          Out.push_back(n);
        }

        return true;
      }
    }

  return false;
}

StringRef FunctionBlock::getFunctionName() const {
  assert(!Lines.empty());
  assert(Lines.front().contains("OpFunction"));

  Optional<StringRef> Name = Lines.front().getLHSIdentifier();
  assert(Name);
  return *Name;
}

size_t FunctionBlock::getArity() const {
  size_t A = 0;
  for (const auto &L : Lines)
    if (L.contains("OpFunctionParameter"))
      ++A;

  return A;
}

void FunctionBlock::getArgNames(SmallVectorImpl<StringRef> &Out) const {
  for (const auto &L : Lines)
    if (L.contains("OpFunctionParameter")) {
      Optional<StringRef> Id = L.getLHSIdentifier();
      assert(Id);
      Out.push_back(*Id);
    }
}

Optional<StringRef> FunctionBlock::getRetValName() const {
  for (const auto &L : Lines)
    if (L.contains("OpReturnValue")) {
      SmallVector<StringRef, 1> Id;
      L.getIdentifiers(Id);
      assert(Id.size() == 1);
      return Id.front();
    }

  return None;
}

iterator_range<Block::const_line_iter> FunctionBlock::body() const {
  auto It = Lines.begin();
  const auto End = Lines.end();

  while (It != End && !It->contains("OpLabel"))
    ++It;

  assert(It != End);

  ++It;
  const auto BBegin = It;

  while (It != End && !It->contains("OpReturn"))
    ++It;

  assert(It != End);

  return make_range(BBegin, It);
}

void FunctionBlock::getCalledFunctions(
    llvm::SmallVectorImpl<llvm::StringRef> &Out) const {
  for (const auto &L : Lines)
    if (L.contains("OpFunctionCall")) {
      SmallVector<StringRef, 4> Ids;
      L.getRHSIdentifiers(Ids);
      assert(Ids.size() >= 2);

      Out.push_back(Ids[1]);
    }
}

bool FunctionBlock::hasFunctionCalls() const {
  SmallVector<StringRef, 4> Callees;
  getCalledFunctions(Callees);
  return !Callees.empty();
}

bool FunctionBlock::isDirectlyRecursive() const {
  SmallVector<StringRef, 4> Callees;
  getCalledFunctions(Callees);

  const auto FName = getFunctionName();
  return std::find(Callees.begin(), Callees.end(), FName) != Callees.end();
}

bool FunctionBlock::isReturnTypeVoid() const {
  assert(Lines.size() >= 4);
  // At least 4 lines: OpFunction, OpLabel, OpReturn, OpFunctionEnd.

  SmallVector<StringRef, 2> Ids;
  Lines.front().getRHSIdentifiers(Ids);
  assert(Ids.size() == 2);

  if (Ids.front() != "%void" && Ids.front() != "%rs_linker_void")
    return false;

  SPIRVLine SecondLast = Lines[Lines.size() - 2];
  SecondLast.trim();
  return SecondLast.str() == "OpReturn";
}

LinkerModule::LinkerModule(std::istream &ModuleIn) {
  std::string Temp;
  std::vector<SPIRVLine> Ls;
  while (std::getline(ModuleIn, Temp))
    Ls.push_back(StringRef(Temp));

  auto It = Ls.begin();
  const auto End = Ls.end();

  {
    auto &HeaderBlck = addBlock<HeaderBlock>();
    while (It != End && !It->contains("OpDecorate"))
      HeaderBlck.addLine(*(It++));
  }

  {
    auto &DcrBlck = addBlock<DecorBlock>();
    while (It != End && !It->contains("OpType"))
      DcrBlck.addLine(*(It++));

    DcrBlck.removeNonCodeLines();
  }

  {
    auto &TypeAndConstBlck = addBlock<TypeAndConstBlock>();
    auto &VarBlck = addBlock<VarBlock>();

    while (It != End && !It->contains("OpFunction")) {
      if (!It->hasCode()) {
        ++It;
        continue;
      }

      if (It->contains("OpType") || It->contains("OpConstant")) {
        TypeAndConstBlck.addLine(*It);
      } else {
        VarBlck.addLine(*It);
      }

      ++It;
    }

    TypeAndConstBlck.removeNonCodeLines();
    VarBlck.removeNonCodeLines();
  }

  while (It != End) {
    // Consume empty lines between blocks.
    if (It->empty()) {
      ++It;
      continue;
    }

    Optional<StringRef> Id = It->getLHSIdentifier();
    assert(Id && "Functions should start with OpFunction");

    FunctionBlock &FunBlck = KernelSignature::isWrapper(*Id) ?
        addBlock<MainFunBlock>() : addBlock<FunctionBlock>();
    bool HasReturn = false;

    while (It != End) {
      if (It->empty()) {
        ++It;
        continue;
      }
      HasReturn |= It->contains("OpReturn");

      FunBlck.addLine(*(It++));
      if (FunBlck.getLastLine().contains("OpFunctionEnd"))
        break;
    }

    FunBlck.removeNonCodeLines();

    if (!HasReturn) {
      FunDeclBlock FunDeclBlck;
      for (auto &L : FunBlck.lines())
        FunDeclBlck.addLine(std::move(L));

      Blocks.pop_back();
      addBlock<FunDeclBlock>(std::move(FunDeclBlck));
    }
  }

  removeNonCode();
}

void LinkerModule::fixBlockOrder() {
  std::stable_sort(Blocks.begin(), Blocks.end(),
                   [](const block_ptr &LHS, const block_ptr &RHS) {
                     return LHS->getKind() < RHS->getKind();
                   });
}

bool LinkerModule::saveToFile(StringRef FName) const {
  std::ofstream Out(FName, std::ios::trunc);
  if (!Out.good())
    return false;

  for (const auto &BPtr : blocks()) {
    if (!isa<HeaderBlock>(BPtr.get()))
      Out << "\n\n; " << BPtr->Name.str() << "\n\n";

    for (const auto &L : BPtr->lines()) {
      if (L.hasCode())
        Out << "\t";
      Out << L.str() << '\n';
    }
  }

  return true;
}

void LinkerModule::removeEmptyBlocks() {
  removeBlocksIf([](const Block &B) { return B.empty(); });
}

void LinkerModule::removeNonCode() {
  for (auto &BPtr : Blocks)
    if (!isa<HeaderBlock>(BPtr.get()))
      BPtr->removeNonCodeLines();

  removeBlocksIf([](const Block &B) { return !B.hasCode(); });
}

void LinkerModule::removeUnusedFunctions() {
  std::vector<std::string> UsedFunctions;

  assert(Blocks.size());

  const auto &MB = getLastBlock<MainFunBlock>();
  for (const auto &L : MB.lines())
    if (L.contains("OpFunctionCall")) {
      SmallVector<StringRef, 4> Ids;
      L.getRHSIdentifiers(Ids);
      assert(Ids.size() >= 2);

      const auto &FName = Ids[1];
      UsedFunctions.push_back(FName.str());
    }

  removeBlocksIf([&UsedFunctions](const Block &B) {
    const auto *FunBlck = dyn_cast<FunctionBlock>(&B);
    if (!FunBlck)
      return false;

    if (isa<MainFunBlock>(FunBlck))
      return false;

    const auto FName = FunBlck->getFunctionName().str();
    return std::find(UsedFunctions.begin(), UsedFunctions.end(), FName) ==
           UsedFunctions.end();
  });
}

bool FuseTypesAndConstants(LinkerModule &LM) {
  StringMap<std::string> TypesAndConstDefs;
  StringMap<std::string> NameReps;

  for (auto *LPtr : LM.lines()) {
    assert(LPtr);
    auto &L = *LPtr;
    if (!L.contains("="))
      continue;

    SmallVector<StringRef, 4> IdsRefs;
    L.getRHSIdentifiers(IdsRefs);

    SmallVector<std::string, 4> Ids;
    Ids.reserve(IdsRefs.size());
    for (const auto &I : IdsRefs)
      Ids.push_back(I.str());

    for (auto &I : Ids)
      if (NameReps.count(I) != 0) {
        const bool Res = L.replaceId(I, NameReps[I]);
        (void)Res;
        assert(Res);
      }

    if (L.contains("OpType") || L.contains("OpConstant")) {
      const auto LHS = L.getLHSIdentifier();
      const auto RHS = L.getRHS();
      assert(LHS);
      assert(RHS);

      if (!RHS->startswith("OpTypeStruct") &&
          !RHS->startswith("OpTypeRuntimeArray") &&
          TypesAndConstDefs.count(*RHS) != 0) {
        NameReps.insert(
            std::make_pair(LHS->str(), TypesAndConstDefs[RHS->str()]));
        DEBUG(dbgs() << "New mapping: [" << LHS->str() << ", "
                     << TypesAndConstDefs[RHS->str()] << "]\n");
        L.markAsEmpty();
      } else {
        TypesAndConstDefs.insert(std::make_pair(RHS->str(), LHS->str()));
        DEBUG(dbgs() << "New val:\t" << RHS->str() << " : " << LHS->str()
                     << '\n');
      }
    };
  }

  LM.removeNonCode();

  return true;
}

struct FunctionCallInfo {
  StringRef RetValName;
  StringRef RetTy;
  StringRef FName;
  SmallVector<StringRef, 4> ArgNames;
};

static FunctionCallInfo GetFunctionCallInfo(const SPIRVLine &L) {
  assert(L.contains("OpFunctionCall"));

  const Optional<StringRef> Ret = L.getLHSIdentifier();
  assert(Ret);

  SmallVector<StringRef, 6> Ids;
  L.getRHSIdentifiers(Ids);
  assert(Ids.size() >= 2 && "No return type and function name");

  const StringRef RetTy = Ids[0];
  const StringRef FName = Ids[1];
  SmallVector<StringRef, 4> Args(Ids.begin() + 2, Ids.end());

  return {*Ret, RetTy, FName, std::move(Args)};
}

bool InlineFunctionCalls(LinkerModule &LM, MainFunBlock &MB) {
  DEBUG(dbgs() << "InlineFunctionCalls\n");
  MainFunBlock NewMB;

  auto MLines = MB.lines();
  auto MIt = MLines.begin();
  const auto MEnd = MLines.end();
  using iter_ty = decltype(MIt);

  auto SkipToFunctionCall = [&MEnd, &NewMB](iter_ty &It) {
    while (++It != MEnd && !It->contains("OpFunctionCall"))
      NewMB.addLine(*It);

    return It != MEnd;
  };

  NewMB.addLine(*MIt);

  std::vector<std::pair<std::string, std::string>> NameMapping;

  while (SkipToFunctionCall(MIt)) {
    assert(MIt->contains("OpFunctionCall"));
    const auto FInfo = GetFunctionCallInfo(*MIt);
    DEBUG(dbgs() << "Found function call:\t" << MIt->str() << '\n');

    SmallVector<Block *, 1> Callee;
    LM.getBlocksIf(Callee, [&FInfo](Block &B) {
      auto *FB = dyn_cast<FunctionBlock>(&B);
      if (!FB)
        return false;

      return FB->getFunctionName() == FInfo.FName;
    });

    if (Callee.size() != 1) {
      errs() << "Callee not found\n";
      return false;
    }

    auto *FB = cast<FunctionBlock>(Callee.front());

    if (FB->getArity() != FInfo.ArgNames.size()) {
      errs() << "Arity mismatch (caller: " << FInfo.ArgNames.size()
             << ", callee: " << FB->getArity() << ")\n";
      return false;
    }

    Optional<StringRef> RetValName = FB->getRetValName();
    if (!RetValName && !FB->isReturnTypeVoid()) {
      errs() << "Return value not found for a function with non-void "
                "return type.\n";
      return false;
    }

    SmallVector<StringRef, 4> Params;
    FB->getArgNames(Params);

    if (Params.size() != FInfo.ArgNames.size()) {
      errs() << "Params size mismatch\n";
      return false;
    }

    for (size_t i = 0, e = FInfo.ArgNames.size(); i < e; ++i) {
      DEBUG(dbgs() << "New param mapping: " << Params[i] << " -> "
                   << FInfo.ArgNames[i] << "\n");
      NameMapping.emplace_back(Params[i].str(), FInfo.ArgNames[i].str());
    }

    if (RetValName) {
      DEBUG(dbgs() << "New ret-val mapping: " << FInfo.RetValName << " -> "
                   << *RetValName << "\n");
      NameMapping.emplace_back(FInfo.RetValName.str(), RetValName->str());
    }

    const auto Body = FB->body();
    for (const auto &L : Body)
      NewMB.addLine(L);
  }

  while (MIt != MEnd) {
    NewMB.addLine(*MIt);
    ++MIt;
  }

  std::reverse(NameMapping.begin(), NameMapping.end());
  for (const auto &P : NameMapping) {
    DEBUG(dbgs() << "Replace " << P.first << " with " << P.second << "\n");
    NewMB.replaceAllIds(P.first, P.second);
  }

  MB = NewMB;

  return true;
}

bool TranslateInBoundsPtrAccessToAccess(SPIRVLine &L) {
  assert(L.contains(" OpInBoundsPtrAccessChain "));

  SmallVector<StringRef, 4> Ids;
  L.getRHSIdentifiers(Ids);

  if (Ids.size() < 4) {
    errs() << "OpInBoundsPtrAccessChain has not enough parameters:\n\t"
           << L.str();
    return false;
  }

  std::istringstream SS(L.str());
  std::string LHS, Eq, Op;
  SS >> LHS >> Eq >> Op;

  if (LHS.empty() || Eq != "=" || Op != "OpInBoundsPtrAccessChain") {
    errs() << "Could not decompose OpInBoundsPtrAccessChain:\n\t" << L.str();
    return false;
  }

  constexpr size_t ElementArgPosition = 2;

  std::ostringstream NewLine;
  NewLine << LHS << " " << Eq << " OpAccessChain ";
  for (size_t i = 0, e = Ids.size(); i != e; ++i)
    if (i != ElementArgPosition)
      NewLine << Ids[i].str() << " ";

  L.str() = NewLine.str();
  L.trim();

  return true;
}

bool FixInBoundsPtrAccessChain(MainFunBlock &MainB) {
  for (auto &L : MainB.lines()) {
    if (!L.contains("OpInBoundsPtrAccessChain"))
      continue;

    if (!TranslateInBoundsPtrAccessToAccess(L))
      return false;
  }

  return true;
}

bool InlineKernelIntoWrapper(LinkerModule &LM, MainFunBlock &MainB) {
  while (MainB.hasFunctionCalls())
    if (!InlineFunctionCalls(LM, MainB)) {
      errs() << "Could not inline function calls in main\n";
      return false;
    }

  return true;
}

// Replaces UndefValues in VectorShuffles with zeros, which is always
// safe, as the result for components marked as Undef is unused.
// Ex. 1)    OpVectorShuffle %v4uchar %a %b 0 1 2 4294967295 -->
//           OpVectorShuffle %v4uchar %a %b 0 1 2 0.
//
// Ex. 2)    OpVectorShuffle %v4uchar %a %b 0 4294967295 3 4294967295 -->
//           OpVectorShuffle %v4uchar %a %b 0 0 3 0.
//
// Fix needed for the current Vulkan driver, which crashed during
// backend compilation when case is not handled.
bool FixVectorShuffles(MainFunBlock &MB) {
  const StringRef UndefStr = " 4294967295 ";

  for (auto &L : MB.lines()) {
    if (!L.contains("OpVectorShuffle"))
      continue;

    L.str().push_back(' ');
    while (L.contains(UndefStr))
      L.replaceStr(UndefStr, " 0 ");

    L.trim();
  }

  return true;
}

// This function changes all Function StorageClass use into Uniform.
// It's needed, because llvm-spirv converter emits wrong StorageClass
// for globals.
// The transfromation, however, breaks legitimate uses of Function StorageClass
// inside functions.
//
//  Ex. 1. %ptr_Function_uint = OpTypePointer Function %uint
//     --> %ptr_Uniform_uint = OpTypePointer Uniform %uint
//
//  Ex. 2. %gep = OpAccessChain %ptr_Function_uchar %G %uint_zero
//     --> %gep = OpAccessChain %ptr_Uniform_uchar %G %uint_zero
//
// TODO: Consider a better way of fixing this.
void FixModuleStorageClass(LinkerModule &M) {
  for (auto *LPtr : M.lines()) {
    assert(LPtr);
    auto &L = *LPtr;

    while (L.contains(" Function"))
      L.replaceStr(" Function", " Uniform");

    while (L.contains("_Function_"))
      L.replaceStr("_Function_", "_Uniform_");
  }
}

bool Link(llvm::StringRef KernelFilename, llvm::StringRef WrapperFilename,
          llvm::StringRef OutputFilename) {
  DEBUG(dbgs() << "Linking...\n");

  std::ifstream WrapperF(WrapperFilename);
  if (!WrapperF.good()) {
    errs() << "Cannot open file: " << WrapperFilename << "\n";
  }
  std::ifstream KernelF(KernelFilename);
  if (!KernelF.good()) {
    errs() << "Cannot open file: " << KernelFilename << "\n";
  }

  LinkerModule WrapperM(WrapperF);
  LinkerModule KernelM(KernelF);

  WrapperF.close();
  KernelF.close();

  DEBUG(dbgs() << "WrapperF:\n");
  DEBUG(WrapperM.dump());
  DEBUG(dbgs() << "\n~~~~~~~~~~~~~~~~~~~~~~\n\nKernelF:\n");
  DEBUG(KernelM.dump());
  DEBUG(dbgs() << "\n======================\n\n");

  const char Prefix[] = "%rs_linker_";

  for (auto *LPtr : KernelM.lines()) {
    assert(LPtr);
    auto &L = *LPtr;
    size_t Pos = 0;
    while ((Pos = L.str().find("%", Pos)) != std::string::npos) {
      L.str().replace(Pos, 1, Prefix);
      Pos += strlen(Prefix);
    }
  }

  FixModuleStorageClass(KernelM);
  DEBUG(KernelM.dump());

  auto WBlocks = WrapperM.blocks();
  auto WIt = WBlocks.begin();
  const auto WEnd = WBlocks.end();

  auto KBlocks = KernelM.blocks();
  auto KIt = KBlocks.begin();
  const auto KEnd = KBlocks.end();

  LinkerModule OutM;

  if (WIt == WEnd || KIt == KEnd)
    return false;

  const auto *HeaderB = dyn_cast<HeaderBlock>(WIt->get());
  if (!HeaderB || !isa<HeaderBlock>(KIt->get()))
    return false;

  SmallVector<std::string, 2> KernelNames;
  const bool KernelsFound = HeaderB->getRSKernelNames(KernelNames);

  if (!KernelsFound) {
    errs() << "RS kernel names not found in wrapper\n";
    return false;
  }

  // KernelM module's HeaderBlock is skipped - it has OpenCL-specific code that
  // is replaced here with compute shader code.

  OutM.addBlock<HeaderBlock>(*HeaderB);

  if (++WIt == WEnd || ++KIt == KEnd)
    return false;

  const auto *DecorBW = dyn_cast<DecorBlock>(WIt->get());
  if (!DecorBW || !isa<DecorBlock>(KIt->get()))
    return false;

  // KernelM module's DecorBlock is skipped, because it contains OpenCL-specific
  // code that is not needed (eg. linkage type information).

  OutM.addBlock<DecorBlock>(*DecorBW);

  if (++WIt == WEnd || ++KIt == KEnd)
    return false;

  const auto *TypeAndConstBW = dyn_cast<TypeAndConstBlock>(WIt->get());
  auto *TypeAndConstBK = dyn_cast<TypeAndConstBlock>(KIt->get());
  if (!TypeAndConstBW || !TypeAndConstBK)
    return false;

  OutM.addBlock<TypeAndConstBlock>(*TypeAndConstBW);
  OutM.addBlock<TypeAndConstBlock>(*TypeAndConstBK);

  if (++WIt == WEnd || ++KIt == KEnd)
    return false;

  const auto *VarBW = dyn_cast<VarBlock>(WIt->get());
  auto *VarBK = dyn_cast<VarBlock>(KIt->get());
  if (!VarBW)
    return false;

  OutM.addBlock<VarBlock>(*VarBW);

  if (VarBK)
    OutM.addBlock<VarBlock>(*VarBK);
  else
    --KIt;

  SmallVector<MainFunBlock *, 2> MainBs;

  while (++WIt != WEnd) {
    auto *FunB = dyn_cast<FunctionBlock>(WIt->get());
    if (!FunB)
      return false;

    if (auto *MB = dyn_cast<MainFunBlock>(WIt->get())) {
      MainBs.push_back(&OutM.addBlock<MainFunBlock>(*MB));
    } else {
      OutM.addBlock<FunctionBlock>(*FunB);
    }
  }

  if (!MainBs.size()) {
    errs() << "Wrapper module has no main function\n";
    return false;
  }

  while (++KIt != KEnd) {
    // TODO: Check if FunDecl is a known runtime function.
    if (isa<FunDeclBlock>(KIt->get()))
      continue;

    auto *FunB = dyn_cast<FunctionBlock>(KIt->get());
    if (!FunB)
      return false;

    // TODO: Detect also indirect recurion.
    if (FunB->isDirectlyRecursive()) {
      errs() << "Function: " << FunB->getFunctionName().str()
             << " is recursive\n";
      return false;
    }

    OutM.addBlock<FunctionBlock>(*FunB);
  }

  OutM.fixBlockOrder();

  auto KernelName = KernelNames.begin();
  const auto KE = KernelNames.end();
  auto MainB = MainBs.begin();
  const auto ME = MainBs.end();

  for (; KernelName != KE && MainB != ME; ++KernelName, ++MainB) {
    if (!InlineKernelIntoWrapper(OutM, **MainB))
      return false;

    if (!FixInBoundsPtrAccessChain(**MainB))
      return false;

    if (!FixVectorShuffles(**MainB))
      return false;
  }

  if (KernelName != KE || MainB != ME) {
    errs() << "Inconsistent kernel metadata and definitions\n";
    return false;
  }

  OutM.removeUnusedFunctions();

  DEBUG(dbgs() << ">>>>>>>>>>>>  Output module after prelink:\n\n");
  DEBUG(OutM.dump());

  if (!FuseTypesAndConstants(OutM)) {
    errs() << "Type fusion failed\n";
    return false;
  }

  DEBUG(dbgs() << ">>>>>>>>>>>>  Output module after value fusion:\n\n");
  DEBUG(OutM.dump());

  if (!OutM.saveToFile(OutputFilename)) {
    errs() << "Could not save to file: " << OutputFilename << "\n";
    return false;
  }

  return true;
}

} // namespace rs2spirv
