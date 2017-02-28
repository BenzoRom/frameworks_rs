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

#include "pass.h"

#include "module.h"
#include "word_stream.h"

namespace android {
namespace spirit {

Module *Pass::run(Module *module, int *error) {
  int intermediateError;
  auto words = runAndSerialize(module, &intermediateError);
  if (intermediateError) {
    if (error) {
      *error = intermediateError;
    }
    return nullptr;
  }
  std::unique_ptr<InputWordStream> IS(InputWordStream::Create(words));
  return Deserialize<Module>(*IS);
}

std::vector<uint32_t> Pass::runAndSerialize(Module *module, int *error) {
  int intermediateError;
  auto m1 = run(module, &intermediateError);
  if (intermediateError) {
    if (error) {
      *error = intermediateError;
    }
    return std::vector<uint32_t>();
  }
  std::unique_ptr<OutputWordStream> OS(OutputWordStream::Create());
  m1->Serialize(*OS);
  return OS->getWords();
}

} // namespace spirit
} // namespace android

