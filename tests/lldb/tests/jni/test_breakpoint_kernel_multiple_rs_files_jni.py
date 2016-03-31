'''Module that contains the test TestBreakpointKernelMultipleRSFilesJNI.'''

from harness.test_base_remote import TestBaseRemote

class TestBreakpointKernelMultipleRSFilesJNI(TestBaseRemote):
    '''Tests setting a breakpoint on RS kernels in many files in a JNI apk.'''

    def get_bundle_target(self):
        '''Return string with name of bundle executable to run.

        Returns:
            A string containing the name of the binary that this test can be run
            with.
        '''
        return 'JNIMultipleRSFiles'

    def test_case(self, _):
        '''Run the lldb commands that are being tested.

        Raises:
            TestFail: One of the lldb commands did not provide the expected
                      output.
        '''
        # pylint: disable=line-too-long
        self.try_command('language renderscript kernel breakpoint set first_kernel',
                         ['Breakpoint(s) created',
                          '(pending)'])

        self.try_command('breakpoint list',
                         ["'first_kernel', locations = 0 (pending)"])

        self.try_command('process continue',
                         ['stopped',
                          'librs.first.so`first_kernel',
                          "name = 'multiplersfiles'",
                          'stop reason = breakpoint 1'],
                          [r'at first\.rs:1[012]'])

        self.try_command('breakpoint list',
                         ["'first_kernel', locations = 1, resolved = 1"])

        self.try_command('language renderscript kernel breakpoint set second_kernel',
                         ['Breakpoint(s) created',
                          'Breakpoint 2',
                          'Breakpoint(s) created'],
                          [r"librs\.second\.so`second_kernel at second\.rs:[56]",])

        self.try_command('breakpoint list',
                         ["'first_kernel', locations = 1, resolved = 1",
                          "'second_kernel', locations = 1, resolved = 1"])

        self.try_command('breakpoint delete 1',
                         ['1 breakpoints deleted'])

        self.try_command('breakpoint list',
                         ["'second_kernel', locations = 1, resolved = 1"])

        self.try_command('process continue',
                         ['resuming',
                          'stopped',
                          'stop reason = breakpoint',
                          "librs.second.so`second_kernel"],
                          [r'second\.rs:[56]'])

        self.try_command('breakpoint delete 2',
                         ['1 breakpoints deleted'])

        self.try_command('breakpoint list',
                         ['No breakpoints currently set'])
