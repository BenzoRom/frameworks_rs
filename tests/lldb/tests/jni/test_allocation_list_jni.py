'''Module that contains the test TestAllocationListJNI.'''

from harness.test_base_remote import TestBaseRemote

#
class TestAllocationListJNI(TestBaseRemote):
    '''Tests printing the details of all allocations of a JNI apk.'''

    def get_bundle_target(self):
        '''Return string with name of bundle executable to run.

        Returns:
            A string containing the name of the binary that this test can be run
            with.
        '''
        return 'JNIAllocations'

    def test_case(self, _):
        '''Run the lldb commands that are being tested.

        Raises:
            TestFail: One of the lldb commands did not provide the expected
                      output.
        '''
        # pylint: disable=anomalous-backslash-in-string
        self.try_command('language renderscript kernel breakpoint all enable',
                         ['Breakpoints will be set on all kernels'])

        self.try_command('process continue',
                         ['resuming',
                          'stopped',
                          'stop reason = breakpoint'])

        # Test command line flag for single allocation
        self.try_command('language renderscript allocation list -i 3',
                         [],
                         ['3:\n'
                          '    Context: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Address: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Data pointer: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Dimensions: \(1, 3, 8\)\n'
                          '    Data Type: char\n'
                          '    Data Kind: User'])

        self.try_command('language renderscript allocation list',
                         [],
                         ['1:\n'
                          # Regex for non zero hex number
                          '    Context: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Address: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Data pointer: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Dimensions: \(64, 64, 0\)\n'
                          '    Data Type: uchar4\n'
                          '    Data Kind: RGBA Pixel',
                          '2:\n'
                          '    Context: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Address: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Data pointer: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Dimensions: \(64, 64, 0\)\n'
                          '    Data Type: uchar4\n'
                          '    Data Kind: RGBA Pixel',
                          '3:\n'
                          '    Context: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Address: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Data pointer: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Dimensions: \(1, 3, 8\)\n'
                          '    Data Type: char\n'
                          '    Data Kind: User',
                          '4:\n'
                          '    Context: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Address: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Data pointer: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Dimensions: \(12, 0, 0\)\n'
                          '    Data Type: char2\n'
                          '    Data Kind: User',
                          '5:\n'
                          '    Context: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Address: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Data pointer: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Dimensions: \(6, 0, 0\)\n'
                          '    Data Type: char3\n'
                          '    Data Kind: User',
                          '6:\n'
                          '    Context: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Address: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Data pointer: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Dimensions: \(6, 0, 0\)\n'
                          '    Data Type: char4\n'
                          '    Data Kind: User',
                          '7:\n'
                          '    Context: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Address: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Data pointer: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Dimensions: \(24, 0, 0\)\n'
                          '    Data Type: short\n'
                          '    Data Kind: User',
                          '8:\n'
                          '    Context: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Address: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Data pointer: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Dimensions: \(6, 1, 2\)\n'
                          '    Data Type: short2\n'
                          '    Data Kind: User',
                          '9:\n'
                          '    Context: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Address: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Data pointer: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Dimensions: \(6, 0, 0\)\n'
                          '    Data Type: short3\n'
                          '    Data Kind: User',
                          '10:\n'
                          '    Context: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Address: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Data pointer: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Dimensions: \(6, 0, 0\)\n'
                          '    Data Type: short4\n'
                          '    Data Kind: User',
                          '11:\n'
                          '    Context: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Address: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Data pointer: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Dimensions: \(24, 0, 0\)\n'
                          '    Data Type: int\n'
                          '    Data Kind: User',
                          '12:\n'
                          '    Context: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Address: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Data pointer: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Dimensions: \(12, 0, 0\)\n'
                          '    Data Type: int2\n'
                          '    Data Kind: User',
                          '13:\n'
                          '    Context: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Address: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Data pointer: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Dimensions: \(3, 2, 0\)\n'
                          '    Data Type: int3\n'
                          '    Data Kind: User',
                          '14:\n'
                          '    Context: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Address: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Data pointer: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Dimensions: \(6, 0, 0\)\n'
                          '    Data Type: int4\n'
                          '    Data Kind: User',
                          '15:\n'
                          '    Context: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Address: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Data pointer: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Dimensions: \(24, 0, 0\)\n'
                          '    Data Type: long\n'
                          '    Data Kind: User',
                          '16:\n'
                          '    Context: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Address: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Data pointer: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Dimensions: \(12, 0, 0\)\n'
                          '    Data Type: long2\n'
                          '    Data Kind: User',
                          '17:\n'
                          '    Context: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Address: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Data pointer: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Dimensions: \(6, 0, 0\)\n'
                          '    Data Type: long3\n'
                          '    Data Kind: User',
                          '18:\n'
                          '    Context: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Address: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Data pointer: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Dimensions: \(1, 6, 0\)\n'
                          '    Data Type: long4\n'
                          '    Data Kind: User',
                          '19:\n'
                          '    Context: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Address: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Data pointer: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Dimensions: \(24, 0, 0\)\n'
                          '    Data Type: bool\n'
                          '    Data Kind: User'
                         ])


        self.try_command('breakpoint del 1',
                         ['1 breakpoints deleted'])

        # Hit second kernel
        self.try_command('process continue',
                         ['resuming',
                          'stopped',
                          'stop reason = breakpoint'])

        self.try_command('language renderscript allocation list',
                         [],
                         ['2:\n'
                          '    Context: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Address: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Data pointer: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Dimensions: \(64, 64, 0\)\n'
                          '    Data Type: uchar4\n'
                          '    Data Kind: RGBA Pixel',
                          '7:\n'
                          '    Context: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Address: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Data pointer: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Dimensions: \(24, 0, 0\)\n'
                          '    Data Type: short\n'
                          '    Data Kind: User',
                          '20:\n'
                          '    Context: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Address: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Data pointer: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Dimensions: \(24, 0, 0\)\n'
                          '    Data Type: uchar\n'
                          '    Data Kind: User',
                          '21:\n'
                          '    Context: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Address: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Data pointer: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Dimensions: \(2, 6, 0\)\n'
                          '    Data Type: uchar2\n'
                          '    Data Kind: User',
                          '22:\n'
                          '    Context: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Address: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Data pointer: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Dimensions: \(6, 0, 0\)\n'
                          '    Data Type: uchar3\n'
                          '    Data Kind: User',
                          '23:\n'
                          '    Context: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Address: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Data pointer: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Dimensions: \(6, 0, 0\)\n'
                          '    Data Type: uchar4\n'
                          '    Data Kind: User',
                          '24:\n'
                          '    Context: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Address: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Data pointer: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Dimensions: \(24, 0, 0\)\n'
                          '    Data Type: ushort\n'
                          '    Data Kind: User',
                          '25:\n'
                          '    Context: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Address: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Data pointer: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Dimensions: \(12, 0, 0\)\n'
                          '    Data Type: ushort2\n'
                          '    Data Kind: User',
                          '26:\n'
                          '    Context: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Address: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Data pointer: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Dimensions: \(1, 6, 0\)\n'
                          '    Data Type: ushort3\n'
                          '    Data Kind: User',
                          '27:\n'
                          '    Context: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Address: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Data pointer: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Dimensions: \(6, 0, 0\)\n'
                          '    Data Type: ushort4\n'
                          '    Data Kind: User',
                          '28:\n'
                          '    Context: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Address: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Data pointer: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Dimensions: \(24, 0, 0\)\n'
                          '    Data Type: uint\n'
                          '    Data Kind: User',
                          '29:\n'
                          '    Context: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Address: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Data pointer: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Dimensions: \(12, 0, 0\)\n'
                          '    Data Type: uint2\n'
                          '    Data Kind: User',
                          '30:\n'
                          '    Context: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Address: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Data pointer: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Dimensions: \(6, 0, 0\)\n'
                          '    Data Type: uint3\n'
                          '    Data Kind: User',
                          '31:\n'
                          '    Context: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Address: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Data pointer: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Dimensions: \(1, 1, 6\)\n'
                          '    Data Type: uint4\n'
                          '    Data Kind: User',
                          '32:\n'
                          '    Context: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Address: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Data pointer: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Dimensions: \(4, 3, 2\)\n'
                          '    Data Type: ulong\n'
                          '    Data Kind: User',
                          '33:\n'
                          '    Context: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Address: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Data pointer: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Dimensions: \(12, 0, 0\)\n'
                          '    Data Type: ulong2\n'
                          '    Data Kind: User',
                          '34:\n'
                          '    Context: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Address: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Data pointer: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Dimensions: \(6, 0, 0\)\n'
                          '    Data Type: ulong3\n'
                          '    Data Kind: User',
                          '35:\n'
                          '    Context: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Address: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Data pointer: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Dimensions: \(6, 0, 0\)\n'
                          '    Data Type: ulong4\n'
                          '    Data Kind: User'
                         ])

        self.try_command('breakpoint del 2',
                         ['1 breakpoints deleted'])

        # Hit third kernel
        self.try_command('process continue',
                         ['resuming',
                          'stopped',
                          'stop reason = breakpoint'])

        self.try_command('language renderscript allocation list',
                         [],
                         ['2:\n'
                          '    Context: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Address: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Data pointer: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Dimensions: \(64, 64, 0\)\n'
                          '    Data Type: uchar4\n'
                          '    Data Kind: RGBA Pixel',
                          '7:\n'
                          '    Context: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Address: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Data pointer: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Dimensions: \(24, 0, 0\)\n'
                          '    Data Type: short\n'
                          '    Data Kind: User',
                          '28:\n'
                          '    Context: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Address: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Data pointer: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Dimensions: \(24, 0, 0\)\n'
                          '    Data Type: uint\n'
                          '    Data Kind: User',
                          '36:\n'
                          '    Context: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Address: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Data pointer: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Dimensions: \(24, 0, 0\)\n'
                          '    Data Type: half\n'
                          '    Data Kind: User',
                          '37:\n'
                          '    Context: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Address: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Data pointer: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Dimensions: \(12, 0, 0\)\n'
                          '    Data Type: half2\n'
                          '    Data Kind: User',
                          '38:\n'
                          '    Context: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Address: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Data pointer: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Dimensions: \(1, 6, 0\)\n'
                          '    Data Type: half3\n'
                          '    Data Kind: User',
                          '39:\n'
                          '    Context: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Address: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Data pointer: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Dimensions: \(6, 0, 0\)\n'
                          '    Data Type: half4\n'
                          '    Data Kind: User',
                          '40:\n'
                          '    Context: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Address: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Data pointer: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Dimensions: \(24, 0, 0\)\n'
                          '    Data Type: float\n'
                          '    Data Kind: User',
                          '41:\n'
                          '    Context: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Address: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Data pointer: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Dimensions: \(12, 0, 0\)\n'
                          '    Data Type: float2\n'
                          '    Data Kind: User',
                          '42:\n'
                          '    Context: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Address: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Data pointer: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Dimensions: \(6, 0, 0\)\n'
                          '    Data Type: float3\n'
                          '    Data Kind: User',
                          '43:\n'
                          '    Context: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Address: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Data pointer: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Dimensions: \(3, 2, 0\)\n'
                          '    Data Type: float4\n'
                          '    Data Kind: User',
                          '44:\n'
                          '    Context: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Address: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Data pointer: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Dimensions: \(24, 0, 0\)\n'
                          '    Data Type: double\n'
                          '    Data Kind: User',
                          '45:\n'
                          '    Context: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Address: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Data pointer: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Dimensions: \(4, 1, 3\)\n'
                          '    Data Type: double2\n'
                          '    Data Kind: User',
                          '46:\n'
                          '    Context: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Address: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Data pointer: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Dimensions: \(1, 2, 3\)\n'
                          '    Data Type: double3\n'
                          '    Data Kind: User',
                          '47:\n'
                          '    Context: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Address: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Data pointer: 0x0*[1-9a-fA-F][0-9a-fA-F]*\n'
                          '    Dimensions: \(1, 2, 3\)\n'
                          '    Data Type: double4\n'
                          '    Data Kind: User'
                         ])
