'''Module that contains the test TestInvokeFun.'''

from harness.test_base_remote import TestBaseRemote


class TestInvokeFun(TestBaseRemote):
    '''Tests debugging a function executed from Java using invoke_*.'''

    def get_bundle_target(self):
        '''Return string with name of bundle executable to run.

        Returns:
            String that is the name of the binary that this test should be run
            with.
        '''
        return "BranchingFunCalls"

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

        self.try_command('breakpoint set --name addToGlobal',
                         ['Breakpoint 1', '(pending)'])

        self.try_command('process continue',
                         ['stopped',
                          'stop reason = breakpoint'],
                         [r'scalars\.rs:5[789]'])

        self.try_command('language renderscript kernel breakpoint set simple_kernel',
                         ['Breakpoint 2', 'Breakpoint(s) created'])

        self.try_command('process continue',
                         ['stopped',
                          'stop reason = breakpoint',
                          'simple_kernel'],
                         [r'scalars\.rs:4[567]'])

        self.try_command('expr glob',
                         ['(int)',
                          '357'])
