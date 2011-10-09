import khmer

import khmer_tst_utils as utils

READTABLE_SIZE=50

def teardown():
    utils.cleanup()

class Test_Basic(object):
    def __init__(self):
        self.rt = khmer.new_readmask(READTABLE_SIZE)
        
    def test_tablesize(self):
        assert self.rt.tablesize() == READTABLE_SIZE

    def test_set_true(self):
        rt = self.rt

        rt.set(0, True)
        assert rt.get(0)
        assert rt.n_kept() == READTABLE_SIZE, rt.n_kept()

    def test_set_false(self):
        rt = self.rt

        rt.set(0, False)
        assert not rt.get(0)
        assert rt.n_kept() == READTABLE_SIZE - 1, rt.n_kept()

    def test_invert(self):
        rt = self.rt

        rt.set(0, False)
        rt.set(1, True)
        rt.invert()
        
        assert rt.get(0)
        assert not rt.get(1)

    def test_do_and_1(self):
        rt = self.rt

        rt.set(0, True)
        rt.do_and(0, True)
        assert rt.get(0)

    def test_do_and_2(self):
        rt = self.rt

        rt.set(0, True)
        rt.do_and(0, False)
        assert not rt.get(0)

    def test_do_and_3(self):
        rt = self.rt

        rt.set(0, False)
        rt.do_and(0, True)
        assert not rt.get(0)

    def test_do_and_4(self):
        rt = self.rt

        rt.set(0, False)
        rt.do_and(0, False)
        assert not rt.get(0)

    def test_merge_1(self):
        rt = self.rt
        rt2 = khmer.new_readmask(READTABLE_SIZE)

        rt.set(0, True)
        rt2.set(0, True)

        rt.merge(rt2)

        v = rt.get(0)
        assert v, v

    def test_merge_2(self):
        rt = self.rt
        rt2 = khmer.new_readmask(READTABLE_SIZE)

        rt.set(0, True)
        rt2.set(0, False)

        rt.merge(rt2)

        v = rt.get(0)
        assert not v, v

    def test_merge_3(self):
        rt = self.rt
        rt2 = khmer.new_readmask(READTABLE_SIZE)

        rt.set(0, False)
        rt2.set(0, True)

        rt.merge(rt2)

        v = rt.get(0)
        assert not v, v

    def test_merge_4(self):
        rt = self.rt
        rt2 = khmer.new_readmask(READTABLE_SIZE)

        rt.set(0, False)
        rt2.set(0, False)

        rt.merge(rt2)

        v = rt.get(0)
        assert not v, v

class Test_Filestuff(object):
    def __init__(self):
        self.rt = khmer.new_readmask(READTABLE_SIZE)

    def test_saveload(self):
        filename = utils.get_temp_filename('tst')

        rt = self.rt
        
        rt.set(0, True)
        rt.set(1, False)
        rt.set(2, True)
        rt.set(3, False)
        rt.set(4, True)

        rt.save(filename)
        
        rt2 = khmer.new_readmask(0)
        rt2.load(filename)

        for i in range(5):
            assert rt.get(i) == rt2.get(i), i

    def test_save_no_load(self):
        filename = utils.get_temp_filename('tst')

        rt = self.rt
        
        rt.set(0, True)
        rt.set(1, False)
        rt.set(2, True)
        rt.set(3, False)
        rt.set(4, True)

        rt.save(filename)
        
        rt2 = khmer.new_readmask(READTABLE_SIZE)
        # no load!

        try:
            for i in range(5):
                if rt.get(i) != rt2.get(i):
                    raise Exception     # supposed to happen; no load.
            assert 0
        except AssertionError:
            raise
        except Exception:
            pass
