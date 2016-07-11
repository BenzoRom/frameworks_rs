'''Module that contains the test TestBreakpointFileLineMultipleRSFiles.'''

from __future__ import absolute_import

from harness.test_base_remote import TestBaseRemote
from harness.decorators import (
    cpp_only_test,
    ordered_test
)


class TestBreakpointFileLineMultipleRSFiles(TestBaseRemote):
    '''Tests the setting of a breakpoint on one of multiple RS files.'''

    bundle_target = {
        'java': 'MultipleRSFiles',
        'jni': 'JNIMultipleRSFiles',
        'cpp': 'CppMultipleRSFiles'
    }

    def _binary_name(self):
        return {
            'java': 'multiplersfiles',
            'jni': 'multiplersfiles',
            'cpp': 'CppMultipleRSFi'
        }[self.app_type]

    @ordered_test(0)
    def test_breakpoint_fileline_multiple_files(self):
        self.try_command('language renderscript status',
                         ['Runtime Library discovered',
                          'Runtime Driver discovered'])

        self.try_command('breakpoint set --file first.rs --line 12',
                         ['(pending)'])

        self.try_command('process continue',
                         ['stopped',
                          'librs.first.so`first_kernel',
                          'at first.rs:12',
                          "name = '%s'" % self._binary_name(),
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
                          "name = '%s'" % self._binary_name(),
                          'stop reason = breakpoint 2'])

        self.try_command('process status',
                         ['stopped',
                          'stop reason = breakpoint'])

    @cpp_only_test()
    @ordered_test('last')
    def test_cpp_cleanup(self):
        self.try_command('breakpoint delete 2', ['1 breakpoints deleted'])

        self.try_command('process continue', ['exited with status = 0'])
