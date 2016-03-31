#!/usr/bin/env python2.7
'''This script will run one specific test.'''
from __future__ import print_function

import sys
import atexit
import inspect
import logging
import os

import harness
from harness import util_constants
from harness import util_log
from harness import util_warnings
from harness.util_functions import load_py_module
from harness.util_lldb import UtilLLDB
from harness.exception import DisconnectedException
from harness.exception import TestSuiteException
from harness.util_timer import Timer


def _test_pre_run(state):
    '''This function is called before a test is executed (setup).

    Args:
        state: Test suite state collection, instance of State.

    Returns:
        True if the pre_run step completed without error. Currently the pre-run
        will launch the target test binary on the device and attach an
        lldb-server to it in platform mode.

    Raises:
        AssertionError: If an assertion fails.
        TestSuiteException: Previous processes of this apk required for this
                            test could not be killed.
    '''
    assert state.test
    assert state.bundle

    log = util_log.get_logger()
    log.info('running: {0}'.format(state.name))

    # Remove any cached NDK scripts between tests
    state.bundle.delete_ndk_cache()

    # query our test case for the remote target app it needs
    target_name = state.test.get_bundle_target()
    if not target_name:
        # test case doesn't require a remote process to debug
        return True
    else:
        # find the pid of our remote test process
        state.pid = state.bundle.launch(target_name)
        if not state.pid:
            log.error('unable to get pid of target')
            return False
        state.android.kill_servers()
        # spawn lldb platform on the target device
        state.android.launch_lldb_platform(state.device_port)
        return True


def _test_post_run(state):
    '''This function is called after a test is executed (cleanup).

    Args:
        state: Test suite state collection, instance of State.

    Raises:
        AssertionError: If an assertion fails.
    '''
    assert state.test
    assert state.bundle

    target_name = state.test.get_bundle_target()
    if target_name:
        if state.bundle.is_apk(target_name):
            state.android.stop_app(state.bundle.get_package(target_name))
        else:
            state.android.kill_process(target_name)


def _test_run(state):
    '''Execute a single test case.

    Args:
        state: test suite state collection, instance of State.

    Returns:
        True: if the test case ran successfully and passed.
        False: if the test case failed or suffered an error.

    Raises:
        AssertionError: If an assertion fails.
    '''
    assert state.lldb
    assert state.lldb_module
    assert state.test

    if not state.test.run(state.lldb, state.pid, state.lldb_module,
                          state.wimpy):
        util_log.get_logger().error('test {0} failed'.format(state.name))
        return False

    return True


def _initialise_timer(android, interval):
    '''Start a 'timeout' timer, to catch stalled execution.

    This function will start a timer that will act as a timeout killing this
    test session if a test becomes un-responsive.

    Args:
        android: current instance of harness.UtilAndroid
        interval: the interval for the timeout, in seconds

    Returns:
        The instance of the Timer class that was created.
    '''

    def on_timeout():
        '''This is a callback function that will fire if a test takes longer
        then a threshold time to complete.'''
        # Clean up the android properties
        android.reset_all_props()
        # pylint: disable=protected-access
        sys.stdout.flush()
        # hard exit to force kill all threads that may block our exit
        os._exit(util_constants.RC_TEST_TIMEOUT)

    timer = Timer(interval, on_timeout)
    timer.start()
    atexit.register(Timer.stop, timer)
    return timer


def _quit_test(num, timer):
    '''This function will exit making sure the timeout thread is killed.

    Args:
        num: An integer specifying the exit status, 0 meaning "successful
             termination".
        timer: The current Timer instance.
    '''
    if timer:
        timer.stop()
    sys.stdout.flush()
    sys.exit(num)


def _execute_test(state):
    '''Execute a test case.

    Args:
        state: The current State object.
        log: The current Log object.
    '''
    log = util_log.get_logger()

    state.test.test_setup(state.android)
    try:
        if not _test_pre_run(state):
            raise TestSuiteException('test_pre_run() failed')
        if not _test_run(state):
            raise TestSuiteException('test_run() failed')
        _test_post_run(state)
        log.info('Test passed')

    finally:
        state.test.post_run()
        state.test.test_shutdown(state.android)


def _get_test_case_class(module):
    '''Inspect a test case module and return the test case class.

    Args:
        module: A loaded test case module.
    '''
    for _, obj in inspect.getmembers(module):
        if inspect.isclass(obj):
            if (hasattr(module, "TestBase") and
                issubclass(obj, module.TestBase) and
                obj is not module.TestBase):
                return obj
            if (hasattr(module, "TestBaseRemote") and
                issubclass(obj, module.TestBaseRemote) and
                obj is not module.TestBaseRemote):
                return obj
    return None


