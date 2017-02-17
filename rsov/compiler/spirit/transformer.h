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

#ifndef TRANSFORMER_H
#define TRANSFORMER_H

#include <vector>

#include "instructions.h"
#include "visitor.h"
#include "word_stream.h"

namespace android {
namespace spirit {

class Transformer : public DoNothingVisitor {
public:
  Transformer() : mStream(WordStream::Create()) {}

  virtual ~Transformer() {}

  Module *applyTo(Module *m);
  std::vector<uint32_t> transformSerialize(Module *m);

  // Inserts a new instruction before the current instruction
  void insert(Instruction *);

#define HANDLE_INSTRUCTION(OPCODE, INST_CLASS)                                 \
  virtual Instruction *transform(INST_CLASS *inst) {                           \
    return static_cast<Instruction *>(inst);                                   \
  }                                                                            \
  virtual void visit(INST_CLASS *inst) {                                       \
    if (Instruction *transformed = transform(inst)) {                          \
      transformed->Serialize(*mStream);                                        \
    }                                                                          \
  }
#include "instruction_dispatches_generated.h"
#undef HANDLE_INSTRUCTION

private:
  std::unique_ptr<WordStream> mStream;
};

} // namespace spirit
} // namespace android

#endif // TRANSFORMER_H
