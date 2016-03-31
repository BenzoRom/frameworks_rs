'''Module that contains the test TestRSConsts.'''

from harness.test_base_remote import TestBaseRemote


class TestRSConsts(TestBaseRemote):
    '''Tests examining the RenderScript constants.'''

    def get_bundle_target(self):
        '''Return string with name of bundle executable to run.

        Returns:
            A string containing the name of the binary that this test can be run
            with.
        '''
        return "KernelVariables"

    def test_case(self, _):
        '''Run the lldb commands that are being tested.

        Raises:
            TestFail: One of the lldb commands did not provide the expected
                      output.
        '''

        self.try_command('language renderscript status',
                         ['Runtime Library discovered',
                          'Runtime Driver discovered'])

        self.try_command('language renderscript kernel breakpoint set kernel',
                         [])

        self.try_command('process continue',
                         ['resuming',
                          'stopped',
                          'stop reason = breakpoint'])

        # Constants
        self.try_command('expr M_1_PI',
                         ['0.318309'])

        self.try_command('expr M_2_PI',
                         ['0.636619'])

        self.try_command('expr M_2_SQRTPI',
                         ['1.128379'])

        self.try_command('expr M_E',
                         ['2.718281'])

        self.try_command('expr M_LN10',
                         ['2.302585'])

        self.try_command('expr M_LN2',
                         ['0.693147'])

        self.try_command('expr M_LOG10E',
                         ['0.434294'])

        self.try_command('expr M_LOG2E',
                         ['1.442695'])

        self.try_command('expr M_PI',
                         ['3.141592'])

        self.try_command('expr M_PI_2',
                         ['1.570796'])

        self.try_command('expr M_PI_4',
                         ['0.785398'])

        self.try_command('expr M_SQRT1_2',
                         ['0.707106'])

        self.try_command('expr M_SQRT2',
                         ['1.414213'])
