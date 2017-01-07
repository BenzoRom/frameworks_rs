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

#include "ReflectionPass.h"

#include "KernelSignature.h"

#include "RSAllocationUtils.h"
#include "bcinfo/MetadataExtractor.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/SPIRV.h"

#include <map>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>

#define DEBUG_TYPE "rs2spirv-reflection"

using namespace llvm;

namespace rs2spirv {

namespace {

enum class RSType {
  rs_bad = -1,
  rs_void,
  rs_uchar,
  rs_int,
  rs_float,
  rs_uchar4,
  rs_int4,
  rs_float4
};

RSType StrToRsTy(StringRef S) {
  RSType Ty = StringSwitch<RSType>(S)
                  .Case("void", RSType::rs_void)
                  .Case("uchar", RSType::rs_uchar)
                  .Case("int", RSType::rs_int)
                  .Case("float", RSType::rs_float)
                  .Case("uchar4", RSType::rs_uchar4)
                  .Case("int4", RSType::rs_int4)
                  .Case("float4", RSType::rs_float4)
                  .Default(RSType::rs_bad);
  return Ty;
}

struct TypeMapping {
  RSType RSTy;
  bool isVectorTy;
  // Scalar types are accessed (loaded/stored) using wider (vector) types.
  // 'vecLen' corresponds to width of such vector type.
  // As for vector types, 'vectorWidth' is just width of such type.
  size_t vectorWidth;
  std::string SPIRVTy;
  std::string SPIRVScalarTy;
  std::string SPIRVImageFormat;
  // TODO: Handle different image formats for read and write.
  std::string SPIRVImageReadType;

  TypeMapping(RSType RSTy, bool IsVectorTy, size_t VectorLen,
              StringRef SPIRVScalarTy, StringRef SPIRVImageFormat)
      : RSTy(RSTy), isVectorTy(IsVectorTy), vectorWidth(VectorLen),
        SPIRVScalarTy(SPIRVScalarTy), SPIRVImageFormat(SPIRVImageFormat) {
    assert(vectorWidth != 0);

    if (isVectorTy) {
      std::ostringstream OSS;
      OSS << "%v" << vectorWidth << SPIRVScalarTy.drop_front().str();
      SPIRVTy = OSS.str();
      SPIRVImageReadType = SPIRVTy;
      return;
    }

    SPIRVTy = SPIRVScalarTy;
    std::ostringstream OSS;
    OSS << "%v" << vectorWidth << SPIRVScalarTy.drop_front().str();
    SPIRVImageReadType = OSS.str();
  }
};

class ReflectionPass : public ModulePass {
  typedef SmallVector<KernelSignature, 4> KernelSignatures;

  std::ostream &OS;
  bcinfo::MetadataExtractor &ME;

  static const std::map<RSType, TypeMapping> TypeMappings;

  static const TypeMapping *getMapping(RSType RsTy) {
    auto it = TypeMappings.find(RsTy);
    if (it != TypeMappings.end())
      return &it->second;

    return nullptr;
  };

  static const TypeMapping *getMapping(StringRef Str) {
    auto Ty = StrToRsTy(Str);
    return getMapping(Ty);
  }

  static const TypeMapping *getMappingOrPrintError(StringRef Str) {
    const auto *TM = ReflectionPass::getMapping(Str);
    if (!TM)
      errs() << "LLVM to SPIRV type mapping for type:\t" << Str
             << " not found\n";

    return TM;
  }

  std::string nextResultID();
  std::string bufferNameToStructName(const std::string &);
  std::string emitBuffer(const std::string &,
                         const std::string &idBufVar = std::string(),
                         const std::string &idArrTy = std::string());
  std::string emitBufferUsingRSType(const std::string &,
                                    const std::string &idBufVar = std::string(),
                                    const std::string &idArrTy = std::string());
  std::string emitInputBuffer(const KernelSignature &Kernel,
                              const std::string &idBufVar = std::string(),
                              const std::string &idArrTy = std::string());
  std::string emitOutputBuffer(const KernelSignature &Kernel,
                               const std::string &idBufVar = std::string(),
                               const std::string &idArrTy = std::string());

  bool emitHeader(const Module &M, const KernelSignatures &Kernel);
  bool emitDecorations(const Module &M,
                       const SmallVectorImpl<RSAllocationInfo> &RSAllocs,
                       const KernelSignatures &Kernels,
                       const std::string &idInput, const std::string &idOutput,
                       const std::string &idInputMemTy,
                       const std::string &idOutputMemTy);
  void emitCommonTypes();
  bool extractKernelSignatures(const Module &M,
                               SmallVectorImpl<KernelSignature> &Out);
  bool emitKernelTypes(const KernelSignature &Kernel);
  void emitGLGlobalInput();
  bool emitRSAllocImages(const SmallVectorImpl<RSAllocationInfo> &RSAllocs);
  bool emitConstants(const KernelSignature &Kernel);
  void emitRTFunctions();
  bool emitRSAllocFunctions(
      Module &M, const SmallVectorImpl<RSAllocationInfo> &RSAllocs,
      const SmallVectorImpl<RSAllocationCallInfo> &RSAllocAccesses);
  bool emitMainUsingBuffersForInputOutput(
      const KernelSignature &Kernel,
      const SmallVectorImpl<RSAllocationInfo> &RSAllocs,
      const std::string &inputBuffer, const std::string &outputBuffer);

public:
  static char ID;
  explicit ReflectionPass(std::ostream &OS, bcinfo::MetadataExtractor &ME)
      : ModulePass(ID), OS(OS), ME(ME) {}

