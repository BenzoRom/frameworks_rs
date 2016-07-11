'''Module that contains the test TestInvokeFun.'''

from __future__ import absolute_import

from harness.test_base_remote import TestBaseRemote
from harness.decorators import (
    ordered_test,
    cpp_only_test
)


class TestInvokeFun(TestBaseRemote):
    '''Tests debugging a function executed from Java using invoke_*.'''

    bundle_target = {
        'java': 'BranchingFunCalls',
        'jni': 'JNIBranchingFunCalls',
        'cpp': 'CppBranchingFunCalls'
    }

    def test_invoke_fun(self):
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

    @ordered_test('last')
    @cpp_only_test()
    def test_cpp_cleanup(self):
        self.try_command('breakpoint delete 1', ['1 breakpoints deleted'])

        self.try_command('breakpoint delete 2', ['1 breakpoints deleted'])

        self.try_command('process continue', ['exited with status = 0'])
