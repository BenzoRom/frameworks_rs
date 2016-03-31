'''Module that contains the test TestMultipleRSFilesJNI.'''

from harness.test_base_remote import TestBaseRemote

class TestMultipleRSFilesJNI(TestBaseRemote):
    '''Tests some commands on a JNI apk which has two rs files.'''

    def get_bundle_target(self):
        '''Return string with name of bundle executable to run.

        Returns:
            A string containing the name of the binary that this test can be run
            with.
        '''
        return 'JNIMultipleRSFiles'

    def test_case(self, _):
        '''Run the lldb commands that are being tested.'''

        # pylint: disable=line-too-long

        self.try_command('language renderscript status',
                         ['Runtime Library discovered',
                          'Runtime Driver discovered',
                          'Runtime functions hooked'])

        self.try_command('breakpoint set --file first.rs --line 12',
                         ['(pending)'])

        self.try_command('process continue',
                         ['stopped',
                          'librs.first.so`first_kernel',
                          'at first.rs:12',
                          "name = 'multiplersfiles'",
                          'stop reason = breakpoint 1'])

        self.try_command('language renderscript kernel list',
                         ['RenderScript Kernels',
                          "Resource 'first'",
                          "Resource 'second'",
                          'root',
                          'first_kernel',
                          'second_kernel'])

        self.try_command('language renderscript context dump',
                         ['Inferred RenderScript Contexts',
                          '2 script instances'])

        self.try_command('language renderscript module dump',
                         ['RenderScript Modules:',
                          'librs.first.so',
                          'librs.second.so',
                          'Debug info loaded',
                          'Globals: 1',
                          'gColor - float4',
                          'Kernels: 2',
                          'root',
                          'first_kernel',
                          'second_kernel',
                          'java_package_name: com.android.rs.jnimultiplersfiles',
                          'version:'])
