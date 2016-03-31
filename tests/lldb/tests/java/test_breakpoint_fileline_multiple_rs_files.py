'''Module that contains the test TestBreakpointFileLineMultipleRSFiles.'''

from harness.test_base_remote import TestBaseRemote


class TestBreakpointFileLineMultipleRSFiles(TestBaseRemote):
    '''Tests the setting of a breakpoint on one of multiple RS files.'''

    def get_bundle_target(self):
        '''Return string with name of bundle executable to run.

        Returns:
            A string containing the name of the binary that this test can be run
            with.
        '''
        return 'MultipleRSFiles'

    def test_case(self, _):
        '''Run the lldb commands that are being tested.

        Raises:
            TestFail: One of the lldb commands did not provide the expected
                      output.
        '''
        self.try_command('language renderscript status',
                         ['Runtime Library discovered',
                          'Runtime Driver discovered'])

        self.try_command('breakpoint set --file first.rs --line 12',
                         ['(pending)'])

        self.try_command('process continue',
                         ['stopped',
                          'librs.first.so`first_kernel',
                          'at first.rs:12',
                          "name = 'multiplersfiles'",
                          'stop reason = breakpoint 1'])

        self.try_command('breakpoint set --file second.rs --line 7',
                         ['Breakpoint 2',
                          'librs.second.so`second_kernel',
                          'second.rs:7'])

        self.try_command('breakpoint list',
                         ['first.rs',
                          'second.rs',
                          'resolved = 1',
                          'first.rs:12',
                          'second.rs:7'])

        self.try_command('breakpoint delete 1',
                         ['1 breakpoints deleted'])

        self.try_command('process continue',
                         ['stopped',
                          'librs.second.so`second_kernel',
                          'at second.rs:7',
                          "name = 'multiplersfiles'",
                          'stop reason = breakpoint 2'])

        self.try_command('process status',
                         ['stopped',
                          'stop reason = breakpoint'])
