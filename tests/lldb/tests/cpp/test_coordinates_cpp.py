'''Module that contains the test TestCoordinatesCpp.'''

from harness.test_base_remote import TestBaseRemote


class TestCoordinatesCpp(TestBaseRemote):
    '''Tests the inspection of coordinates in an NDK app.

    Tests the inspection of the range and dimension of coordinates as well
    as the current coordinates.'''

    def get_bundle_target(self):
        '''Return string with name of bundle executable to run.

        Returns:
            A string containing the name of the binary that this test can be run
            with.
        '''
        return 'CppDebugWaitAttach'

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

    def test_case(self, _):
        '''Run the lldb commands that are being tested.

        Raises:
            TestFail: One of the lldb commands did not provide the expected
                      output.
        '''
        # pylint: disable=line-too-long
        self.try_command('language renderscript status',
                         ['Runtime Library discovered',
                          'Runtime Driver discovered'])

        self.try_command('language renderscript kernel breakpoint set simple_kernel',
                         ['Breakpoint(s) created',
                          '(pending)'])

        # Check the initial conditions.
        self._lldb_continue()
        self._inspect_coordinates((0, 0, 0))

        # Check two more steps.
        self._lldb_continue()
        self._inspect_coordinates((1, 0, 0))
        self._lldb_continue()
        self._inspect_coordinates((2, 0, 0))

        # After eight more steps we should have advanced one step in the y dimension.
        for _ in range(8):
            self._lldb_continue()
        self._inspect_coordinates((2, 1, 0))

        self.try_command('breakpoint delete 1', ['1 breakpoints deleted'])

        self.try_command('process continue',
                         ['exited with status = 0'])

    def _lldb_continue(self):
        '''Try 'continue' lldb command. Expect to hit a breakpoint.'''
        self.try_command('process continue',
                         ['resuming',
                          'stopped',
                          'stop reason = breakpoint'])

    def _inspect_coordinates(self, (x_coord, y_coord, z_coord)):
        '''Run lldb commands to inspect kernel size and coordinates
        and match against expected values.

        Args:
            (x_coord, y_coord, z_coord): The expected coordinates (int triple)

        Raises:
            TestFail: One of the lldb commands did not provide the expected
                      output.
        '''
        self.try_command('language renderscript kernel coordinate',
                         ['Coordinate: (%d, %d, %d)' % (x_coord, y_coord,
                                                        z_coord)])

        self.try_command('frame select 1',
                         ['librs.simple', 'simple_kernel.expand',
                         'at generated.rs:1'])

        # Inspect the invocation length, should be the same every time.
        self.try_command('expr p->dim',
                         ['x = 8',
                          'y = 8',
                          'z = 0'])

        # The X coordinate is in the rsIndex variable.
        self.try_command('expr rsIndex',
                          ['= ' + str(x_coord)])

        # Inspect the Y and Z coordinates.
        self.try_command('expr p->current',
                         ['x = ' + str(0),
                          'y = ' + str(y_coord),
                          'z = ' + str(z_coord)])
