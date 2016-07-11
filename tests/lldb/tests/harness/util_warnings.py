'''Redirect the Python warnings into the log.'''

from __future__ import absolute_import

import warnings

from . import util_log

_OLD_WARNINGS_HANDLER = None


def redirect_warnings():
    '''Redirect all warnings issued by warnings::warn to the log.

    By default all python warnings are printed into sys.stderr. This method
    will force to redirect them into the test suite logger.
    '''

    # pylint: disable=global-statement
    global _OLD_WARNINGS_HANDLER

    # Already redirecting?
    if _OLD_WARNINGS_HANDLER:
        return None

    _OLD_WARNINGS_HANDLER = warnings.showwarning

    log = util_log.get_logger()

    def _redirect_warnings_to_log(*args):
        '''Redirect the warnings to the Logger.'''
        log.warn(warnings.formatwarning(*args).rstrip())

    warnings.showwarning = _redirect_warnings_to_log


def restore_warnings():
    '''Restore the reporting of warnings::warn as before.'''

    # pylint: disable=global-statement
    global _OLD_WARNINGS_HANDLER

    if _OLD_WARNINGS_HANDLER:
        warnings.showwarning = _OLD_WARNINGS_HANDLER
        _OLD_WARNINGS_HANDLER = None

