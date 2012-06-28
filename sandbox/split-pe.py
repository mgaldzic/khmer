#! /usr/bin/env python
import screed
import sys
import os.path

fp1 = open(os.path.basename(sys.argv[1]) + '.1', 'w')
fp2 = open(os.path.basename(sys.argv[1]) + '.2', 'w')

n1 = 0
n2 = 0
for n, record in enumerate(screed.open(sys.argv[1])):
    if n % 100000 == 0:
        print >>sys.stderr, '...', n

    name = record.name
    if name.endswith('/1'):
        print >>fp1, '>%s\n%s' % (record.name, record.sequence,)
        n1 += 1
    elif name.endswith('/2'):
        print >>fp2, '>%s\n%s' % (record.name, record.sequence,)
        n2 += 1

print >>sys.stderr, "DONE; split %d sequences (%d left, %d right)" % \
      (n + 1, n1, n2)
