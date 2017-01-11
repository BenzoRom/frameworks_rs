/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "rsovScript.h"

#include "bcinfo/MetadataExtractor.h"
#include "rsContext.h"
#include "rsType.h"
#include "rsUtils.h"
#include "rsovAllocation.h"
#include "rsovContext.h"
#include "rsovCore.h"

#include <fstream>

namespace android {
namespace renderscript {
namespace rsov {

namespace {

const char *COMPILER_EXE_PATH = "/system/bin/bcc_rsov";

std::vector<const char *> setCompilerArgs(const char *bcFileName,
                                          const char *cacheDir) {
  rsAssert(bcFileName && cacheDir);

  std::vector<const char *> args;

  args.push_back(COMPILER_EXE_PATH);
  args.push_back(bcFileName);

  args.push_back(nullptr);
  return args;
}

void writeBytes(const char *filename, const char *bytes, size_t size) {
  std::ofstream ofs(filename, std::ios::binary);
  ofs.write(bytes, size);
  ofs.close();
}

std::vector<uint32_t> readWords(const char *filename) {
  std::ifstream ifs(filename, std::ios::binary);

  ifs.seekg(0, ifs.end);
  int length = ifs.tellg();
  ifs.seekg(0, ifs.beg);

  rsAssert(((length & 3) == 0) && "File size expected to be multiples of 4");

  std::vector<uint32_t> spvWords(length / sizeof(uint32_t));

  ifs.read((char *)(spvWords.data()), length);

  ifs.close();

  return spvWords;
}

std::vector<uint32_t> compileBitcode(const char *resName, const char *cacheDir,
                                     const char *bitcode, size_t bitcodeSize) {
  rsAssert(bitcode && bitcodeSize);

  // TODO: Cache the generated code

  std::string bcFileName(cacheDir);
  bcFileName.append("/");
  bcFileName.append(resName);
  bcFileName.append(".bc");

  writeBytes(bcFileName.c_str(), bitcode, bitcodeSize);

  auto args = setCompilerArgs(bcFileName.c_str(), cacheDir);

  if (!rsuExecuteCommand(COMPILER_EXE_PATH, args.size() - 1, args.data())) {
    ALOGE("compiler command line failed");
    return std::vector<uint32_t>();
  }

  ALOGV("compiler command line succeeded");

  std::string spvFileName(cacheDir);
  spvFileName.append("/");
  spvFileName.append(resName);
  spvFileName.append(".spv");

  return readWords(spvFileName.c_str());
}

}  // anonymous namespace

RSoVScript::RSoVScript(RSoVContext *context, std::vector<uint32_t> &&spvWords,
                       bcinfo::MetadataExtractor *ME)
    : mRSoV(context),
      mDevice(context->getDevice()),
      mSPIRVWords(std::move(spvWords)),
      mME(ME) {}

RSoVScript::~RSoVScript() {
  delete mCpuScript;
  delete mME;
  // TODO: destroy shader
}

void RSoVScript::populateScript(Script *) {
  // TODO: implement this
}

void RSoVScript::invokeFunction(uint32_t slot, const void *params,
                                size_t paramLength) {
  // TODO: implement this
}

int RSoVScript::invokeRoot() {
  // TODO: implement this
  return 0;
}

void RSoVScript::invokeForEach(uint32_t slot, const Allocation **ains,
                               uint32_t inLen, Allocation *aout,
                               const void *usr, uint32_t usrLen,
                               const RsScriptCall *sc) {
  // TODO: Handle kernel without input Allocation
  // TODO: Handle multi-input kernel
  rsAssert(ains && inLen == 1);

  RSoVAllocation *inputAllocation =
      static_cast<RSoVAllocation *>(ains[0]->mHal.drv);
  RSoVAllocation *outputAllocation =
      static_cast<RSoVAllocation *>(aout->mHal.drv);
  runForEach(slot, inputAllocation, outputAllocation);
}

void RSoVScript::invokeReduce(uint32_t slot, const Allocation **ains,
                              uint32_t inLen, Allocation *aout,
                              const RsScriptCall *sc) {
  // TODO: implement this
}

void RSoVScript::invokeInit() {
  // TODO: implement this
}

void RSoVScript::invokeFreeChildren() {
  // TODO: implement this
}

void RSoVScript::setGlobalVar(uint32_t slot, const void *data,
                              size_t dataLength) {
  // TODO: implement this
}

void RSoVScript::getGlobalVar(uint32_t slot, void *data, size_t dataLength) {
  // TODO: implement this
}

void RSoVScript::setGlobalVarWithElemDims(uint32_t slot, const void *data,
                                          size_t dataLength, const Element *e,
                                          const uint32_t *dims,
                                          size_t dimLength) {
  // TODO: implement this
}

void RSoVScript::setGlobalBind(uint32_t slot, Allocation *data) {
  // TODO: implement this
}

void RSoVScript::setGlobalObj(uint32_t slot, ObjectBase *obj) {
  // TODO: implement this
}

Allocation *RSoVScript::getAllocationForPointer(const void *ptr) const {
  // TODO: implement this
  return nullptr;
}

int RSoVScript::getGlobalEntries() const {
  // TODO: implement this
  return 0;
}

const char *RSoVScript::getGlobalName(int i) const {
  // TODO: implement this
  return nullptr;
}

const void *RSoVScript::getGlobalAddress(int i) const {
  // TODO: implement this
  return nullptr;
}

size_t RSoVScript::getGlobalSize(int i) const {
  // TODO: implement this
  return 0;
}

uint32_t RSoVScript::getGlobalProperties(int i) const {
  // TODO: implement this
  return 0;
}

void RSoVScript::InitDescriptorAndPipelineLayouts() {
  VkDescriptorSetLayoutBinding layout_bindings[] = {
      {
          .binding = 2,
          .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
          .descriptorCount = 1,
          .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
          .pImmutableSamplers = nullptr,
      },
      {
          .binding = 1,
          .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
          .descriptorCount = 1,
          .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
          .pImmutableSamplers = nullptr,
      },
#ifdef SUPPORT_GLOBAL_VARIABLES
      {
          .binding = 0,
          .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
          .descriptorCount = 1,
          .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
          .pImmutableSamplers = nullptr,
      }
#endif
  };

  VkDescriptorSetLayoutCreateInfo descriptor_layout = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .bindingCount = NELEM(layout_bindings),
      .pBindings = layout_bindings,
  };

