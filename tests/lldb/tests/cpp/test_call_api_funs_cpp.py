'''Module that contains the test TestCallApiFunsCpp.'''

from harness.test_base_remote import TestBaseRemote
from harness import RS_funs


class TestCallApiFunsCpp(TestBaseRemote):
    '''Tests calling of some RS API functions in an NDK app.

     This tests that JITing works.'''

    def get_bundle_target(self):
        '''Return string with name of bundle executable to run.

        Returns:
            A string containing the name of the binary that this test
            can be run with.
        '''
        return "CppKernelVariables"

    def test_case(self, _):
        '''Run the lldb commands that are being tested.

        Raises:
            TestFail: One of the lldb commands did not provide the expected
                      output.
        '''
        self.try_command('language renderscript status',
                         ['Runtime Library discovered',
                          'Runtime Driver discovered'])

        self.try_command('b -f simple.rs -l 129', [])

        self.try_command('process continue',
                         ['resuming',
                          'stopped',
                          'stop reason = breakpoint'])

        # Test the atomics separately because we want to check the output
        # AtomicAdd(1234, 2)
        self.try_command('expr rsAtomicAdd(&int_global, 2)',
                         ['1234'],
                         [r'\(int(32_t)?\)'])

        self.try_command('expr int_global',
                         ['(int)',
                          '1236'])

        # AtomicAnd(2345, 333)
        self.try_command('expr rsAtomicAnd(&uint_global, 333)',
                         ['2345'],
                         [r'\(int(32_t)?\)'])

        self.try_command('expr uint_global',
                         ['(uint)',
                          '265'])

        # AtomicCas(1236, 1236, 2345)
        self.try_command('expr rsAtomicCas(&int_global, 1236, 2345)',
                         ['1236'],
                         [r'\(int(32_t)?\)'])

        self.try_command('expr int_global',
                         ['(int)',
                          '2345'])

        # AtomicDec(265)
        self.try_command('expr rsAtomicDec(&uint_global)',
                         ['265'],
                         [r'\(int(32_t)?\)'])

        self.try_command('expr uint_global',
                         ['(uint)',
                          '264'])

        # AtomicInc(2345)
        self.try_command('expr rsAtomicInc(&int_global)',
                         ['2345'],
                         [r'\(int(32_t)?\)'])

        self.try_command('expr int_global',
                         ['(int)',
                          '2346'])

        # AtomicMax(264, 3456)
        self.try_command('expr rsAtomicMax(&uint_global, 3456)',
                         ['264'],
                         [r'\(uint(32_t)?\)'])

        self.try_command('expr uint_global',
                         ['(uint)',
                          '3456'])

        # AtomicMin(2346, 3)
        self.try_command('expr rsAtomicMin(&int_global, 3)',
                         ['2346'],
                         [r'\(int(32_t)?\)'])

        self.try_command('expr int_global',
                         ['(int)',
                          '3'])

        # AtomicOr(3, 456)
        self.try_command('expr rsAtomicOr(&int_global, 456)',
                         ['3'],
                         [r'\(int(32_t)?\)'])

        self.try_command('expr int_global',
                         ['(int)',
                          '459'])

        # AtomicSub(3456, 7)
        self.try_command('expr rsAtomicSub(&uint_global, 7)',
                         ['3456'],
                         [r'\(int(32_t)?\)'])

        self.try_command('expr uint_global',
                         ['(uint)',
                          '3449'])

        # AtomicXor(459, 89)
        self.try_command('expr rsAtomicXor(&int_global, 89)',
                         ['459'],
                         [r'\(int(32_t)?\)'])

        self.try_command('expr int_global',
                         ['(int)',
                          '402'])

        for line in RS_funs.FUNC_LIST:
            # build the expression
            ret, expr = RS_funs.build_expr(line)
            # query return type table
            if ret in RS_funs.TYPE_MAP:
                # evaluate the expression
                self.try_command(expr, [], [RS_funs.TYPE_MAP[ret]])
            else:
                # evaluate the expression
                self.try_command(expr, '('+ret+')')

        self.try_command('breakpoint delete 1', ['1 breakpoints deleted'])

        self.try_command('process continue',
                         ['exited with status = 0'])
