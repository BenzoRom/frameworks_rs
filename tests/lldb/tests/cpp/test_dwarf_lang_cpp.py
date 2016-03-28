'''Module that contains the test DWARF language attribute test for NDK.'''

from harness.test_base_remote import TestBaseRemote


class TestDWARFLangCpp(TestBaseRemote):
    '''Tests the DWARF language attribute is present in RS kernels in a NDK.'''

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
        self.try_command('language renderscript status', [])
        self.try_command('b simple_kernel', [])
        self.try_command('process continue', [])

        self.assert_lang_renderscript()
