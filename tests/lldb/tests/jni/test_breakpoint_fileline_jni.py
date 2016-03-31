'''Module that contains the test TestBreakpointFileLineJNI.'''

from harness.test_base_remote import TestBaseRemote

class TestBreakpointFileLineJNI(TestBaseRemote):
    '''Tests setting a bp on a specific line of a RS file in a JNI apk.'''

    def get_bundle_target(self):
        '''Return string with name of bundle executable to run.

        Returns:
            A string containing the name of the binary that this test can be run
            with.
        '''
        return 'JNIDebugWaitAttach'

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
                         ['librs.simple.so',
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
