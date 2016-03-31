'''Module that contains the test TestLanguage.'''

from harness.test_base import TestBase


class TestLanguage(TestBase):
    '''Tests the "language" command and "language renderscript" subcommand.

    This doesn't require a binary to be running.
    '''

    def get_bundle_target(self):
        '''Return string with name of bundle executable to run.

        Returns:
            A string containing the name of the binary that this test can be run
            with.
        '''
        return None

    def run(self, dbg, _, lldb, _unused):
        '''Execute the test case.

        Args:
            dbg: The instance of the SBDebugger that is used to test commands.
            lldb: A handle to the lldb module.

        Returns:
            True: test passed, False: test failed.
        '''
        assert dbg
        assert lldb

        self._lldb = lldb

        try:
            self._ci = dbg.GetCommandInterpreter()
            assert self._ci

            if (not self._ci.IsValid() or
                not self._ci.HasCommands() or
                not self._ci.CommandExists('language')):
                return False

            self.try_command('language', ['renderscript'])

            self.try_command('language renderscript', ['kernel',
                                                       'context',
                                                       'module',
                                                       'status'])

        except self.TestFail:
            return False

        return True
