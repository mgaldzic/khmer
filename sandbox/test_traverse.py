import khmer

def rc(kmer):
    ret = ''
    for i in range(len(kmer)-1,-1,-1):
        base = kmer[i]
        if base == 'A':
            ret += 'T'
        elif base == 'C':
            ret += 'G'
        elif base == 'G':
            ret += 'C'
        else:
            ret += 'A'
    return ret
    
K = 20
seq = 'ATCGTCGATCTACGACTACGACTCGATCGATCGACTCGATCGATC'

ind = 2

start = seq[ind:ind+K]
assert len(start) == K

left = seq[ind-1:ind-1+K]
right = seq[ind+1:ind+1+K]

assert len(left) == K
assert len(right) == K

lb = khmer.LabelHash(K, 1e9, 4)
lb.consume(seq)

tr_left = lb.traverse_from_kmer(start, 'l')
tr_right = lb.traverse_from_kmer(start, 'r')

print 'sequence', seq
print 'start', start
print '*' * 40

print 'expected L:', left, rc(left)
print 'actual L:', tr_left, rc(tr_left[0])
print '*' * 40

print 'expected R:', right, rc(right)
print khmer.forward_hash(right, K)
print khmer.forward_hash(rc(right), K)
print lb.get(right)
print lb.get(rc(right))

print seq.find(right)

print '=' * 40
print 'actual R:', tr_right[0], rc(tr_right[0])
print khmer.forward_hash(tr_right[0], K)
print khmer.forward_hash(rc(tr_right[0]), K)
print lb.get(tr_right[0])
print lb.get(rc(tr_right[0]))

print seq.find(tr_right[0])
print seq.find(rc(tr_right[0]))
