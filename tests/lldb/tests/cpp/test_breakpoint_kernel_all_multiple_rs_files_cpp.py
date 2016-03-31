'''Module that contains the test TestBreakpointKernelAllMultipleRSFilesCpp.'''

from harness.test_base_remote import TestBaseRemote


class TestBreakpointKernelAllMultipleRSFilesCpp(TestBaseRemote):
    '''Tests setting bp's on all kernels in >1 kernel files in an NDK app.'''

    def get_bundle_target(self):
        '''Return string with name of bundle executable to run.

        Returns:
            A string containing the name of the binary that this test can be run
            with.
        '''
        return 'CppMultipleRSFiles'

    def test_case(self, _):
        '''Run the lldb commands that are being tested.

        Raises:
            TestFail: One of the lldb commands did not provide the expected
                      output.
        '''
        # Test command works with no kernels currently loaded
        self.try_command('language renderscript kernel breakpoint all enable',
                         ['Breakpoints will be set on all kernels'])

        self.try_command('process continue',
                         ['resuming',
                          'stopped',
                          'stop reason = breakpoint'])

        self.try_command('breakpoint list',
                         ["'first_kernel', locations = 1, resolved = 1",
                          "'second_kernel', locations = 1, resolved = 1"])

        # Check disable doesn't delete breakpoints
        self.try_command('language renderscript kernel breakpoint all disable',
                         ['Breakpoints will not be set on any new kernels'])

        # Delete all breakpoints manually
        self.try_command('breakpoint delete 1',
                         ['1 breakpoints deleted'])

        self.try_command('breakpoint delete 2',
                         ['1 breakpoints deleted'])

        self.try_command('breakpoint list',
                         ["No breakpoints currently set"])

        # Test command works when kernels are loaded
        self.try_command('language renderscript kernel breakpoint all enable',
                         ['Breakpoints will be set on all kernels'])

        self.try_command('breakpoint list',
                         ["'first_kernel', locations = 1, resolved = 1",
                          "'second_kernel', locations = 1, resolved = 1"])

        self.try_command('process continue',
                         ['resuming',
                          'stopped',
                          'stop reason = breakpoint'])

        self.try_command('breakpoint delete 3',
                         ['1 breakpoints deleted'])

        # Check other_kernel breakpoint gets hit
        self.try_command('breakpoint list',
                         ["'second_kernel', locations = 1, resolved = 1"])

        self.try_command('process continue',
                         ['resuming',
                          'stopped',
                          'stop reason = breakpoint'])

        self.try_command('breakpoint delete 4', ['1 breakpoints deleted'])

        self.try_command('process continue',
                         ['exited with status = 0'])