  const char *getPassName() const override { return "ReflectionPass"; }

  bool runOnModule(Module &M) override {
    DEBUG(dbgs() << "ReflectionPass\n");

    KernelSignatures Kernels;
    if (!extractKernelSignatures(M, Kernels)) {
      errs() << "Extraction of kernels failed\n";
      return false;
    }

    if (!emitHeader(M, Kernels)) {
      errs() << "Emiting header failed\n";
      return false;
    }

    SmallVector<RSAllocationInfo, 2> RSAllocs;
    if (!getRSAllocationInfo(M, RSAllocs)) {
      errs() << "Extracting rs_allocation info failed\n";
      return false;
    }

    SmallVector<RSAllocationCallInfo, 4> RSAllocAccesses;
    if (!getRSAllocAccesses(RSAllocs, RSAllocAccesses)) {
      errs() << "Extracting rsGEA/rsSEA info failed\n";
      return false;
    }

    if (!emitDecorations(M, RSAllocs, Kernels, "inputBuffer", "outputBuffer",
                         "inputMemTy", "outputMemTy")) {
      errs() << "Emiting decorations failed\n";
      return false;
    }

    emitCommonTypes();

    for (const auto &Kernel : Kernels) {
      if (!emitKernelTypes(Kernel)) {
        errs() << "Emitting kernel types for " << Kernel.name << " failed\n";
        return false;
      }
    }

    emitGLGlobalInput();

    for (const auto &Kernel : Kernels) {
      std::string inputBuffer =
          emitInputBuffer(Kernel, "inputBuffer", "inputMemTy");
      if (inputBuffer.empty()) {
        errs() << "Emitting input buffer failed\n";
        return false;
      }

      std::string outputBuffer =
          emitOutputBuffer(Kernel, "outputBuffer", "outputMemTy");
      if (outputBuffer.empty()) {
        errs() << "Emitting output buffer failed\n";
        return false;
      }

      if (!emitRSAllocImages(RSAllocs)) {
        errs() << "Emitting rs_allocation images for " << Kernel.name
               << " failed\n";
        return false;
      }

      if (!emitConstants(Kernel)) {
        errs() << "Emitting constants for " << Kernel.name << " failed\n";
        return false;
      }
    }

    emitRTFunctions();

    for (const auto &Kernel : Kernels) {
      if (!emitRSAllocFunctions(M, RSAllocs, RSAllocAccesses)) {
        errs() << "Emitting rs_allocation runtime functions for " << Kernel.name
               << " failed\n";
        return false;
      }

      if (!emitMainUsingBuffersForInputOutput(
              Kernel, RSAllocs, Kernel.getTempName("inputBuffer"),
              Kernel.getTempName("outputBuffer"))) {
        errs() << "Emitting main using buffers failed\n";
        return false;
      }
    }

    // Return false, as the module is not modified.
    return false;
  }
};

// TODO: Add other types: bool, double, char, uchar, long, ulong
//  and their vector counterparts.
// TODO: Support vector types of width different than 4. eg. float3.
const std::map<RSType, TypeMapping> ReflectionPass::TypeMappings = {
    {RSType::rs_void, {RSType::rs_void, false, 1, "%void", ""}},
    {RSType::rs_uchar, {RSType::rs_uchar, false, 4, "%uchar", "R8ui"}},
    {RSType::rs_int, {RSType::rs_void, false, 4, "%int", "R32i"}},
    {RSType::rs_float, {RSType::rs_float, false, 4, "%float", "R32f"}},
    {RSType::rs_uchar4, {RSType::rs_uchar4, true, 4, "%uchar", "Rgba8ui"}},
    {RSType::rs_int4, {RSType::rs_int4, true, 4, "%int", "Rgba32i"}},
    {RSType::rs_float4, {RSType::rs_float4, true, 4, "%float", "Rgba32f"}}};
};

char ReflectionPass::ID = 0;

ModulePass *createReflectionPass(std::ostream &OS,
                                 bcinfo::MetadataExtractor &ME) {
  return new ReflectionPass(OS, ME);
}

bool ReflectionPass::emitHeader(const Module &M,
                                const KernelSignatures &Kernels) {
  DEBUG(dbgs() << "emitHeader\n");

  OS << "; SPIR-V\n"
        "; Version: 1.0\n"
        "; Generator: rs2spirv;\n"
        "; Bound: 1024\n"
        "; Schema: 0\n"
        "      OpCapability Shader\n"
        "      OpCapability StorageImageWriteWithoutFormat\n"
        "      OpCapability Addresses\n"
        " %glsl_ext_ins = OpExtInstImport \"GLSL.std.450\"\n"
        "      OpMemoryModel Physical32 GLSL450\n";
  for (const auto &Kernel : Kernels) {
    OS << "      OpEntryPoint GLCompute " << Kernel.getWrapperName() << " "
       << "\"" << Kernel.name
       << "\" %gl_GlobalInvocationID %gl_NumWorkGroups\n";
  }
  for (const auto &Kernel : Kernels) {
    OS << "      OpExecutionMode " << Kernel.getWrapperName()
       << " LocalSize 1 1 1\n";
  }
  OS << "      OpSource GLSL 450\n"
        "      OpSourceExtension \"GL_ARB_separate_shader_objects\"\n"
        "      OpSourceExtension \"GL_ARB_shading_language_420pack\"\n"
        "      OpSourceExtension \"GL_GOOGLE_cpp_style_line_directive\"\n"
        "      OpSourceExtension \"GL_GOOGLE_include_directive\"\n";

  const size_t RSKernelNum = ME.getExportForEachSignatureCount();

  if (RSKernelNum == 0)
    return false;

  const char **RSKernelNames = ME.getExportForEachNameList();

  OS << " %RS_KERNELS = OpString \"";

  for (size_t i = 0; i < RSKernelNum; ++i)
    if (RSKernelNames[i] != StringRef("root"))
      OS << '%' << RSKernelNames[i] << " ";

  OS << "\"\n";

  return true;
}

bool ReflectionPass::emitDecorations(
    const Module &M, const SmallVectorImpl<RSAllocationInfo> &RSAllocs,
    const KernelSignatures &Kernels, const std::string &inputBuffer,
    const std::string &outputBuffer, const std::string &inputMemTy,
    const std::string &outputMemTy) {
  DEBUG(dbgs() << "emitDecorations\n");

  const std::string &inputBufferS = bufferNameToStructName(inputBuffer);
  const std::string &outputBufferS = bufferNameToStructName(outputBuffer);

  // TODO: adjust stride based on type
  OS << R"(
    OpDecorate %gl_GlobalInvocationID BuiltIn GlobalInvocationId
    OpDecorate %gl_NumWorkGroups BuiltIn NumWorkgroups
    OpDecorate %gl_WorkGroupSize BuiltIn WorkgroupSize
)";