  VkResult res;

  mDescLayout.resize(NUM_DESCRIPTOR_SETS);
  res = vkCreateDescriptorSetLayout(mDevice, &descriptor_layout, NULL,
                                    mDescLayout.data());
  if (res != VK_SUCCESS) {
    __android_log_print(ANDROID_LOG_ERROR, "ComputeTest",
                        "vkCreateDescriptorSetLayout() returns %d", res);
  }
  rsAssert(res == VK_SUCCESS);
  /*
    VkPushConstantRange pushConstantRange[] = { {
    .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
    .offset = 0,
    .size = 16
    } };
  */
  /* Now use the descriptor layout to create a pipeline layout */
  VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .pNext = nullptr,
      .pushConstantRangeCount = 0,
      .pPushConstantRanges = nullptr,
      .setLayoutCount = NUM_DESCRIPTOR_SETS,
      .pSetLayouts = mDescLayout.data(),
  };

  res = vkCreatePipelineLayout(mDevice, &pPipelineLayoutCreateInfo, NULL,
                               &mPipelineLayout);
  rsAssert(res == VK_SUCCESS);

  ALOGV("%s succeeded.", __FUNCTION__);
}

void RSoVScript::InitShader(uint32_t slot) {
  VkResult res;

  mShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  mShaderStage.pNext = nullptr;
  mShaderStage.pSpecializationInfo = nullptr;
  mShaderStage.flags = 0;
  mShaderStage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
  const char **RSKernelNames = mME->getExportForEachNameList();
  size_t RSKernelNum = mME->getExportForEachSignatureCount();
  rsAssert(slot < RSKernelNum);
  rsAssert(RSKernelNames);
  rsAssert(RSKernelNames[slot]);
  ALOGV("slot = %d kernel name = %s", slot, RSKernelNames[slot]);
  mShaderStage.pName = RSKernelNames[slot];

  VkShaderModuleCreateInfo moduleCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .codeSize = mSPIRVWords.size() * sizeof(unsigned int),
      .pCode = mSPIRVWords.data(),
  };
  res = vkCreateShaderModule(mDevice, &moduleCreateInfo, NULL,
                             &mShaderStage.module);
  rsAssert(res == VK_SUCCESS);

  ALOGV("%s succeeded.", __FUNCTION__);
}

