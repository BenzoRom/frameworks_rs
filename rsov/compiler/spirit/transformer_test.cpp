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

#include "transformer.h"

#include <stdint.h>

#include "file_utils.h"
#include "spirit.h"
#include "test_utils.h"
#include "word_stream.h"
#include "gtest/gtest.h"

namespace android {
namespace spirit {

namespace {

class MulToAddTransformer : public Transformer {
public:
  Instruction *transform(IMulInst *mul) override {
    auto ret = new IAddInst(mul->mResultType, mul->mOperand1, mul->mOperand2);
    ret->setId(mul->getId());
    return ret;
  }
};

class Deleter : public Transformer {
public:
  Instruction *transform(IMulInst *) override { return nullptr; }
};

} // annonymous namespace

class TransformerTest : public ::testing::Test {
protected:
  virtual void SetUp() { mWordsGreyscale = readWords("greyscale.spv"); }

  std::vector<uint32_t> mWordsGreyscale;

private:
  std::vector<uint32_t> readWords(const char *testFile) {
    static const std::string testDataPath(
        "frameworks/rs/rsov/compiler/spirit/test_data/");
    const std::string &fullPath = getAbsolutePath(testDataPath + testFile);
    return readFile<uint32_t>(fullPath);
  }
};

TEST_F(TransformerTest, testMulToAdd) {
  std::unique_ptr<InputWordStream> IS(InputWordStream::Create(mWordsGreyscale));
  std::unique_ptr<Module> m(Deserialize<Module>(*IS));

  ASSERT_NE(nullptr, m);

  EXPECT_EQ(1, countEntity<IAddInst>(m.get()));
  EXPECT_EQ(1, countEntity<IMulInst>(m.get()));

  MulToAddTransformer trans;
  std::unique_ptr<Module> m1(trans.applyTo(m.get()));

  ASSERT_NE(nullptr, m1);

  ASSERT_TRUE(m1->resolveIds());

  EXPECT_EQ(2, countEntity<IAddInst>(m1.get()));
  EXPECT_EQ(0, countEntity<IMulInst>(m1.get()));
}

TEST_F(TransformerTest, testDeletion) {
  std::unique_ptr<InputWordStream> IS(InputWordStream::Create(mWordsGreyscale));
  std::unique_ptr<Module> m(Deserialize<Module>(*IS));

  ASSERT_NE(nullptr, m.get());

  EXPECT_EQ(1, countEntity<IMulInst>(m.get()));

  Deleter trans;
  std::unique_ptr<Module> m1(trans.applyTo(m.get()));

  ASSERT_NE(nullptr, m1.get());

  EXPECT_EQ(1, countEntity<IAddInst>(m1.get()));
  EXPECT_EQ(0, countEntity<IMulInst>(m1.get()));
}

} // namespace spirit
} // namespace android