  for (const auto &K : Kernels) {
    OS << "OpDecorate " << K.getTempName(inputMemTy) << " ArrayStride 16\n";
    OS << "OpMemberDecorate " << K.getTempName(inputBufferS) << " 0 Offset 0\n";
    OS << "OpDecorate " << K.getTempName(inputBufferS) << " BufferBlock\n";
    OS << "OpDecorate " << K.getTempName(inputBuffer) << " DescriptorSet 0\n";
    OS << "OpDecorate " << K.getTempName(inputBuffer) << " Binding 0\n";
    OS << "OpDecorate " << K.getTempName(outputMemTy) << " ArrayStride 16\n";
    OS << "OpMemberDecorate " << K.getTempName(outputBufferS) << " 0 Offset 0\n";
    OS << "OpDecorate " << K.getTempName(outputBufferS) << " BufferBlock\n";
    OS << "OpDecorate " << K.getTempName(outputBuffer) << " DescriptorSet 0\n";
    OS << "OpDecorate " << K.getTempName(outputBuffer) << " Binding 1\n";
  }

  const auto GlobalsB = M.globals().begin();
  const auto GlobalsE = M.globals().end();
  const auto Found =
      std::find_if(GlobalsB, GlobalsE, [](const GlobalVariable &GV) {
        return GV.getName() == "__GPUBlock";
      });

  if (Found == GlobalsE)
    return true; // GPUBlock not found - not an error by itself.

  const GlobalVariable &G = *Found;

  DEBUG(dbgs() << "Found GPUBlock:\t");
  DEBUG(G.dump());

  bool IsCorrectTy = false;
  if (const auto *PtrTy = dyn_cast<PointerType>(G.getType())) {
    if (auto *StructTy = dyn_cast<StructType>(PtrTy->getElementType())) {
      IsCorrectTy = true;

      const auto &DLayout = M.getDataLayout();
      const auto *SLayout = DLayout.getStructLayout(StructTy);
      assert(SLayout);

      for (size_t i = 0, e = StructTy->getNumElements(); i != e; ++i)
        OS << "      OpMemberDecorate %rs_linker_struct___GPUBuffer " << i
           << " Offset " << SLayout->getElementOffset(i) << '\n';
    }
  }

  if (!IsCorrectTy) {
    errs() << "GPUBlock is not of expected type:\t";
    G.print(errs());
    G.getType()->print(errs());
    return false;
  }

  OS << "      OpDecorate %rs_linker_struct___GPUBuffer BufferBlock\n";
  OS << "      OpDecorate %rs_linker___GPUBlock DescriptorSet 0\n";
  OS << "      OpDecorate %rs_linker___GPUBlock Binding 2\n";

  size_t BindingNum = 3;

  for (const auto &A : RSAllocs) {
    OS << "      OpDecorate " << A.VarName << "_var DescriptorSet 0\n";
    OS << "      OpDecorate " << A.VarName << "_var Binding " << BindingNum
       << '\n';
    ++BindingNum;
  }

  return true;
}