void RSoVScript::InitDescriptorPool() {
  /* DEPENDS on InitDescriptorAndPipelineLayouts() */

  VkResult res;
  VkDescriptorPoolSize type_count[] = {
      {
          .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, .descriptorCount = 1,
      },
      {
          .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, .descriptorCount = 1,
      },
#ifdef SUPPORT_GLOBAL_VARIABLES
      {
          .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, .descriptorCount = 1,
      }
#endif
  };

  VkDescriptorPoolCreateInfo descriptor_pool = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .pNext = nullptr,
      .maxSets = 1,
      .poolSizeCount = NELEM(type_count),
      .pPoolSizes = type_count,
  };

  res = vkCreateDescriptorPool(mDevice, &descriptor_pool, NULL, &mDescPool);
  rsAssert(res == VK_SUCCESS);

  ALOGV("%s succeeded.", __FUNCTION__);
}

void RSoVScript::InitDescriptorSet(const RSoVAllocation *inputAllocation,
                                   RSoVAllocation *outputAllocation) {
  VkResult res;

  VkDescriptorSetAllocateInfo alloc_info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .pNext = NULL,
      .descriptorPool = mDescPool,
      .descriptorSetCount = NUM_DESCRIPTOR_SETS,
      .pSetLayouts = mDescLayout.data(),
  };

  mDescSet.resize(NUM_DESCRIPTOR_SETS);
  res = vkAllocateDescriptorSets(mDevice, &alloc_info, mDescSet.data());
  rsAssert(res == VK_SUCCESS);

  const VkWriteDescriptorSet writes[] = {
      {
          .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
          .dstSet = mDescSet[0],
          .dstBinding = 2,
          .dstArrayElement = 0,
          .descriptorCount = 1,
          .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
          .pBufferInfo = inputAllocation->getBufferInfo(),
      },
      {
          .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
          .dstSet = mDescSet[0],
          .dstBinding = 1,
          .dstArrayElement = 0,
          .descriptorCount = 1,
          .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
          .pBufferInfo = outputAllocation->getBufferInfo(),
      },
#ifdef SUPPORT_GLOBAL_VARIABLES
      {
          .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
          .dstSet = mDescSet[0],
          .dstBinding = 0,
          .dstArrayElement = 0,
          .descriptorCount = 1,
          .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
          .pBufferInfo = somebuffer_info,
      },
#endif
  };

  vkUpdateDescriptorSets(mDevice, NELEM(writes), writes, 0, NULL);

  ALOGV("%s succeeded.", __FUNCTION__);
}

void RSoVScript::InitPipeline() {
  // DEPENDS on mShaderStage, i.e., InitShader()

  VkResult res;

  VkComputePipelineCreateInfo pipeline_info = {
      .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
      .pNext = nullptr,
      .layout = mPipelineLayout,
      .basePipelineHandle = VK_NULL_HANDLE,
      .basePipelineIndex = 0,
      .flags = 0,
      .stage = mShaderStage,
  };
  res = vkCreateComputePipelines(mDevice, VK_NULL_HANDLE, 1, &pipeline_info,
                                 NULL, &mComputePipeline);
  rsAssert(res == VK_SUCCESS);

  ALOGV("%s succeeded.", __FUNCTION__);
}

void RSoVScript::runForEach(uint32_t slot,
                            const RSoVAllocation *inputAllocation,
                            RSoVAllocation *outputAllocation) {
  VkResult res;

  InitDescriptorAndPipelineLayouts();
  InitShader(slot);
  InitDescriptorPool();
  InitDescriptorSet(inputAllocation, outputAllocation);
  // InitPipelineCache();
  InitPipeline();

  VkCommandBuffer cmd;

  VkCommandBufferAllocateInfo cmd_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .pNext = nullptr,
      .commandPool = mRSoV->getCmdPool(),
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1,
  };

  res = vkAllocateCommandBuffers(mDevice, &cmd_info, &cmd);
  rsAssert(res == VK_SUCCESS);

  VkCommandBufferBeginInfo cmd_buf_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .pNext = nullptr,
      .flags = 0,
      .pInheritanceInfo = nullptr,
  };

  res = vkBeginCommandBuffer(cmd, &cmd_buf_info);
  rsAssert(res == VK_SUCCESS);

  vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, mComputePipeline);

  vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, mPipelineLayout,
                          0, mDescSet.size(), mDescSet.data(), 0, nullptr);

  const uint32_t width = inputAllocation->getWidth();
  const uint32_t height = rsMax(inputAllocation->getHeight(), 1U);
  const uint32_t depth = rsMax(inputAllocation->getDepth(), 1U);
  vkCmdDispatch(cmd, width, height, depth);

  res = vkEndCommandBuffer(cmd);
  assert(res == VK_SUCCESS);

  VkSubmitInfo submit_info = {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .commandBufferCount = 1,
      .pCommandBuffers = &cmd,
  };

  VkFence fence;

  VkFenceCreateInfo fenceInfo = {
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
  };

  vkCreateFence(mDevice, &fenceInfo, NULL, &fence);

  vkQueueSubmit(mRSoV->getQueue(), 1, &submit_info, fence);

  // Make sure command buffer is finished
  do {
    res = vkWaitForFences(mDevice, 1, &fence, VK_TRUE, 100000);
  } while (res == VK_TIMEOUT);

  rsAssert(res == VK_SUCCESS);

  vkDestroyFence(mDevice, fence, NULL);

  // TODO: shall we reuse command buffers?
  VkCommandBuffer cmd_bufs[] = {cmd};
  vkFreeCommandBuffers(mDevice, mRSoV->getCmdPool(), 1, cmd_bufs);

  vkDestroyPipeline(mDevice, mComputePipeline, nullptr);
  for (int i = 0; i < NUM_DESCRIPTOR_SETS; i++)
    vkDestroyDescriptorSetLayout(mDevice, mDescLayout[i], nullptr);
  vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);
  vkDestroyDescriptorPool(mDevice, mDescPool, nullptr);
  vkDestroyShaderModule(mDevice, mShaderStage.module, nullptr);

  ALOGV("%s succeeded.", __FUNCTION__);
}

}  // namespace rsov
}  // namespace renderscript
}  // namespace android

