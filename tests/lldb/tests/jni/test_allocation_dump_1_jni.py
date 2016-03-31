'''Module that contains the test TestAllocationDump1JNI.'''

from harness.test_base_remote import TestBaseRemote
import os


class TestAllocationDump1(TestBaseRemote):
    '''Tests printing the contents of allocations.'''

    def get_bundle_target(self):
        '''Return string with name of bundle executable to run.

        Returns:
            String that is the name of the binary that this test should be run
            with.
        '''
        return 'JNIAllocations'

    def test_case(self, _):
        '''Run the lldb commands that are being tested.

        Raises:
            TestFail: One of the lldb commands did not provide the expected
            output.
        '''
        self.try_command('language renderscript kernel breakpoint all enable',
                         ['Breakpoints will be set on all kernels'])

        self.try_command('process continue',
                         ['resuming',
                          'stopped',
                          'stop reason = breakpoint'])

        # Test dumping large allocations to file
        output_file_1 = self.get_tmp_file_path()
        output_file_2 = self.get_tmp_file_path()

        self.try_command('language renderscript allocation dump 1 -f ' +
                         output_file_1,
                         ["Results written to '%s'" % output_file_1])

        self.try_command('language renderscript allocation dump 2 -f '
                         + output_file_2,
                         ["Results written to '%s'" % output_file_2])

        # Check files were created
        self.test_assert(os.path.isfile(output_file_1))
        self.test_assert(os.path.isfile(output_file_2))
        os.remove(output_file_1)
        os.remove(output_file_2)

        # char
        self.try_command('language renderscript allocation dump 3',
                         ['(0, 0, 0) = 0',
                          '(0, 1, 0) = 1',
                          '(0, 2, 0) = 2',
                          '(0, 0, 1) = 3',
                          '(0, 1, 1) = 4',
                          '(0, 2, 1) = 5',
                          '(0, 0, 2) = 6',
                          '(0, 1, 2) = 7',
                          '(0, 2, 2) = 8',
                          '(0, 0, 3) = 9',
                          '(0, 1, 3) = 10',
                          '(0, 2, 3) = 11',
                          '(0, 0, 4) = 12',
                          '(0, 1, 4) = 13',
                          '(0, 2, 4) = 14',
                          '(0, 0, 5) = 15',
                          '(0, 1, 5) = 16',
                          '(0, 2, 5) = 17',
                          '(0, 0, 6) = 18',
                          '(0, 1, 6) = 19',
                          '(0, 2, 6) = 20',
                          '(0, 0, 7) = 21',
                          '(0, 1, 7) = 22',
                          '(0, 2, 7) = 23'])

        # char2
        self.try_command('language renderscript allocation dump 4',
                         ['(0, 0, 0) = {0 1}',
                          '(1, 0, 0) = {2 3}',
                          '(2, 0, 0) = {4 5}',
                          '(3, 0, 0) = {6 7}',
                          '(4, 0, 0) = {8 9}',
                          '(5, 0, 0) = {10 11}',
                          '(6, 0, 0) = {12 13}',
                          '(7, 0, 0) = {14 15}',
                          '(8, 0, 0) = {16 17}',
                          '(9, 0, 0) = {18 19}',
                          '(10, 0, 0) = {20 21}',
                          '(11, 0, 0) = {22 23}'])

        # char3
        self.try_command('language renderscript allocation dump 5',
                         ['(0, 0, 0) = {0 1 2}',
                          '(1, 0, 0) = {4 5 6}',
                          '(2, 0, 0) = {8 9 10}',
                          '(3, 0, 0) = {12 13 14}',
                          '(4, 0, 0) = {16 17 18}',
                          '(5, 0, 0) = {20 21 22}'])

        # char4
        self.try_command('language renderscript allocation dump 6',
                         ['(0, 0, 0) = {0 1 2 3}',
                          '(1, 0, 0) = {4 5 6 7}',
                          '(2, 0, 0) = {8 9 10 11}',
                          '(3, 0, 0) = {12 13 14 15}',
                          '(4, 0, 0) = {16 17 18 19}',
                          '(5, 0, 0) = {20 21 22 23}'])

        # short
        self.try_command('language renderscript allocation dump 7',
                         ['(0, 0, 0) = 0',
                          '(1, 0, 0) = 1',
                          '(2, 0, 0) = 2',
                          '(3, 0, 0) = 3',
                          '(4, 0, 0) = 4',
                          '(5, 0, 0) = 5',
                          '(6, 0, 0) = 6',
                          '(7, 0, 0) = 7',
                          '(8, 0, 0) = 8',
                          '(9, 0, 0) = 9',
                          '(10, 0, 0) = 10',
                          '(11, 0, 0) = 11',
                          '(12, 0, 0) = 12',
                          '(13, 0, 0) = 13',
                          '(14, 0, 0) = 14',
                          '(15, 0, 0) = 15',
                          '(16, 0, 0) = 16',
                          '(17, 0, 0) = 17',
                          '(18, 0, 0) = 18',
                          '(19, 0, 0) = 19',
                          '(20, 0, 0) = 20',
                          '(21, 0, 0) = 21',
                          '(22, 0, 0) = 22',
                          '(23, 0, 0) = 23'])

        # short2
        self.try_command('language renderscript allocation dump 8',
                         ['(0, 0, 0) = {0 1}',
                          '(1, 0, 0) = {2 3}',
                          '(2, 0, 0) = {4 5}',
                          '(3, 0, 0) = {6 7}',
                          '(4, 0, 0) = {8 9}',
                          '(5, 0, 0) = {10 11}',
                          '(0, 0, 1) = {12 13}',
                          '(1, 0, 1) = {14 15}',
                          '(2, 0, 1) = {16 17}',
                          '(3, 0, 1) = {18 19}',
                          '(4, 0, 1) = {20 21}',
                          '(5, 0, 1) = {22 23}'])

        # short3
        self.try_command('language renderscript allocation dump 9',
                         ['(0, 0, 0) = {0 1 2}',
                          '(1, 0, 0) = {4 5 6}',
                          '(2, 0, 0) = {8 9 10}',
                          '(3, 0, 0) = {12 13 14}',
                          '(4, 0, 0) = {16 17 18}',
                          '(5, 0, 0) = {20 21 22}'])

        # short4
        self.try_command('language renderscript allocation dump 10',
                         ['(0, 0, 0) = {0 1 2 3}',
                          '(1, 0, 0) = {4 5 6 7}',
                          '(2, 0, 0) = {8 9 10 11}',
                          '(3, 0, 0) = {12 13 14 15}',
                          '(4, 0, 0) = {16 17 18 19}',
                          '(5, 0, 0) = {20 21 22 23}'])

        # int
        self.try_command('language renderscript allocation dump 11',
                         ['(0, 0, 0) = 0',
                          '(1, 0, 0) = 1',
                          '(2, 0, 0) = 2',
                          '(3, 0, 0) = 3',
                          '(4, 0, 0) = 4',
                          '(5, 0, 0) = 5',
                          '(6, 0, 0) = 6',
                          '(7, 0, 0) = 7',
                          '(8, 0, 0) = 8',
                          '(9, 0, 0) = 9',
                          '(10, 0, 0) = 10',
                          '(11, 0, 0) = 11',
                          '(12, 0, 0) = 12',
                          '(13, 0, 0) = 13',
                          '(14, 0, 0) = 14',
                          '(15, 0, 0) = 15',
                          '(16, 0, 0) = 16',
                          '(17, 0, 0) = 17',
                          '(18, 0, 0) = 18',
                          '(19, 0, 0) = 19',
                          '(20, 0, 0) = 20',
                          '(21, 0, 0) = 21',
                          '(22, 0, 0) = 22',
                          '(23, 0, 0) = 23'])

        # int2
        self.try_command('language renderscript allocation dump 12',
                         ['(0, 0, 0) = {0 1}',
                          '(1, 0, 0) = {2 3}',
                          '(2, 0, 0) = {4 5}',
                          '(3, 0, 0) = {6 7}',
                          '(4, 0, 0) = {8 9}',
                          '(5, 0, 0) = {10 11}',
                          '(6, 0, 0) = {12 13}',
                          '(7, 0, 0) = {14 15}',
                          '(8, 0, 0) = {16 17}',
                          '(9, 0, 0) = {18 19}',
                          '(10, 0, 0) = {20 21}',
                          '(11, 0, 0) = {22 23}'])

        # int3
        self.try_command('language renderscript allocation dump 13',
                         ['(0, 0, 0) = {0 1 2}',
                          '(1, 0, 0) = {4 5 6}',
                          '(2, 0, 0) = {8 9 10}',
                          '(0, 1, 0) = {12 13 14}',
                          '(1, 1, 0) = {16 17 18}',
                          '(2, 1, 0) = {20 21 22}'])
        # int4
        self.try_command('language renderscript allocation dump 14',
                         ['(0, 0, 0) = {0 1 2 3}',
                          '(1, 0, 0) = {4 5 6 7}',
                          '(2, 0, 0) = {8 9 10 11}',
                          '(3, 0, 0) = {12 13 14 15}',
                          '(4, 0, 0) = {16 17 18 19}',
                          '(5, 0, 0) = {20 21 22 23}'])

        # long
        self.try_command('language renderscript allocation dump 15',
                         ['(0, 0, 0) = 0',
                          '(1, 0, 0) = 1',
                          '(2, 0, 0) = 2',
                          '(3, 0, 0) = 3',
                          '(4, 0, 0) = 4',
                          '(5, 0, 0) = 5',
                          '(6, 0, 0) = 6',
                          '(7, 0, 0) = 7',
                          '(8, 0, 0) = 8',
                          '(9, 0, 0) = 9',
                          '(10, 0, 0) = 10',
                          '(11, 0, 0) = 11',
                          '(12, 0, 0) = 12',
                          '(13, 0, 0) = 13',
                          '(14, 0, 0) = 14',
                          '(15, 0, 0) = 15',
                          '(16, 0, 0) = 16',
                          '(17, 0, 0) = 17',
                          '(18, 0, 0) = 18',
                          '(19, 0, 0) = 19',
                          '(20, 0, 0) = 20',
                          '(21, 0, 0) = 21',
                          '(22, 0, 0) = 22',
                          '(23, 0, 0) = 23'])

        # long2
        self.try_command('language renderscript allocation dump 16',
                         ['(0, 0, 0) = {0 1}',
                          '(1, 0, 0) = {2 3}',
                          '(2, 0, 0) = {4 5}',
                          '(3, 0, 0) = {6 7}',
                          '(4, 0, 0) = {8 9}',
                          '(5, 0, 0) = {10 11}',
                          '(6, 0, 0) = {12 13}',
                          '(7, 0, 0) = {14 15}',
                          '(8, 0, 0) = {16 17}',
                          '(9, 0, 0) = {18 19}',
                          '(10, 0, 0) = {20 21}',
                          '(11, 0, 0) = {22 23}'])

        # long3
        self.try_command('language renderscript allocation dump 17',
                         ['(0, 0, 0) = {0 1 2}',
                          '(1, 0, 0) = {4 5 6}',
                          '(2, 0, 0) = {8 9 10}',
                          '(3, 0, 0) = {12 13 14}',
                          '(4, 0, 0) = {16 17 18}',
                          '(5, 0, 0) = {20 21 22}'])

        # long4
        self.try_command('language renderscript allocation dump 18',
                         ['(0, 0, 0) = {0 1 2 3}',
                          '(0, 1, 0) = {4 5 6 7}',
                          '(0, 2, 0) = {8 9 10 11}',
                          '(0, 3, 0) = {12 13 14 15}',
                          '(0, 4, 0) = {16 17 18 19}',
                          '(0, 5, 0) = {20 21 22 23}'])

        # bool
        self.try_command('language renderscript allocation dump 19',
                         ['(0, 0, 0) = false',
                          '(1, 0, 0) = true',
                          '(2, 0, 0) = false',
                          '(3, 0, 0) = true',
                          '(4, 0, 0) = false',
                          '(5, 0, 0) = true',
                          '(6, 0, 0) = false',
                          '(7, 0, 0) = true',
                          '(8, 0, 0) = false',
                          '(9, 0, 0) = true',
                          '(10, 0, 0) = false',
                          '(11, 0, 0) = true',
                          '(12, 0, 0) = false',
                          '(13, 0, 0) = true',
                          '(14, 0, 0) = false',
                          '(15, 0, 0) = true',
                          '(16, 0, 0) = false',
                          '(17, 0, 0) = true',
                          '(18, 0, 0) = false',
                          '(19, 0, 0) = true',
                          '(20, 0, 0) = false',
                          '(21, 0, 0) = true',
                          '(22, 0, 0) = false',
                          '(23, 0, 0) = true'])