void ReflectionPass::emitCommonTypes() {
  DEBUG(dbgs() << "emitCommonTypes\n");

  OS << "\n\n"
        "%void = OpTypeVoid\n"
        "%fun_void = OpTypeFunction %void\n"
        "%float = OpTypeFloat 32\n"
        "%v2float = OpTypeVector %float 2\n"
        "%v3float = OpTypeVector %float 3\n"
        "%v4float = OpTypeVector %float 4\n"
        "%int = OpTypeInt 32 1\n"
        "%v2int = OpTypeVector %int 2\n"
        "%v4int = OpTypeVector %int 4\n"
        "%uchar = OpTypeInt 8 0\n"
        "%v2uchar = OpTypeVector %uchar 2\n"
        "%v3uchar = OpTypeVector %uchar 3\n"
        "%v4uchar = OpTypeVector %uchar 4\n"
        "%uint = OpTypeInt 32 0\n"
        "%v2uint = OpTypeVector %uint 2\n"
        "%v3uint = OpTypeVector %uint 3\n"
        "%v4uint = OpTypeVector %uint 4\n"
        "%fun_f3_uc3 = OpTypeFunction %v3float %v3uchar\n"
        "%fun_f3_u3 = OpTypeFunction %v3float %v3uint\n"
        "%fun_f4_uc4 = OpTypeFunction %v4float %v4uchar\n"
        "%fun_uc3_f3 = OpTypeFunction %v3uchar %v3float\n"
        "%fun_u3_f3 = OpTypeFunction %v3uint %v3float\n"
        "%fun_uc4_f4 = OpTypeFunction %v4uchar %v4float\n"
        "%fun_uc4_u4 = OpTypeFunction %v4uchar %v4uint\n"
        "%fun_u4_uc4 = OpTypeFunction %v4uint %v4uchar\n"
        "%fun_f_f = OpTypeFunction %float %float\n"
        "%fun_f_ff = OpTypeFunction %float %float %float\n"
        "%fun_f_fff = OpTypeFunction %float %float %float %float\n"
        "%fun_f_f2f2 = OpTypeFunction %float %v2float %v2float\n"
        "%fun_f_f3f3 = OpTypeFunction %float %v3float %v3float\n"
        "%fun_f3_f3ff = OpTypeFunction %v3float %v3float %float %float\n"
        "%fun_i_iii = OpTypeFunction %int %int %int %int\n"
        "%fun_uc_uu = OpTypeFunction %uchar %uint %uint\n"
        "%fun_u_uu = OpTypeFunction %uint %uint %uint\n"
        "%fun_u_uuu = OpTypeFunction %uint %uint %uint %uint\n"
        "%fun_u3_u3uu = OpTypeFunction %v3uint %v3uint %uint %uint\n";
}

static Coords GetCoordsKind(const Function &F) {
  if (F.arg_size() <= 1)
    return Coords::None;

  DEBUG(F.getFunctionType()->dump());

  SmallVector<const Argument *, 4> Args;
  Args.reserve(F.arg_size());
  for (const auto &Arg : F.args())
    Args.push_back(&Arg);

  auto IsInt32 = [](const Argument *Arg) {
    assert(Arg);
    auto *Ty = Arg->getType();
    auto IntTy = dyn_cast<IntegerType>(Ty);
    if (!IntTy)
      return false;

    return IntTy->getBitWidth() == 32;
  };

  size_t LastInt32Num = 0;
  size_t XPos = -1; // npos - not found.
  auto RIt = Args.rbegin();
  const auto REnd = Args.rend();
  while (RIt != REnd && IsInt32(*RIt)) {
    if ((*RIt)->getName() == "x")
      XPos = Args.size() - 1 - LastInt32Num;

    ++LastInt32Num;
    ++RIt;
  }

  DEBUG(dbgs() << "Original number of last i32's: " << LastInt32Num << '\n');
  DEBUG(dbgs() << "X found at position: " << XPos << '\n');
  if (XPos == size_t(-1) || Args.size() - XPos > size_t(Coords::Last))
    return Coords::None;

  // Check remaining coordinate names.
  for (size_t i = 1, c = XPos + 1, e = Args.size(); c != e; ++i, ++c)
    if (Args[c]->getName() != CoordsNames[i])
      return Coords::None;

  DEBUG(dbgs() << "Coords: not none!\n");

  return Coords(Args.size() - XPos);
}

bool ReflectionPass::extractKernelSignatures(
    const Module &M, SmallVectorImpl<KernelSignature> &Out) {
  DEBUG(dbgs() << "extractKernelSignatures\n");

  for (const auto &F : M.functions()) {
    if (F.isDeclaration())
      continue;

    const auto CoordsKind = GetCoordsKind(F);
    const auto CoordsNum = unsigned(CoordsKind);
    if (F.arg_size() != CoordsNum + 1) {
      // TODO: Handle different arrities (and lack of return value).
      errs() << "Unsupported kernel signature.\n";
      return false;
    }

    const auto *FT = F.getFunctionType();
    Out.push_back(KernelSignature(FT, F.getName(), CoordsKind));
    DEBUG(Out.back().dump());
  }

  return true;
}

bool ReflectionPass::emitKernelTypes(const KernelSignature &Kernel) {
  DEBUG(dbgs() << "emitKernelTypes\n");

  const auto *RTMapping = getMappingOrPrintError(Kernel.returnType);
  const auto *ArgTMapping = getMappingOrPrintError(Kernel.argumentTypes[0]);

  if (!RTMapping || !ArgTMapping)
    return false;

  OS << '\n'
     << Kernel.getTempName("kernel_function_ty") << " = OpTypeFunction "
     << RTMapping->SPIRVTy << ' ' << ArgTMapping->SPIRVTy;

  const auto CoordsNum = unsigned(Kernel.coordsKind);
  for (size_t i = 0; i != CoordsNum; ++i)
    OS << " %uint";

  OS << '\n';

  OS << Kernel.getTempName("ptr_function_ty") << " = OpTypePointer Function "
     << RTMapping->SPIRVTy << "\n";
  OS << Kernel.getTempName("ptr_function_access_ty")
     << " = OpTypePointer Function " << RTMapping->SPIRVImageReadType << "\n\n";

  return true;
}

std::string ReflectionPass::nextResultID() {
  static unsigned int nextID = 0;
  std::string str;
  std::stringstream ss(str);
  ss << "%res" << nextID++;
  return ss.str();
}