using android::renderscript::Allocation;
using android::renderscript::Context;
using android::renderscript::Element;
using android::renderscript::ObjectBase;
using android::renderscript::RsdCpuReference;
using android::renderscript::Script;
using android::renderscript::ScriptC;
using android::renderscript::rs_script;
using android::renderscript::rsov::RSoVContext;
using android::renderscript::rsov::RSoVScript;
using android::renderscript::rsov::compileBitcode;

bool rsovScriptInit(const Context *rsc, ScriptC *script, char const *resName,
                    char const *cacheDir, uint8_t const *bitcode,
                    size_t bitcodeSize, uint32_t flags) {
  std::vector<uint32_t> &&spvWords =
      compileBitcode(resName, cacheDir, (const char *)bitcode, bitcodeSize);

  if (spvWords.empty()) {
    ALOGE("compilation failed for script %s", resName);
    return false;
  }

  RSoVHal *hal = static_cast<RSoVHal *>(rsc->mHal.drv);
  RSoVContext *rsov = hal->mRSoV;

  std::unique_ptr<RsdCpuReference::CpuScript> cs(hal->mCpuRef->createScript(
      script, resName, cacheDir, bitcode, bitcodeSize, flags));
  if (cs == nullptr) {
    return false;
  }
  cs->populateScript(script);

  bcinfo::MetadataExtractor *bitcodeMetadata =
      new bcinfo::MetadataExtractor((const char *)bitcode, bitcodeSize);
  if (!bitcodeMetadata->extract()) {
    ALOGE("Could not extract metadata from bitcode");
    return false;
  }

  RSoVScript *rsovScript =
      new RSoVScript(rsov, std::move(spvWords), bitcodeMetadata);

  if (!rsovScript) {
    ALOGV("Failed creating a RSoV script");
    // Uncomment below to choose CPU driver instead
    // script->mHal.drv = cs;
    return false;
  }

  rsovScript->setCpuScript(cs.release());
  script->mHal.drv = rsovScript;

  return true;
}

bool rsovInitIntrinsic(const Context *rsc, Script *s, RsScriptIntrinsicID iid,
                       Element *e) {
  RSoVHal *dc = (RSoVHal *)rsc->mHal.drv;
  RsdCpuReference::CpuScript *cs = dc->mCpuRef->createIntrinsic(s, iid, e);
  if (cs == nullptr) {
    return false;
  }
  s->mHal.drv = cs;
  cs->populateScript(s);
  return true;
}

void rsovScriptInvokeForEach(const Context *rsc, Script *s, uint32_t slot,
                             const Allocation *ain, Allocation *aout,
                             const void *usr, size_t usrLen,
                             const RsScriptCall *sc) {
  if (ain == nullptr) {
    rsovScriptInvokeForEachMulti(rsc, s, slot, nullptr, 0, aout, usr, usrLen,
                                 sc);
  } else {
    const Allocation *ains[1] = {ain};

    rsovScriptInvokeForEachMulti(rsc, s, slot, ains, 1, aout, usr, usrLen, sc);
  }
}

