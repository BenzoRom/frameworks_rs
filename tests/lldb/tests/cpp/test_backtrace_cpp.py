'''Module that contains the test TestBacktraceCpp.'''

from harness.test_base_remote import TestBaseRemote


class TestBacktraceCpp(TestBaseRemote):
    '''Tests viewing the call stack of an NDK app.'''

    def get_bundle_target(self):
        '''Return string with name of bundle executable to run.

        Returns:
            A string containing the name of the binary that this test can be run
            with.
        '''
        return 'CppBranchingFunCalls'

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

        self.try_command('process continue',
                         ['resuming',
                          'stopped',
                          'stop reason = breakpoint'])

        self.try_command('bt',
                         ['stop reason = breakpoint',
                          # We should be able to see three functions in bt:
                          # libRSCpuRef, kernel.expand and the kernel
                          'frame #2:',
                          'librs.simple',
                          'simple_kernel'],
                         [r'simple\.rs:4[567]'])

        self.try_command('breakpoint delete 1',
                         ['1 breakpoints deleted'])

        self.try_command('b set_i',
                         ['Breakpoint 2',
                          'set_i'],
                         [r'simple.rs:2[012]'])

        self.try_command('breakpoint list',
                         ['set_i', 'resolved'])

        self.try_command('process continue',
                         ['resuming',
                          'stopped',
                          'stop reason = breakpoint'])

        self.try_command('bt',
                         ['stop reason = breakpoint',
                          # We should be able to see five functions in bt:
                          # libRSCpuRef, kernel.expand, kernel and two functions
                          'frame #4:',
                          'librs.simple',
                          'modify_i',
                          'set_i'],
                         [r'simple\.rs:2[012]'])

        self.try_command('breakpoint delete 2',
                         ['1 breakpoints deleted'])

        self.try_command('process continue',
                         ['exited with status = 0'])