def get_test_dir(test_name):
    ''' Get the directory that contains a test with a given name.

    Returns:
        A string that is the directory containing the test.

    Raises:
        TestSuiteException: If a test with this name does not exist.
    '''
    tests_dir = os.path.dirname(os.path.realpath(__file__))
    for sub_dir in os.listdir(tests_dir):
        current_test_dir = os.path.join(tests_dir, sub_dir)
        if (os.path.isdir(current_test_dir) and
            test_name in os.listdir(current_test_dir)):
            return current_test_dir

    raise TestSuiteException(
        'unable to find test: {0}'.format(test_name))


def main():
    '''Test runner entry point.'''

    # re-open stdout with no buffering
    sys.stdout = os.fdopen(sys.stdout.fileno(), 'w', 0)

    android = None
    timer = None
    log = None

    try:
        # parse the command line
        if len(sys.argv) < 12:
            raise TestSuiteException('Invalid number of arguments')

        assert sys.argv[8] in ('True', 'False')
        args = dict(test_name=sys.argv[1],
                    log_file_path=sys.argv[2],
                    adb_path=sys.argv[3],
                    lldb_server_path_device=sys.argv[4],
                    aosp_product_path=sys.argv[5],
                    device_port=int(sys.argv[6]),
                    device=sys.argv[7],
                    print_to_stdout=sys.argv[8] == 'True',
                    verbose=sys.argv[9] == 'True',
                    wimpy=sys.argv[10] == 'True',
                    timeout=int(sys.argv[11]))

        # create utility classes
        harness.util_log.initialise(args['test_name'],
            print_to_stdout=args['print_to_stdout'],
            level=logging.INFO if not args['verbose'] else logging.DEBUG,
            file_path=args['log_file_path'],
            file_mode='a'
        )
        log = util_log.get_logger()
        log.debug('Logger initialised')

        android = harness.UtilAndroid(args['adb_path'],
                                      args['lldb_server_path_device'],
                                      args['device'])

        # start the timeout counter
        timer = _initialise_timer(android, args['timeout'])

        # startup lldb and register teardown handler
        atexit.register(UtilLLDB.stop)
        UtilLLDB.start()

        current_test_dir = get_test_dir(args['test_name'])

        # load a test case module
        test_module = load_py_module(os.path.join(current_test_dir,
                                                  args['test_name']))
        assert test_module
        assert (hasattr(test_module, "TestBase") or
                hasattr(test_module, "TestBaseRemote"))

        # inspect the test module and locate our test case class
        test_class = _get_test_case_class(test_module)
        assert test_class

        # if our test inherits from TestBaseRemote, check we have a valid device
        if (hasattr(test_module, "TestBaseRemote") and
            issubclass(test_class, test_module.TestBaseRemote)):
            android.validate_device()

        # create an instance of our test case
        test_inst = test_class(args['device_port'], args['device'], timer)
        assert test_inst

        # instantiate a test target bundle
        bundle = harness.UtilBundle(android, args['aosp_product_path'])

        # execute the test case
        try:
            for _ in range(2):
                try:
                    # create an lldb instance
                    lldb = UtilLLDB.create_debugger()

                    # create state object to encapsulate instances
                    state = type('State',
                                 (object,),
                                 dict(
                                     android=android,
                                     bundle=bundle,
                                     lldb=lldb,
                                     lldb_module=UtilLLDB.get_module(),
                                     test=test_inst,
                                     pid=None,
                                     name=args['test_name'],
                                     device_port=args['device_port'],
                                     wimpy=args['wimpy'])
                                 )

                    util_warnings.redirect_warnings()

                    _execute_test(state)

                    # tear down the lldb instance
                    UtilLLDB.destroy_debugger(lldb)
                    break
                except DisconnectedException as error:
                    log.warning(error)
                    log.warning('Trying again.')
            else:
                log.fatal('Not trying again, maximum retries exceeded.')
                raise TestSuiteException('Lost connection to lldb-server')

        finally:
            util_warnings.restore_warnings()

        _quit_test(util_constants.RC_TEST_OK, timer)

    except AssertionError:
        if log:
            log.critical('Internal test suite error', exc_info=1)
        print('Internal test suite error', file=sys.stderr)
        _quit_test(util_constants.RC_TEST_FATAL, timer)

    except TestSuiteException as error:
        if log:
            log.exception(str(error))
        else:
            print(str(error), file=sys.stderr)
        _quit_test(util_constants.RC_TEST_ERROR, timer)

    # use a global exception handler to be sure that we will
    # exit safely and correctly
    except Exception:
        if log:
            log.exception('INTERNAL ERROR')
        else:
            import traceback
            print('Exception {0}'.format(traceback.format_exc()),
                  file=sys.stderr)
        _quit_test(util_constants.RC_TEST_FATAL, timer)

    finally:
        if android:
            android.reset_all_props()
        if timer:
            timer.stop()


# execution trampoline
if __name__ == '__main__':
    main()