std::string ReflectionPass::bufferNameToStructName(const std::string &buffer) {
  return std::string(buffer).append("S");
}

std::string ReflectionPass::emitBuffer(const std::string &elementType,
                                       const std::string &idBufVar,
                                       const std::string &idArrTy) {
  std::string arrayType = idArrTy.empty() ? nextResultID() : idArrTy;
  std::string bufferPtrType = nextResultID();
  std::string bufferVar = idBufVar.empty() ? nextResultID() : idBufVar;
  std::string bufferType = bufferNameToStructName(bufferVar);

  OS << arrayType << " = OpTypeRuntimeArray " << elementType << "\n";
  OS << bufferType << " = OpTypeStruct " << arrayType << "\n";
  OS << bufferPtrType << " = OpTypePointer Uniform " << bufferType << "\n";
  OS << bufferVar << " = OpVariable " << bufferPtrType << " Uniform\n";

  return bufferVar;
  ;
}

std::string ReflectionPass::emitBufferUsingRSType(const std::string &type,
                                                  const std::string &idBufVar,
                                                  const std::string &idArrTy) {
  const auto *ArgTMapping = getMappingOrPrintError(type);
  if (!ArgTMapping)
    return std::string();

  std::string bufferID = emitBuffer(ArgTMapping->SPIRVTy, idBufVar, idArrTy);

  return bufferID;
}

std::string ReflectionPass::emitInputBuffer(const KernelSignature &Kernel,
                                            const std::string &idBufVar,
                                            const std::string &idArrTy) {
  DEBUG(dbgs() << __FUNCTION__ << "\n");
  return emitBufferUsingRSType(Kernel.argumentTypes[0],
                               Kernel.getTempName(idBufVar),
                               Kernel.getTempName(idArrTy));
}

std::string ReflectionPass::emitOutputBuffer(const KernelSignature &Kernel,
                                             const std::string &idBufVar,
                                             const std::string &idArrTy) {
  DEBUG(dbgs() << __FUNCTION__ << "\n");
  return emitBufferUsingRSType(Kernel.returnType, Kernel.getTempName(idBufVar),
                               Kernel.getTempName(idArrTy));
}

void ReflectionPass::emitGLGlobalInput() {
  DEBUG(dbgs() << "emitGLGlobalInput\n");

  OS << R"(
%_ptr_Function_uint = OpTypePointer Function %uint
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Input_uint = OpTypePointer Input %uint
%_ptr_Input_v3uint = OpTypePointer Input %v3uint
%gl_GlobalInvocationID = OpVariable %_ptr_Input_v3uint Input
%gl_NumWorkGroups = OpVariable %_ptr_Input_v3uint Input
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%group_size_x = OpConstant %uint 1
%group_size_y = OpConstant %uint 1
%group_size_z = OpConstant %uint 1
%gl_WorkGroupSize = OpConstantComposite %v3uint %group_size_x %group_size_y %group_size_z
%global_input_ptr_ty = OpTypePointer Input %v3uint
)";
}

bool ReflectionPass::emitRSAllocImages(
    const SmallVectorImpl<RSAllocationInfo> &RSAllocs) {
  DEBUG(dbgs() << "emitRSAllocImages\n");

  for (const auto &A : RSAllocs) {
    if (!A.RSElementType) {
      errs() << "Type of variable " << A.VarName << " not infered.\n";
      return false;
    }

    const auto *AMapping = getMappingOrPrintError(*A.RSElementType);
    if (!AMapping)
      return false;

    OS << '\n'
       << A.VarName << "_image_ty"
       << " = OpTypeImage " << AMapping->SPIRVScalarTy << " 2D 0 0 0 2 "
       << AMapping->SPIRVImageFormat << '\n'
       << A.VarName << "_image_ptr_ty"
       << " = OpTypePointer UniformConstant " << A.VarName << "_image_ty\n";

    OS << A.VarName << "_var = OpVariable " << A.VarName
       << "_image_ptr_ty Image\n";
  }

  return true;
}

bool ReflectionPass::emitConstants(const KernelSignature &Kernel) {
  DEBUG(dbgs() << "emitConstants\n");

  // TODO: The types do not seem to belong here
  OS << "%uint_zero = OpConstant %uint 0\n"
        "%uint_one = OpConstant %uint 1\n"
        "%float_zero = OpConstant %float 0\n";

  return true;
}

static std::string GenerateConversionFun(const char *Name, const char *FType,
                                         const char *From, const char *To,
                                         const char *ConversionOp) {
  std::ostringstream OS;

  OS << "\n"
     << "%rs_linker_" << Name << " = OpFunction " << To << " Pure " << FType
     << "\n"
     << "%param" << Name << " = OpFunctionParameter " << From << "\n"
     << "%label" << Name << " = OpLabel\n"
     << "%res" << Name << " = " << ConversionOp << " " << To << " %param"
     << Name << "\n"
     << "      OpReturnValue %res" << Name << "\n"
     << "      OpFunctionEnd\n";

  return OS.str();
}

