'''Module that contains the test TestInvokeFun.'''

from harness.test_base_remote import TestBaseRemote


class TestSingleSource(TestBaseRemote):
    '''Tests debugging a function executed from Java using invoke_*.'''

    bundle_target = {
        'java': "SingleSource"
    }

    def test_single_source(self):
        # pylint: disable=line-too-long
        self.try_command('language renderscript status',
                         ['Runtime Library discovered',
                          'Runtime Driver discovered'])

        self.try_command('breakpoint set --name script_invoke',
                         ['Breakpoint 1', '(pending)'])

        self.try_command('breakpoint set --name square_kernel',
                         ['Breakpoint 2', '(pending)'])

        self.try_command('process continue',
                         ['stopped',
                          'stop reason = breakpoint'],
                         [r'script_invoke'])

        self.try_command('process continue',
                         ['stopped',
                          'stop reason = breakpoint'],
                         [r'square_kernel'])
