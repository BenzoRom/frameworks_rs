'''Module that contains the test TestBreakpointFileLineCpp.'''

from harness.test_base_remote import TestBaseRemote


class TestBreakpointFileLineCpp(TestBaseRemote):
    '''Tests the setting of a breakpoint on a specific line of a RS file.'''

    def get_bundle_target(self):
        '''Return string with name of bundle executable to run.

        Returns:
            A string containing the name of the binary that this test can be run
            with.
        '''
        return 'CppDebugWaitAttach'

    def test_case(self, _):
        '''Run the lldb commands that are being tested.

        Raises:
            TestFail: One of the lldb commands did not provide the expected
                      output.
        '''
        self.try_command('language renderscript status',
                         ['Runtime Library discovered',
                          'Runtime Driver discovered'])

        self.try_command('breakpoint set --file simple.rs --line 12',
                         ['(pending)'])

        self.try_command('process continue',
                         [])

        self.try_command('bt',
                         ['librs.simple',
                          'simple_kernel',
                          'stop reason = breakpoint'])

        self.try_command('breakpoint list',
                         ['simple.rs',
                          'resolved = 1'])

        self.try_command('process status',
                         ['stopped',
                          'stop reason = breakpoint'])

        self.try_command('breakpoint delete 1',
                         ['1 breakpoints deleted'])

        self.try_command('process continue',
                         ['exited with status = 0'])