void rsovScriptInvokeForEachMulti(const Context *rsc, Script *s, uint32_t slot,
                                  const Allocation **ains, size_t inLen,
                                  Allocation *aout, const void *usr,
                                  size_t usrLen, const RsScriptCall *sc) {
  RsdCpuReference::CpuScript *cs = (RsdCpuReference::CpuScript *)s->mHal.drv;
  cs->invokeForEach(slot, ains, inLen, aout, usr, usrLen, sc);
}

int rsovScriptInvokeRoot(const Context *dc, Script *s) {
  RsdCpuReference::CpuScript *cs = (RsdCpuReference::CpuScript *)s->mHal.drv;
  return cs->invokeRoot();
}

void rsovScriptInvokeInit(const Context *dc, Script *s) {
  RsdCpuReference::CpuScript *cs = (RsdCpuReference::CpuScript *)s->mHal.drv;
  cs->invokeInit();
}

void rsovScriptInvokeFreeChildren(const Context *dc, Script *s) {
  RsdCpuReference::CpuScript *cs = (RsdCpuReference::CpuScript *)s->mHal.drv;
  cs->invokeFreeChildren();
}

void rsovScriptInvokeFunction(const Context *dc, Script *s, uint32_t slot,
                              const void *params, size_t paramLength) {
  RsdCpuReference::CpuScript *cs = (RsdCpuReference::CpuScript *)s->mHal.drv;
  cs->invokeFunction(slot, params, paramLength);
}

void rsovScriptInvokeReduce(const Context *dc, Script *s, uint32_t slot,
                            const Allocation **ains, size_t inLen,
                            Allocation *aout, const RsScriptCall *sc) {
  RsdCpuReference::CpuScript *cs = (RsdCpuReference::CpuScript *)s->mHal.drv;
  cs->invokeReduce(slot, ains, inLen, aout, sc);
}

void rsovScriptSetGlobalVar(const Context *dc, const Script *s, uint32_t slot,
                            void *data, size_t dataLength) {
  RsdCpuReference::CpuScript *cs = (RsdCpuReference::CpuScript *)s->mHal.drv;
  cs->setGlobalVar(slot, data, dataLength);
}

void rsovScriptGetGlobalVar(const Context *dc, const Script *s, uint32_t slot,
                            void *data, size_t dataLength) {
  RsdCpuReference::CpuScript *cs = (RsdCpuReference::CpuScript *)s->mHal.drv;
  cs->getGlobalVar(slot, data, dataLength);
}

void rsovScriptSetGlobalVarWithElemDims(
    const Context *dc, const Script *s, uint32_t slot, void *data,
    size_t dataLength, const android::renderscript::Element *elem,
    const uint32_t *dims, size_t dimLength) {
  RsdCpuReference::CpuScript *cs = (RsdCpuReference::CpuScript *)s->mHal.drv;
  cs->setGlobalVarWithElemDims(slot, data, dataLength, elem, dims, dimLength);
}

void rsovScriptSetGlobalBind(const Context *dc, const Script *s, uint32_t slot,
                             Allocation *data) {
  RsdCpuReference::CpuScript *cs = (RsdCpuReference::CpuScript *)s->mHal.drv;
  cs->setGlobalBind(slot, data);
}

void rsovScriptSetGlobalObj(const Context *dc, const Script *s, uint32_t slot,
                            ObjectBase *data) {
  RsdCpuReference::CpuScript *cs = (RsdCpuReference::CpuScript *)s->mHal.drv;
  cs->setGlobalObj(slot, data);
}

void rsovScriptDestroy(const Context *dc, Script *s) {
  RsdCpuReference::CpuScript *cs = (RsdCpuReference::CpuScript *)s->mHal.drv;
  delete cs;
  s->mHal.drv = nullptr;
}

Allocation *rsovScriptGetAllocationForPointer(
    const android::renderscript::Context *dc,
    const android::renderscript::Script *sc, const void *ptr) {
  RsdCpuReference::CpuScript *cs = (RsdCpuReference::CpuScript *)sc->mHal.drv;
  return cs->getAllocationForPointer(ptr);
}

void rsovScriptUpdateCachedObject(const Context *rsc, const Script *script,
                                  rs_script *obj) {
  obj->p = script;
#ifdef __LP64__
  obj->r = nullptr;
  if (script != nullptr) {
    obj->v1 = script->mHal.drv;
  } else {
    obj->v1 = nullptr;
  }
  obj->v2 = nullptr;
#endif
}
