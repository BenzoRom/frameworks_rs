'''Module that contains the test DWARF language attribute test.'''

from __future__ import absolute_import

from harness.test_base_remote import TestBaseRemote


class TestDWARFLang(TestBaseRemote):
    '''Tests the DWARF language attribute is present in RenderScript kernels.'''

    bundle_target = {
        'java': 'JavaDebugWaitAttach',
        'jni': 'JNIDebugWaitAttach',
        'cpp': 'CppDebugWaitAttach'
    }

    def test_renderscript_kernel_frame_dwarf_language(self):
        self.try_command('language renderscript status', [])
        self.try_command('b simple_kernel', [])
        self.try_command('process continue', [])

        self.assert_lang_renderscript()
