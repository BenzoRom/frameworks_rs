'''This file contains utility functions used by both the test suite and the
single test executor.'''

import os
import importlib
import sys


def load_py_module(path):
    '''Load a python file from disk.

    Args:
        path: String path to python file.

    Returns:
        python module if success, None otherwise.
    '''
    assert isinstance(path, str)
    try:
        if not os.path.exists(path):
            print 'Path does not exist: ' + path
            return None
        path = os.path.abspath(path)
        module_dir, module_file = os.path.split(path)
        module_name, _ = os.path.splitext(module_file)
        # adjust sys.path, runtime counterpart of PYTHONPATH, to temporarily
        # include the folder containing the user configuration module
        sys.path.append(module_dir)
        module_obj = importlib.import_module(module_name)
        sys.path.pop(0)
        return module_obj
    except ImportError as err:
        print str(err)
        print "Looking in directory "
        print module_dir
        return None
