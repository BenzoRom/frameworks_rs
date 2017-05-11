# Copyright 2017, The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


"""Generates necessary files for unit test versions for testing.

After adding/changing RenderScript unit tests, run `python RSUnitTests.py`
to ensure that forward/backward compatibility tests run properly.

This file is so when updating/adding unit tests there is one central location
for managing the list of unit tests and their versions.

This is necessary since forward compatibility unit tests are chosen at compile
time where backward compatibility tests are chosen at runtime.

Generates a Java file for backward compatibility testing.
Generates an Android.mk file for forward compatibility testing.
"""


import sys
import os


# List of API versions and the tests that correspond to the API version.
# The test name must correspond to a UT_{}.java file.
# e.g. alloc -> UT_alloc.java
UNIT_TESTS = {
    19: [
        'alloc',
        'array_alloc',
        'array_init',
        'atomic',
        'bitfield',
        'bug_char',
        'check_dims',
        'clamp_relaxed',
        'clamp',
        'constant',
        'convert_relaxed',
        'convert',
        'copy_test',
        'element',
        'foreach',
        'foreach_bounds',
        'fp_mad',
        'int4',
        'kernel',
        'kernel_struct',
        'math',
        'min',
        'noroot',
        'primitives',
        'refcount',
        'rsdebug',
        'rstime',
        'rstypes',
        'sampler',
        'static_globals',
        'struct_field_simple',
        'struct',
        'unsigned',
        'vector',
    ],

    21: [
        'foreach_multi',
        'math_agree',
        'math_conformance',
    ],

    23: [
        'ctxt_default',
        'kernel2d',
        'kernel2d_oldstyle',
        'kernel3d',
        'rsdebug_23',
        'script_group2_gatherscatter',
        'script_group2_nochain',
        'script_group2_pointwise',
    ],

    24: [
        'fp16',
        'fp16_globals',
        'math_24',
        'math_fp16',
        'reduce_backward',
        'reduce',
        'rsdebug_24',
        'script_group2_float',
        'single_source_alloc',
        'single_source_ref_count',
        'single_source_script',
        'small_struct',
        'small_struct_2',
    ],

    26: [
        'blur_validation',
        'struct_field',
    ],
}


# Dictionary mapping unit tests to the corresponding needed .rs files.
# Only needed if UT_{}.java does not map to {}.rs.
UNIT_TEST_RS_FILES_OVERRIDE = {
    'blur_validation': [],
    'script_group2_float': ['float_test.rs'],
    'script_group2_gatherscatter': ['addup.rs'],
    'script_group2_nochain': ['increment.rs', 'increment2.rs', 'double.rs'],
    'script_group2_pointwise': ['increment.rs', 'double.rs'],
}


# List of API versions and the corresponding build tools release version
BUILD_TOOL_VERSIONS = {
    # TODO(aeubanks): either remove or uncomment
    # after figuring out what to do with 19 and 20.
    #19: '19.1.0',
    #20: '20.0.0',
    21: '21.1.2',
    22: '22.0.1',
    23: '23.0.3',
    24: '24.0.3',
    25: '25.0.2',
}


def ThisScriptDir():
  """Returns the directory this script is in."""
  return os.path.dirname(os.path.realpath(__file__))


def WriteMakeCopyright(gen_file):
  """Writes the copyright for a Makefile to a file."""
  gen_file.write(
      '#\n'
      '# Copyright (C) 2017 The Android Open Source Project\n'
      '#\n'
      '# Licensed under the Apache License, Version 2.0 (the "License");\n'
      '# you may not use this file except in compliance with the License.\n'
      '# You may obtain a copy of the License at\n'
      '#\n'
      '#      http://www.apache.org/licenses/LICENSE-2.0\n'
      '#\n'
      '# Unless required by applicable law or agreed to in writing, software\n'
      '# distributed under the License is distributed on an "AS IS" BASIS,\n'
      '# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n'
      '# See the License for the specific language governing permissions and\n'
      '# limitations under the License.\n'
      '#\n\n'
  )


