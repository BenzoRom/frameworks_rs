'''Module that contains TestSuiteException.'''

from __future__ import absolute_import

class TestSuiteException(Exception):
    '''Exception that is thrown whenever an internal error is encountered.

    Just contains a message.
    '''
    pass

class DisconnectedException(Exception):
    '''Exception that is thrown if lldb-server unexpectedly disconnected.

    Just contains a message.
    '''
    pass


class FailFastException(TestSuiteException):
    '''Quick Bailout'''
    pass


class TestIgnoredException(TestSuiteException):
    '''Raised when a testcase is ignored.'''
    pass
