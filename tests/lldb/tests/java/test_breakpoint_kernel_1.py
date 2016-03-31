'''Module that contains the test TestBreakpointKernel1.'''

from harness.test_base_remote import TestBaseRemote


class TestBreakpointKernel1(TestBaseRemote):
    '''Tests the setting of a breakpoint on a RS kernel.'''

    def get_bundle_target(self):
        '''Return string with name of bundle executable to run.

        Returns:
            A string containing the name of the binary that this test can be run
            with.
        '''
        return 'JavaDebugWaitAttach'

    def test_case(self, _):
        '''Run the lldb commands that are being tested.

        Raises:
            TestFail: One of the lldb commands did not provide the expected
                      output.
        '''
        # pylint: disable=line-too-long
        self.try_command('language renderscript status',
                         ['Runtime Library discovered',
                          'Runtime Driver discovered'])

        self.try_command('language renderscript kernel breakpoint set simple_kernel',
                         ['Breakpoint(s) created',
                          '(pending)'])

        # Try set a breakpoint on a kernel which doesn't exist
        self.try_command('language renderscript kernel breakpoint set imaginary_kernel',
                         ['Breakpoint(s) created',
                          '(pending)'])

        self.try_command('breakpoint list',
                         ["'simple_kernel', locations = 0 (pending)",
                          "'imaginary_kernel', locations = 0 (pending)"])

        self.try_command('process continue',
                         ['resuming',
                          'stopped',
                          'stop reason = breakpoint'])

        self.try_command('bt',
                         ['stop reason = breakpoint',
                          'frame #0:',
                          'librs.simple.so',
                          'simple_kernel'])

        self.try_command('breakpoint list',
                         ["'imaginary_kernel', locations = 0 (pending)",
                          "'simple_kernel', locations = 1, resolved = 1"])

        # Delete breakpoint on kernel which doesn't exist
        self.try_command('breakpoint delete 2',
                         ['1 breakpoints deleted'])

        self.try_command('breakpoint list',
                         ["'simple_kernel', locations = 1, resolved = 1"])

        self.try_command('process continue',
                         ['resuming',
                          'stopped',
                          'stop reason = breakpoint'])

        self.try_command('breakpoint list',
                         ["'simple_kernel', locations = 1, resolved = 1"])

        self.try_command('breakpoint delete 1',
                         ['1 breakpoints deleted'])

        self.try_command('breakpoint list',
                         ['No breakpoints currently set'])