def WriteMakeSrcFiles(gen_file, api_version):
  """Writes applicable LOCAL_SRC_FILES to gen_file.

  Includes everything under ./src, base UnitTest class, and test files.

  api_version: only tests that can run on this version are added."""
  # Get all tests compatible with the build tool version
  # Compatible means build tool version >= test version
  tests = []
  for test_version, tests_for_version in UNIT_TESTS.iteritems():
    if api_version >= test_version:
      tests.extend(tests_for_version)
  tests = sorted(tests)
  gen_file.write(
      'LOCAL_SRC_FILES := $(call all-java-files-under,src)\\\n'
      '    $(my_rs_unit_tests_path)/UnitTest.java\\\n'
  )
  for test in tests:
    # Add the Java and corresponding rs files to LOCAL_SRC_FILES
    gen_file.write(
        '    $(my_rs_unit_tests_path)/UT_{}.java\\\n'
        .format(test)
    )
    for rs_file in RSFilesForUnitTest(test):
      gen_file.write(
          '    $(my_rs_unit_tests_path)/{}\\\n'.format(
              rs_file
          )
      )



def ForwardMakefileLocation():
  """Returns the location of the Makefile for forward compatibility testing."""
  return os.path.join(ThisScriptDir(), '..', 'RSTestForward', 'Android.mk')


def RSFilesForUnitTest(test):
  """Returns a list of .rs files associated with a test."""
  if test in UNIT_TEST_RS_FILES_OVERRIDE:
    return UNIT_TEST_RS_FILES_OVERRIDE[test]
  else:
    # Default is one .rs file with the same name as the input.
    return ['{}.rs'.format(test)]


def WriteForwardMakefile(gen_file):
  """Writes the Makefile for forward compatibility testing.

  Makefile contains a build target per build tool version
  for forward compatibility testing based on the unit test list at the
  top of this file."""
  WriteMakeCopyright(gen_file)
  gen_file.write(
      '# This file is auto-generated by frameworks/rs/tests/java_api/RSUnitTests/RSUnitTests.py.\n'
      '# To change unit tests version, please run the Python script above.\n\n'
      'ifneq ($(ENABLE_RSTESTFORWARD),)\n\n'
      'LOCAL_PATH := $(call my-dir)\n'
      'my_rs_test_forward_targets :=\n'
      'my_rs_unit_tests_path := ../RSUnitTests/src/com/android/rs/unittest\n'
  )
  all_make_target_names = []
  for build_tool_version in sorted(BUILD_TOOL_VERSIONS.keys()):
    build_tool_version_name = BUILD_TOOL_VERSIONS[build_tool_version]
    make_target_name = 'RSTestForward_{}'.format(build_tool_version_name)
    make_target_name = make_target_name.replace('.', '_')
    all_make_target_names.append(make_target_name)
    gen_file.write(
        '\n'
        '# RSTestForward for build tool version {}\n\n'
        'include $(CLEAR_VARS)\n\n'
        'LOCAL_MODULE_TAGS := tests\n'
        'LOCAL_STATIC_JAVA_LIBRARIES := android-support-test\n'
        'LOCAL_COMPATIBILITY_SUITE := device-tests\n'
        'LOCAL_RENDERSCRIPT_TARGET_API := 0\n'
        'LOCAL_PACKAGE_NAME := {}\n'
        'my_rs_path := $(TOP)/prebuilts/renderscript/host/linux-x86/{}\n'
        'LOCAL_RENDERSCRIPT_CC := $(my_rs_path)/bin/llvm-rs-cc\n'
        'LOCAL_RENDERSCRIPT_INCLUDES_OVERRIDE := $(my_rs_path)/include $(my_rs_path)/clang-include\n'
        'my_rs_path :=\n'.format(
            build_tool_version_name, make_target_name, build_tool_version_name
        )
    )
    WriteMakeSrcFiles(gen_file, build_tool_version)
    # With dist-for-goals, copy the target to DIST_DIR with 'make dist'
    gen_file.write(
        '\n'
        'include $(BUILD_PACKAGE)\n'
        '$(call dist-for-goals,RSTestForward,$(LOCAL_BUILT_MODULE):$(LOCAL_PACKAGE_NAME).apk)\n'
        'my_rs_test_forward_targets += $(LOCAL_BUILT_MODULE)\n'
    )
  # Create RSTestForward phony target which builds all above targets
  gen_file.write(
      '\n'
      '.PHONY: RSTestForward\n'
      'RSTestForward: $(my_rs_test_forward_targets)\n'
      'my_rs_unit_tests_path :=\n'
      'my_rs_test_forward_targets :=\n\n'
      'endif\n'.format(' '.join(all_make_target_names))
  )


