'''Module that contains the test TestBreakpointCoordinateCpp.'''

from harness.test_base_remote import TestBaseRemote


class TestBreakpointCoordinateCpp(TestBaseRemote):
    '''Tests breaking on a specific kernel invocation in a NDK app

    Uses the -c option to specify the coordinate.
    '''

    def get_bundle_target(self):
        '''Return string with name of bundle executable to run.

        Returns:
            A string containing the name of the binary that this test can be run
            with.
        '''
        return 'CppAllocations'

    def test_setup(self, android):
        '''This test requires to be run on one thread.

        Args:
            android: The android_util module.
        '''
        android.push_prop('debug.rs.max-threads', 1)

    def test_shutdown(self, android):
        '''Reset the number of RS threads to the previous value.

        Args:
            android: The android_util module.
        '''
        android.pop_prop('debug.rs.max-threads')

    def _check_coordinates(self, (x_coord, y_coord, z_coord), kernel):
        '''Run lldb commands to check that coordinates match expected values.

        Args:
            (x_coord, y_coord, z_coord): The expected coordinates.
            kernel: String that is the name of the kernel function.

        Raises:
            TestFail: One of the lldb commands did not provide the expected
                      output.
        '''
        self.try_command('bt',
                         ['stop reason = breakpoint',
                          'frame #0:',
                          'librs.allocs',
                          kernel])

        self.try_command('language renderscript kernel coordinate',
                         ['Coordinate: (%d, %d, %d)' % (x_coord, y_coord,
                                                        z_coord)])

    def test_case(self, _):
        '''Run the lldb commands that are being tested.

        Raises:
            TestFail: One of the lldb commands did not provide the expected
                      output.
        '''
        # pylint: disable=line-too-long
        # Test conditional coordinate in a two dimensions
        self.try_command('language renderscript kernel breakpoint set swizzle_kernel -c 3,7',
                         ['Conditional kernel breakpoint on coordinate 3, 7, 0',
                          'Breakpoint(s) created'])

        # We will delete this breakpoint before we hit it.
        self.try_command('language renderscript kernel breakpoint set swizzle_kernel -c 199,199',
                        ['Conditional kernel breakpoint on coordinate 199, 199, 0',
                         'Breakpoint(s) created'])

        self.try_command('process continue',
                         ['resuming',
                          'stopped',
                          'stop reason = breakpoint'])

        self._check_coordinates((3, 7, 0), 'swizzle_kernel')

        # Check breakpoints that have been hit are disabled
        self.try_command('breakpoint list',
                         ["1: RenderScript kernel breakpoint for 'swizzle_kernel', locations = 1 Options: disabled",
                          "2: RenderScript kernel breakpoint for 'swizzle_kernel', locations = 1, resolved = 1"])

        # Delete breakpoint on 199,199,0
        self.try_command('breakpoint delete 2',
                         ['1 breakpoints deleted'])

        # Test conditional coordinate in a single dimension
        self.try_command('language renderscript kernel breakpoint set square_kernel -c 8',
                        ['Conditional kernel breakpoint on coordinate 8, 0, 0',
                         'Breakpoint(s) created'])

        self.try_command('process continue',
                         ['resuming',
                          'stopped',
                          'stop reason = breakpoint'])

        self._check_coordinates((8, 0, 0), 'square_kernel')

        # Test conditional coordinate in three dimensions
        self.try_command('language renderscript kernel breakpoint set add_half_kernel -c 0,0,1',
                        ['Conditional kernel breakpoint on coordinate 0, 0, 1',
                         'Breakpoint(s) created'])

        # Test we can set more than one conditional kernel breakpoint and both
        # will be hit
        self.try_command('language renderscript kernel breakpoint set add_half_kernel -c 0,1,2',
                        ['Conditional kernel breakpoint on coordinate 0, 1, 2',
                         'Breakpoint(s) created'])

        self.try_command('process continue',
                         ['resuming',
                          'stopped',
                          'stop reason = breakpoint'])

        self._check_coordinates((0, 0, 1), 'add_half_kernel')

        self.try_command('process continue',
                         ['resuming',
                          'stopped',
                          'stop reason = breakpoint'])

        self._check_coordinates((0, 1, 2), 'add_half_kernel')

        # Check we can see the coordinate from a function invoked by the kernel
        self.try_command('break set -n half_helper',
                         ['librs.allocs.so`half_helper'])

        self.try_command('process continue',
                         ['resuming',
                          'stopped',
                          'stop reason = breakpoint'])

        self._check_coordinates((0, 1, 2), 'half_helper')

        # Delete helper function breakpoint
        self.try_command('breakpoint delete 6',
                             ['1 breakpoints deleted'])

        # Delete breakpoint which has already been disabled
        self.try_command('breakpoint delete 1',
                         ['1 breakpoints deleted'])

        # Check our delete breakpoints have been removed
        self.try_command('breakpoint list',
                         ["3: RenderScript kernel breakpoint for 'square_kernel', locations = 1 Options: disabled",
                          "4: RenderScript kernel breakpoint for 'add_half_kernel', locations = 1 Options: disabled",
                          "5: RenderScript kernel breakpoint for 'add_half_kernel', locations = 1 Options: disabled"])

        self.try_command('breakpoint delete 3', ['1 breakpoints deleted'])

        self.try_command('breakpoint delete 4', ['1 breakpoints deleted'])

        self.try_command('breakpoint delete 5', ['1 breakpoints deleted'])

        self.try_command('process continue',
                         ['exited with status = 0'])
