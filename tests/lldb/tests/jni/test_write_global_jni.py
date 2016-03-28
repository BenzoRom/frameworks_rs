'''Module that contains the test TestWriteGlobalJNI.'''

from harness.test_base_remote import TestBaseRemote

class TestWriteGlobalJNI(TestBaseRemote):
    '''Tests modifying global variables of all types in a JNI apk.'''

    def get_bundle_target(self):
        '''Return string with name of bundle executable to run.

        Returns:
            A string containing the name of the binary that this test can be run
            with.
        '''
        return "JNIKernelVariables"

    def _try_modifying_global(self, global_name, new_value, data_type_in,
                             expected_output, expected_output_regex=None):
        '''Modify and then inspect a global and check for the output.

        Run the "expr" command to set a given global to a new value and
        check that it is set afterwards by running the "target variable"
        command.

        Args:
            global_name: String which is the name of the global to modify.
            new_value: A string that is the new value of the global.
            data_type_in: A string containing a c-style parenthesised data type
                          representing the type of the global.
            expected_output: List of strings that should be found in the output
                             of both commands.
            expected_output_regex: List of regular expressions that should be
                                   found in the output of the target variable
                                   command.

        Raises:
            TestFail: One of the lldb commands did not provide the expected
                      output.
        '''

        # pylint: disable=too-many-arguments

        self.try_command('expr %s = %s%s' %
                         (global_name, data_type_in, new_value),
                         expected_output,
                         expected_output_regex)
        self.try_command('target variable ' + global_name,
                         expected_output,
                         expected_output_regex)

    def test_case(self, _):
        '''Run the lldb commands that are being tested.

        Raises:
            TestFail: One of the lldb commands did not provide the expected
                      output.
        '''

        # pylint: disable=line-too-long
        # There is no way we can make this look nice

        self.try_command('language renderscript status',
                         ['Runtime Library discovered',
                          'Runtime Driver discovered'])

        self.try_command('b -f simple.rs -l 129', [])

        self.try_command('process continue',
                         ['resuming',
                          'stopped',
                          'stop reason = breakpoint'])

        self._try_modifying_global('char_global', '-2',
                                  '(signed char)', ['\'\\xfe\''],
                                  [r'\((signed )?char\)'])

        self._try_modifying_global('uchar_global', '22',
                                  '(uchar)', ['(uchar)', '\'\\x16\''])

        self._try_modifying_global('short_global', '-33',
                                  '(short)', ['(short)', '-33'])

        self._try_modifying_global('ushort_global', '44',
                                  '(ushort)', ['(ushort)', '44'])

        self._try_modifying_global('int_global', '-55',
                                  '(int)', ['(int)', '-55'])

        self._try_modifying_global('uint_global', '66',
                                  '(uint)', ['(uint)', '66'])

        self._try_modifying_global('float_global', '-7.5',
                                  '(float)', ['(float)', '-7.5'])

        self._try_modifying_global('long_global', '-888888',
                                  '(long long)', ['-888888'],
                                  [r'\((long )?long\)'])

        self._try_modifying_global('ulong_global', '99999999',
                                  '(ulong)', ['(ulong)', '99999999'])

        self._try_modifying_global('double_global', '-10101.5',
                                  '(double)', ['(double)', '-10101.5'])


        self._try_modifying_global('char2_global', '{22, 4}',
                                  '(char2)', ['(char2)', '(22, 4)'])

        self._try_modifying_global('uchar2_global', '{44, 55}',
                                  '(uchar2)', ['(uchar2)', '(0x2c, 0x37)'])

        self._try_modifying_global('short2_global', '{-66, 77}',
                                  '(short2)', ['(short2)', '(-66, 77)'])

        self._try_modifying_global('ushort2_global', '{88, 99}',
                                  '(ushort2)', ['(ushort2)', '(88, 99)'])

        self._try_modifying_global('int2_global', '{111, -222}',
                                  '(int2)', ['(int2)', '(111, -222)'])

        self._try_modifying_global('uint2_global', '{333, 444}',
                                  '(uint2)', ['(uint2)', '(333, 444)'])

        self._try_modifying_global('float2_global', '{-55.5f, 6.0}',
                                  '(float2)', ['(float2)', '(-55.5, 6)'])

        self._try_modifying_global('long2_global', '{666666, -777777}',
                                  '(long2)', ['(long2)', '(666666, -777777)'])

        self._try_modifying_global('ulong2_global', '{888888, 999999}',
                                  '(ulong2)', ['(ulong2)', '(888888, 999999)'])

        self._try_modifying_global('double2_global', '{11.0000000, -0.0l}',
                                  '(double2)', ['(double2)', '(11, -0)'])

        self._try_modifying_global('char3_global', '{2, -3, 4}',
                                  '(char3)', ['(char3)', '(2, -3, 4,'])

        self._try_modifying_global('uchar3_global', '{\'a\', \'b\', \'c\'}',
                                  '(uchar3)', ['(uchar3)', '(0x61, 0x62, 0x63,'])

        self._try_modifying_global('short3_global', '{44, -55, 66}',
                                  '(short3)', ['(short3)', '(44, -55, 66,'])

        self._try_modifying_global('ushort3_global', '{88, 99, 111}',
                                  '(ushort3)', ['(ushort3)', '(88, 99, 111,'])

        self._try_modifying_global('int3_global', '{-111, 222, -333}',
                                  '(int3)', ['(int3)', '(-111, 222, -333,'])

        self._try_modifying_global('uint3_global', '{444, 555, 666}',
                                  '(uint3)', ['(uint3)', '(444, 555, 666,'])

        self._try_modifying_global('float3_global', '{7.5F, 0008.000, 9}',
                                  '(float3)', ['(float3)', '(7.5, 8, 9,'])

        self._try_modifying_global('long3_global', '{111111, -22222222, 3333333}',
                                  '(long3)', ['(long3)', '(111111, -22222222, 3333333,'])

        self._try_modifying_global('ulong3_global', '{4444444, 5555555, 66666666}',
                                  '(ulong3)', ['(ulong3)', '(4444444, 5555555, 66666666,'])

        self._try_modifying_global('double3_global', '{7.5L, -0, 8.9e1}',
                                  '(double3)', ['(double3)', '(7.5, 0, 89,'])

        self._try_modifying_global('char4_global', '{0x1, 0x2, 0x3, 0x4}',
                                  '(char4)',
                                  ['(char4)', '(1, 2, 3, 4)'])

        self._try_modifying_global('uchar4_global', '{0x5, 0x6, 0x7, 0x8}',
                                  '(uchar4)',
                                  ['(uchar4)', '(0x05, 0x06, 0x07, 0x08)'])

        self._try_modifying_global('short4_global', '{0x9, 0xa, 0xb, 0xc}',
                                  '(short4)',
                                  ['(short4)', '(9, 10, 11, 12)'])

        self._try_modifying_global('ushort4_global', '{0xd, 0xe, 0xf, 0x10}',
                                  '(ushort4)',
                                  ['(ushort4)', '(13, 14, 15, 16)'])

        self._try_modifying_global('int4_global', '{0x11, 0x12, 0x13, 0x14}',
                                  '(int4)',
                                  ['(int4)', '(17, 18, 19, 20)'])

        self._try_modifying_global('uint4_global', '{0x15, 0x16, 0x17, 0x18}',
                                  '(uint4)',
                                  ['(uint4)', '(21, 22, 23, 24)'])

        self._try_modifying_global('float4_global', '{19.0, 20.5, -21, -22.5}',
                                  '(float4)',
                                  ['(float4)', '(19, 20.5, -21, -22.5)'])

        self._try_modifying_global('long4_global', '{0x1d, 0x1e, 0x1f, 0x20}',
                                  '(long4)',
                                  ['(long4)', '(29, 30, 31, 32)'])

        self._try_modifying_global('ulong4_global', '{0x21, 0x22, 0x23, 0x24}',
                                  '(ulong4)',
                                  ['(ulong4)', '(33, 34, 35, 36)'])

        self._try_modifying_global('double4_global', '{25.000, -26, -27.5, 28.0}',
                                  '(double4)',
                                  ['(double4)', '(25, -26, -27.5, 28)'])