def GenerateForward():
  """Generates the necessary file for forward compatibility testing."""
  with open(ForwardMakefileLocation(), 'w') as gen_file:
    WriteForwardMakefile(gen_file)
  print ('Generated forward compatibility Makefile at {}'
         .format(ForwardMakefileLocation()))


def BackwardJavaFileLocation():
  """Returns the location of Java file for backward compatibility testing."""
  return os.path.join(ThisScriptDir(), '..', 'RSTestBackward', 'src', 'com',
                      'android', 'rs', 'testbackward', 'RSTests.java')


def Backward19JavaFileLocation():
  """Returns the location of Java file for backward compatibility 19 testing."""
  return os.path.join(ThisScriptDir(), '..', 'RSTestBackward19', 'src', 'com',
                      'android', 'rs', 'testbackward19', 'RSTests.java')


def Backward19MakefileLocation():
  """Returns the location of Makefile for backward compatibility 19 testing."""
  return os.path.join(ThisScriptDir(), '..', 'RSTestBackward19', 'Android.mk')


def WriteJavaCopyright(gen_file):
  """Writes the copyright for a Java file to gen_file."""
  gen_file.write(
      '/*\n'
      ' * Copyright (C) 2017 The Android Open Source Project\n'
      ' *\n'
      ' * Licensed under the Apache License, Version 2.0 (the "License");\n'
      ' * you may not use this file except in compliance with the License.\n'
      ' * You may obtain a copy of the License at\n'
      ' *\n'
      ' *      http://www.apache.org/licenses/LICENSE-2.0\n'
      ' *\n'
      ' * Unless required by applicable law or agreed to in writing, software\n'
      ' * distributed under the License is distributed on an "AS IS" BASIS,\n'
      ' * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n'
      ' * See the License for the specific language governing permissions and\n'
      ' * limitations under the License.\n'
      ' */\n\n'
  )


def WriteBackwardJavaFile(gen_file, package, max_api_version=None):
  """Writes the Java file for backward compatibility testing to gen_file.

  Java file determines unit tests for backward compatibility
  testing based on the unit test list at the top of this file."""
  WriteJavaCopyright(gen_file)
  gen_file.write(
      'package {};\n'
      '\n'
      'import com.android.rs.unittest.*;\n'
      '\n'
      'import java.util.ArrayList;\n'
      '\n'
      '/**\n'
      ' * This class is auto-generated by frameworks/rs/tests/java_api/RSUnitTests/RSUnitTests.py.\n'
      ' * To change unit tests version, please run the Python script above.\n'
      ' */\n'
      'public class RSTests {{\n'
      '    public static Iterable<Class<? extends UnitTest>> getTestClassesForCurrentAPIVersion() {{\n'
      '        int thisApiVersion = android.os.Build.VERSION.SDK_INT;\n'
      '\n'
      '        ArrayList<Class<? extends UnitTest>> validClasses = new ArrayList<>();'.format(
          package
      )
  )
  for version in sorted(UNIT_TESTS.keys()):
    if max_api_version is None or version <= max_api_version:
      tests = sorted(UNIT_TESTS[version])
      gen_file.write(
          '\n\n        if (thisApiVersion >= {}) {{\n'.format(version)
      )
      for test in tests:
        gen_file.write(
            '            validClasses.add(UT_{}.class);\n'.format(test)
        )
      gen_file.write('        }')
  gen_file.write('\n\n        return validClasses;\n    }\n}\n')


