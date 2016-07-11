'''Module that contains the test TestLanguage.'''

from __future__ import absolute_import

from harness.test_base import TestBaseNoTargetProcess


class TestLanguage(TestBaseNoTargetProcess):
    '''
    Tests the "language" command and "language renderscript" subcommand.
    '''

    def test_lldb_has_language_commands(self):
        ci = self._ci
        self.assert_true(
            ci.HasCommands() and
            ci.CommandExists('language')
        )

        self.try_command('language', ['renderscript'])
        self.try_command('language renderscript', ['kernel',
                                                   'context',
                                                   'module',
                                                   'status'])

