'''Module that contains the test TestLanguageSubcmds.'''

import os

from harness.test_base_remote import TestBaseRemote


class TestLanguageSubcmds(TestBaseRemote):
    '''Tests the 'language renderscript' subcommands.'''

    def get_bundle_target(self):
        '''Return string with name of bundle executable to run.

        Returns:
            A string containing the name of the binary that this test can be run
            with.
        '''
        return 'JavaDebugWaitAttach'

    def test_setup(self, android):
        '''This test requires to be run on one thread.'''
        android.push_prop('debug.rs.max-threads', 1)

    def test_shutdown(self, android):
        '''Reset the number of RS threads to the previous value.'''
        android.pop_prop('debug.rs.max-threads')

    def test_case(self, _):
        '''Run the lldb commands that are being tested.'''
        self.try_command('language',
                         [])

        self.try_command('language renderscript status',
                         ['Runtime Library discovered',
                          'Runtime Driver discovered',
                          'Runtime functions hooked',
                          'rsdAllocationInit',
                          'rsdAllocationRead2D',
                          'rsdScriptInit',
                          'rsdScriptInvokeForEach',
                          'rsdScriptInvokeForEachMulti',
                          'rsdScriptSetGlobalVar'])

        self.try_command('breakpoint set --file simple.rs --line 12',
                         ['(pending)'])

        self.try_command('process continue',
                         [])

        self.try_command('language renderscript kernel',
                         ['breakpoint',
                          'coordinate',
                          'list'])

        self.try_command('language renderscript kernel breakpoint',
                         ['all',
                          'set'])

        self.try_command('language renderscript kernel list',
                         ['RenderScript Kernels',
                          "Resource 'simple'",
                          'root',
                          'simple_kernel'])

        self.try_command('language renderscript kernel coordinate',
                         ['Coordinate: (0, 0, 0)'])

        self.try_command('language renderscript context',
                         ['dump'])

        self.try_command('language renderscript context dump',
                         ['Inferred RenderScript Contexts',
                          '1 script instances'])

        self.try_command('language renderscript allocation',
                         ['list',
                          'load',
                          'save',
                          'dump',
                          'refresh'])

        self.try_command('language renderscript allocation list',
                         ['RenderScript Allocations:'])

        self.try_command('language renderscript allocation list -i 0',
                         ['RenderScript Allocations:'])

        self.try_command('language renderscript allocation list --id 0',
                         ['RenderScript Allocations:'])

        self.try_command('language renderscript allocation dump 1',
                         ['Data (X, Y, Z):'])

        output_file = self.get_tmp_file_path()
        self.try_command('language renderscript allocation dump 1 -f ' +
                         output_file,
                         ["Results written to '%s'" % output_file])

        if os.path.isfile(output_file):
            os.remove(output_file)

        self.try_command('language renderscript allocation dump 1 --file ' +
                         output_file,
                         ["Results written to '%s'" % output_file])

        self.try_command('language renderscript allocation save 1 ' +
                         output_file,
                         ["Allocation written to file '%s'" % output_file])

        self.try_command('language renderscript allocation load 1 ' +
                         output_file,
                         ["Contents of file '%s' read into allocation 1" %
                          output_file])

        self.try_command('language renderscript allocation refresh',
                         ['All allocations successfully recomputed'])

        self.try_command('language renderscript module',
                         ['dump'])

        self.try_command('language renderscript module dump',
                         ['RenderScript Modules:',
                          'librs.simple.so',
                          'Debug info loaded',
                          'Globals: 1',
                          'gColor - float4',
                          'Kernels: 3',
                          'root',
                          'simple_kernel',
                          'other_kernel',
                          'java_package_name: com.android.rs.waitattachdebug',
                          'version:'])