def WriteBackward19Makefile(gen_file):
  """Writes the Makefile for backward compatibility 19 testing."""
  WriteMakeCopyright(gen_file)
  gen_file.write(
      '# This file is auto-generated by frameworks/rs/tests/java_api/RSUnitTests/RSUnitTests.py.\n'
      '# To change unit tests version, please run the Python script above.\n\n'
      'LOCAL_PATH := $(call my-dir)\n'
      'include $(CLEAR_VARS)\n\n'
      'LOCAL_MODULE_TAGS := tests\n'
      'LOCAL_STATIC_JAVA_LIBRARIES := android-support-test\n'
      'LOCAL_COMPATIBILITY_SUITE := device-tests\n'
      'LOCAL_RENDERSCRIPT_TARGET_API := 19\n'
      'LOCAL_MIN_SDK_VERSION := 17\n'
      'LOCAL_PACKAGE_NAME := RSTestBackward19\n'
      'my_rs_unit_tests_path := ../RSUnitTests/src/com/android/rs/unittest\n'
  )
  WriteMakeSrcFiles(gen_file, 19)
  # With dist-for-goals, copy the target to DIST_DIR with 'make dist'
  gen_file.write(
      '\n'
      'include $(BUILD_PACKAGE)\n\n'
      'my_rs_unit_tests_path :=\n\n'
      '$(call dist-for-goals,$(LOCAL_PACKAGE_NAME),$(LOCAL_BUILT_MODULE):$(LOCAL_PACKAGE_NAME).apk)\n'
  )


def GenerateBackward():
  """Generates Java file for backward compatibility testing."""
  with open(BackwardJavaFileLocation(), 'w') as gen_file:
    WriteBackwardJavaFile(gen_file, 'com.android.rs.testbackward')
  print ('Generated backward compatibility Java file at {}'
         .format(BackwardJavaFileLocation()))


def GenerateBackward19():
  """Generates files for backward compatibility testing for API 19."""
  with open(Backward19JavaFileLocation(), 'w') as gen_file:
    WriteBackwardJavaFile(gen_file, 'com.android.rs.testbackward19', 19)
  print ('Generated backward compatibility (19) Java file at {}'
         .format(Backward19JavaFileLocation()))

  with open(Backward19MakefileLocation(), 'w') as gen_file:
    WriteBackward19Makefile(gen_file)
  print ('Generated backward compatibility (19) Makefile at {}'
         .format(Backward19MakefileLocation()))


def DisplayHelp():
  """Prints help message."""
  print >> sys.stderr, ('Usage: {} [forward] [backward] [backward19] [help|-h|--help]\n'
                        .format(sys.argv[0]))
  print >> sys.stderr, ('[forward]: write forward compatibility Makefile to\n    {}\n'
                        .format(ForwardMakefileLocation()))
  print >> sys.stderr, ('[backward]: write backward compatibility Java file to\n    {}\n'
                        .format(BackwardJavaFileLocation()))
  print >> sys.stderr, ('[backward19]: write backward compatibility Java file (19) to\n    {}\n'
                        .format(Backward19JavaFileLocation()))
  print >> sys.stderr, ('[backward19]: write backward compatibility Makefile (19) to\n    {}\n'
                        .format(Backward19MakefileLocation()))
  print >> sys.stderr, 'if no options are chosen, then all files are generated'


def main():
  """Parses sys.argv and does stuff."""
  display_help = False
  error = False
  actions = []

  for arg in sys.argv[1:]:
    if arg in ('help', '-h', '--help'):
      display_help = True
    elif arg == 'backward':
      actions.append(GenerateBackward)
    elif arg == 'backward19':
      actions.append(GenerateBackward19)
    elif arg == 'forward':
      actions.append(GenerateForward)
    else:
      print >> sys.stderr, 'unrecognized arg: {}'.format(arg)
      error = True

  if display_help or error:
    DisplayHelp()
  elif actions:
    for action in actions:
      action()
  else:
    # No args - do default action.
    GenerateBackward()
    GenerateBackward19()
    GenerateForward()


if __name__ == '__main__':
  main()

