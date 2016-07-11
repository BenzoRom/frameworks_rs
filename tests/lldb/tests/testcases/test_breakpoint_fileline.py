'''Module that contains the test TestBreakpointFileLine.'''

from __future__ import absolute_import

from harness.test_base_remote import TestBaseRemote
from harness.decorators import (
    cpp_only_test,
    ordered_test
)


class TestBreakpointFileLine(TestBaseRemote):
    '''Tests the setting of a breakpoint on a specific line of a RS file.'''

    bundle_target = {
        'java': 'JavaDebugWaitAttach',
        'jni': 'JNIDebugWaitAttach',
        'cpp': 'CppDebugWaitAttach'
    }

    @ordered_test(0)
    def test_breakpoint_fileline(self):
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

    @ordered_test('last')
    @cpp_only_test()
    def test_cpp_cleanup(self):
        self.try_command('process continue', ['exited with status = 0'])
