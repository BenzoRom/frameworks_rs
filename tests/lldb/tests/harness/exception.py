'''Module that contains TestSuiteException.'''

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