static std::string GenerateEISFun(const char *Name, const char *FType,
                                  const char *RType,
                                  const SmallVector<const char *, 4> &ArgTypes,
                                  const char *InstName) {
  std::ostringstream OS;

  OS << '\n'
     << "%rs_linker_" << Name << " = OpFunction " << RType << " Pure " << FType
     << '\n';

  for (size_t i = 0, e = ArgTypes.size(); i < e; ++i)
    OS << "%param" << Name << i << " = OpFunctionParameter " << ArgTypes[i]
       << "\n";

  OS << "%label" << Name << " = OpLabel\n"
     << "%res" << Name << " = "
     << "OpExtInst " << RType << " %glsl_ext_ins " << InstName;

  for (size_t i = 0, e = ArgTypes.size(); i < e; ++i)
    OS << " %param" << Name << i;

  OS << '\n'
     << "      OpReturnValue %res" << Name << "\n"
     << "      OpFunctionEnd\n";

  return OS.str();
}

// This SPIRV function generator relies heavily on future inlining.
// Currently, the inliner doesn't perform any type checking - it blindly
// maps function parameters to supplied parameters at call site.
// It's non-trivial to generate correct SPIRV function signature based only
// on the LLVM one, and the current design doesn't allow lazy type generation.
//
// TODO: Consider less horrible generator design that doesn't rely on lack of
// type checking in the inliner.
static std::string GenerateRSGEA(const char *Name, const char *RType,
                                 StringRef LoadName, Coords CoordsKind) {
  assert(CoordsKind != Coords::None);
  std::ostringstream OS;

  OS << "\n"
     << "%rs_linker_" << Name << " = OpFunction " << RType
     << " None %rs_inliner_placeholder_ty\n";

  // Since the inliner doesn't perform type checking, function and parameter
  // types can be anything. %rs_inliner_placeholder_ty is just a placeholder
  // name that will disappear after inlining.

  OS << "%rs_drop_param_" << Name << " = OpFunctionParameter "
     << "%rs_inliner_placeholder_ty\n";

  for (size_t i = 0, e = size_t(CoordsKind); i != e; ++i)
    OS << "%param" << Name << '_' << CoordsNames[i].str()
       << " = OpFunctionParameter %uint\n";

  OS << "%label" << Name << " = OpLabel\n";
  OS << "%arg" << Name << " = OpCompositeConstruct %v" << size_t(CoordsKind)
     << "uint ";

  for (size_t i = 0, e = size_t(CoordsKind); i != e; ++i)
    OS << "%param" << Name << '_' << CoordsNames[i].str() << ' ';

  OS << '\n';

  OS << "%read" << Name << " = OpImageRead " << RType << ' ' << LoadName.str()
     << " %arg" << Name << '\n';
  OS << "      OpReturnValue %read" << Name << '\n';
  OS << "      OpFunctionEnd\n";

  return OS.str();
}

// The same remarks as to GenerateRSGEA apply to SEA function generator.
static std::string GenerateRSSEA(const char *Name, StringRef LoadName,
                                 Coords CoordsKind) {
  assert(CoordsKind != Coords::None);
  std::ostringstream OS;

  // %rs_inliner_placeholder_ty will disappear after inlining.
  OS << "\n"
     << "%rs_linker_" << Name << " = OpFunction %void None "
     << "%rs_inliner_placeholder_ty\n";

  OS << "%rs_placeholder_param_" << Name << " = OpFunctionParameter "
     << "%rs_inliner_placeholder_ty\n";
  OS << "%param" << Name << "_new_val = OpFunctionParameter "
     << "%rs_inliner_placeholder_ty\n";

  for (size_t i = 0, e = size_t(CoordsKind); i != e; ++i)
    OS << "%param" << Name << '_' << CoordsNames[i].str()
       << " = OpFunctionParameter %uint\n";

  OS << "%label" << Name << " = OpLabel\n";
  OS << "%arg" << Name << " = OpCompositeConstruct %v" << size_t(CoordsKind)
     << "uint ";

  for (size_t i = 0, e = size_t(CoordsKind); i != e; ++i)
    OS << "%param" << Name << '_' << CoordsNames[i].str() << ' ';

  OS << '\n';

  OS << "OpImageWrite " << LoadName.str() << " %arg" << Name << " %param"
     << Name << "_new_val\n";
  OS << "      OpReturn\n";
  OS << "      OpFunctionEnd\n";

  return OS.str();
}

