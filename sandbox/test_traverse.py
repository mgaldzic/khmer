import khmer

def rc(kmer):
    ret = kmer.reverse()

K = 20
seq = 'ATCGTCGATCTACGACTACGACTCGATCGATCGACTCGATCGATC'

ind = 2

start = seq[ind:ind+K]
assert len(start) == K

left = seq[ind-1:ind-1+K]
right = seq[ind-1:ind-1+K]

assert len(left) == K
assert len(right) == K

lb = khmer.LabelHash(K, 1e9, 4)
lb.consume(seq)

tr_left = lb.traverse_from_kmer(start, 'l')
tr_right = lb.traverse_from_kmer(start, 'r')

print seq
print start

print 'expected L:', left
print 'actual L:', tr_left

print 'expected R:', right
print 'actual R:', tr_right