void ReflectionPass::emitRTFunctions() {
  DEBUG(dbgs() << "emitRTFunctions\n");

  // TODO: Emit other runtime functions.
  // TODO: Generate libary file instead of generating functions below
  // every compilation.

  // Use uints as Khronos' SPIRV converter turns LLVM's i32s into uints.

  OS << GenerateConversionFun("_Z14convert_float4Dv4_h", "%fun_f4_uc4",
                              "%v4uchar", "%v4float", "OpConvertUToF");

  OS << GenerateConversionFun("_Z14convert_uchar4Dv4_f", "%fun_uc4_f4",
                              "%v4float", "%v4uchar", "OpConvertFToU");

  OS << GenerateConversionFun("_Z14convert_float3Dv3_h", "%fun_f3_uc3",
                              "%v3uchar", "%v3float", "OpConvertUToF");

  OS << GenerateConversionFun("_Z14convert_uchar3Dv3_f", "%fun_uc3_f3",
                              "%v3float", "%v3uchar", "OpConvertFToU");

  OS << GenerateConversionFun("_Z12convert_int3Dv3_f", "%fun_u3_f3", "%v3float",
                              "%v3uint", "OpConvertFToU");

  OS << GenerateConversionFun("_Z14convert_uchar3Dv3_i", "%fun_uc3_u3",
                              "%v3uint", "%v3uchar", "OpUConvert");

  OS << GenerateConversionFun("_Z14convert_uchar4Dv4_j", "%fun_uc4_u4",
                              "%v4uint", "%v4uchar", "OpUConvert");

  OS << GenerateConversionFun("_Z13convert_uint4Dv4_h", "%fun_u4_uc4",
                              "%v4uchar", "%v4uint", "OpUConvert");

  OS << GenerateEISFun("_Z3sinf", "%fun_f_f", "%float", {"%float"}, "Sin");
  OS << GenerateEISFun("_Z4sqrtf", "%fun_f_f", "%float", {"%float"}, "Sqrt");
  OS << GenerateEISFun("_Z10native_expf", "%fun_f_f", "%float", {"%float"},
                       "Exp");
  OS << GenerateEISFun("_Z3maxii", "%fun_u_uu", "%uint", {"%uint", "%uint"},
                       "SMax");
  OS << GenerateEISFun("_Z3minii", "%fun_u_uu", "%uint", {"%uint", "%uint"},
                       "SMin");
  OS << GenerateEISFun("_Z3maxff", "%fun_f_ff", "%float", {"%float", "%float"},
                       "FMax");
  OS << GenerateEISFun("_Z3minff", "%fun_f_ff", "%float", {"%float", "%float"},
                       "FMin");
  OS << GenerateEISFun("_Z5clampfff", "%fun_f_fff", "%float",
                       {"%float", "%float", "%float"}, "FClamp");
  OS << GenerateEISFun("_Z5clampiii", "%fun_u_uuu", "%uint",
                       {"%uint", "%uint", "%uint"}, "SClamp");

  OS << R"(
%rs_linker__Z3dotDv2_fS_ = OpFunction %float Pure %fun_f_f2f2
%param_Z3dotDv2_fS_0 = OpFunctionParameter %v2float
%param_Z3dotDv2_fS_1 = OpFunctionParameter %v2float
%label_Z3dotDv2_fS = OpLabel
%res_Z3dotDv2_fS = OpDot %float %param_Z3dotDv2_fS_0 %param_Z3dotDv2_fS_1
      OpReturnValue %res_Z3dotDv2_fS
      OpFunctionEnd
)";

  OS << R"(
%rs_linker__Z3dotDv3_fS_ = OpFunction %float Pure %fun_f_f3f3
%param_Z3dotDv3_fS_0 = OpFunctionParameter %v3float
%param_Z3dotDv3_fS_1 = OpFunctionParameter %v3float
%label_Z3dotDv3_fS = OpLabel
%res_Z3dotDv3_fS = OpDot %float %param_Z3dotDv3_fS_0 %param_Z3dotDv3_fS_1
      OpReturnValue %res_Z3dotDv3_fS
      OpFunctionEnd
)";

  OS << R"(
%rs_linker_rsUnpackColor8888 = OpFunction %v4float Pure %fun_f4_uc4
%paramrsUnpackColor88880 = OpFunctionParameter %v4uchar
%labelrsUnpackColor8888 = OpLabel
%castedUnpackColor8888 = OpBitcast %uint %paramrsUnpackColor88880
%resrsUnpackColor8888 = OpExtInst %v4float %glsl_ext_ins UnpackUnorm4x8 %castedUnpackColor8888
      OpReturnValue %resrsUnpackColor8888
      OpFunctionEnd
)";

  OS << R"(
%rs_linker__Z17rsPackColorTo8888Dv4_f = OpFunction %v4uchar Pure %fun_uc4_f4
%param_Z17rsPackColorTo8888Dv4_f0 = OpFunctionParameter %v4float
%label_Z17rsPackColorTo8888Dv4_f = OpLabel
%res_Z17rsPackColorTo8888Dv4_f = OpExtInst %uint %glsl_ext_ins PackUnorm4x8 %param_Z17rsPackColorTo8888Dv4_f0
%casted_Z17rsPackColorTo8888Dv4_f = OpBitcast %v4uchar %res_Z17rsPackColorTo8888Dv4_f
      OpReturnValue %casted_Z17rsPackColorTo8888Dv4_f
      OpFunctionEnd
)";

  OS << R"(
%rs_linker__Z5clampDv3_fff = OpFunction %v3float Pure %fun_f3_f3ff
%param_Z5clampDv3_fff0 = OpFunctionParameter %v3float
%param_Z5clampDv3_fff1 = OpFunctionParameter %float
%param_Z5clampDv3_fff2 = OpFunctionParameter %float
%label_Z5clampDv3_fff = OpLabel
%arg1_Z5clampDv3_fff = OpCompositeConstruct %v3float %param_Z5clampDv3_fff1 %param_Z5clampDv3_fff1 %param_Z5clampDv3_fff1
%arg2_Z5clampDv3_fff = OpCompositeConstruct %v3float %param_Z5clampDv3_fff2 %param_Z5clampDv3_fff2 %param_Z5clampDv3_fff2
%res_Z5clampDv3_fff = OpExtInst %v3float %glsl_ext_ins FClamp %param_Z5clampDv3_fff0 %arg1_Z5clampDv3_fff %arg2_Z5clampDv3_fff
      OpReturnValue %res_Z5clampDv3_fff
      OpFunctionEnd
)";

  OS << R"(
%rs_linker__Z5clampDv3_iii = OpFunction %v3uint Pure %fun_u3_u3uu
%param_Z5clampDv3_iii0 = OpFunctionParameter %v3uint
%param_Z5clampDv3_iii1 = OpFunctionParameter %uint
%param_Z5clampDv3_iii2 = OpFunctionParameter %uint
%label_Z5clampDv3_iii = OpLabel
%arg1_Z5clampDv3_iii = OpCompositeConstruct %v3uint %param_Z5clampDv3_iii1 %param_Z5clampDv3_iii1 %param_Z5clampDv3_iii1
%arg2_Z5clampDv3_iii = OpCompositeConstruct %v3uint %param_Z5clampDv3_iii2 %param_Z5clampDv3_iii2 %param_Z5clampDv3_iii2
%res_Z5clampDv3_iii = OpExtInst %v3uint %glsl_ext_ins UClamp %param_Z5clampDv3_iii0 %arg1_Z5clampDv3_iii %arg2_Z5clampDv3_iii
      OpReturnValue %res_Z5clampDv3_iii
      OpFunctionEnd
)";
}

bool ReflectionPass::emitRSAllocFunctions(
    Module &M, const SmallVectorImpl<RSAllocationInfo> &RSAllocs,
    const SmallVectorImpl<RSAllocationCallInfo> &RSAllocAccesses) {
  DEBUG(dbgs() << "emitRSAllocFunctions\n");

  for (const auto &Access : RSAllocAccesses) {
    solidifyRSAllocAccess(M, Access);

    auto *Fun = Access.FCall->getCalledFunction();
    if (!Fun)
      return false;

    const auto FName = Fun->getName();
    auto *ETMapping = getMappingOrPrintError(Access.RSElementTy);
    if (!ETMapping)
      return false;

    const auto ElementTy = ETMapping->SPIRVTy;
    const std::string LoadName = Access.RSAlloc.VarName + "_load";

    if (Access.Kind == RSAllocAccessKind::GEA)
      OS << GenerateRSGEA(FName.str().c_str(), ElementTy.c_str(),
                          LoadName.c_str(), Coords::XY);
    else
      OS << GenerateRSSEA(FName.str().c_str(), LoadName.c_str(), Coords::XY);
  }

  return true;
}

bool ReflectionPass::emitMainUsingBuffersForInputOutput(
    const KernelSignature &Kernel,
    const SmallVectorImpl<RSAllocationInfo> &RSAllocs,
    const std::string &inputBuffer, const std::string &outputBuffer) {
  const auto *RTMapping = getMappingOrPrintError(Kernel.returnType);
  if (!RTMapping) {
    return false;
  }
  const auto &RetTy = RTMapping->SPIRVTy;

  const auto *ArgTMapping = getMappingOrPrintError(Kernel.argumentTypes[0]);
  if (!ArgTMapping) {
    return false;
  }
  const auto &ArgTy = ArgTMapping->SPIRVTy;

#define TMP(X) (Kernel.getTempName(#X))

  OS << Kernel.getWrapperName() << " = OpFunction %void None %fun_void\n";
  OS << TMP(label) << " = OpLabel\n";
  OS << TMP(coords_load) << " = OpLoad %v3uint %gl_GlobalInvocationID\n";
  OS << TMP(coords_x) << " = OpCompositeExtract %uint " << TMP(coords_load)
     << " 0\n";
  OS << TMP(coords_y) << " = OpCompositeExtract %uint " << TMP(coords_load)
     << " 1\n";
  OS << TMP(coords_z) << " = OpCompositeExtract %uint " << TMP(coords_load)
     << " 2\n";
  OS << TMP(res) << " = OpVariable " << TMP(ptr_function_ty) << " Function\n";

  for (const auto &A : RSAllocs)
    OS << A.VarName << "_load = OpLoad " << A.VarName << "_image_ty "
       << A.VarName << "_var\n";

  OS << TMP(tmp1) << " = OpIMul %uint " << TMP(coords_y) << " %group_size_x\n";
  OS << TMP(tmp2)
     << " = OpAccessChain %_ptr_Input_uint %gl_NumWorkGroups %uint_zero\n";
  OS << TMP(tmp3) << " = OpLoad %uint " << TMP(tmp2) << "\n";
  OS << TMP(tmp4) << " = OpIMul %uint " << TMP(tmp1) << " " << TMP(tmp3) << "\n";
  OS << TMP(tmp5) << " = OpIAdd %uint " << TMP(tmp4) << " " << TMP(coords_x)
     << "\n";
  OS << TMP(tmp6) << " = OpAccessChain " << TMP(ptr_function_ty) << " "
     << inputBuffer << " %uint_zero " << TMP(tmp5) << "\n";
  OS << TMP(inputPixel) << " = OpLoad " << ArgTy << " " << TMP(tmp6) << "\n";

  OS << TMP(tmp7) << " = OpFunctionCall " << RetTy << " %RS_SPIRV_DUMMY_ "
     << TMP(inputPixel);
  const auto CoordsNum = size_t(Kernel.coordsKind);
  for (size_t i = 0; i != CoordsNum; ++i)
    OS << " " << TMP(coords_) << CoordsNames[i].str();
  OS << "\n";

  OS << "OpStore " << TMP(res) << " " << TMP(tmp7) << "\n";
  OS << TMP(tmp8) << " = OpLoad " << RetTy << " " << TMP(res) << "\n";
  OS << TMP(tmp9) << " = OpAccessChain " << TMP(ptr_function_ty) << " "
     << outputBuffer << " %uint_zero " << TMP(tmp5) << "\n";
  OS << "OpStore " << TMP(tmp9) << " " << TMP(tmp8) << "\n";
  OS << R"(
    OpReturn
    OpFunctionEnd
)";
#undef TMP

  return true;
}

} // namespace rs2spirv
