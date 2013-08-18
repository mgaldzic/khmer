#! /usr/bin/env python
"""
Take an interleaved set of reads (/1 and /2), and extract them into separate
files (.1 and .2).

% scripts/split-paired-reads.py <infile>

Reads FASTQ and FASTA input, retains format for output.
"""
import screed
import sys
import os.path

def main():
    infile = sys.argv[1]
    out1 = os.path.basename(infile) + '.1'
    out2 = os.path.basename(infile) + '.2'
    fp1 = open(out1, 'w')
    fp2 = open(out2, 'w')

    # is input file FASTQ or FASTA? Determine.
    is_fastq = False
    record = iter(screed.open(infile)).next()

    if hasattr(record, 'accuracy'):
        is_fastq = True

    n1 = 0
    n2 = 0
    for n, record in enumerate(screed.open(infile)):
        if n % 100000 == 0:
            print >>sys.stderr, '...', n

        name = record.name
        if name.endswith('/1'):
            if is_fastq:
                print >>fp1, '@%s\n%s\n+\n%s' % (record.name, record.sequence,
                                                 record.accuracy)
            else:
                print >>fp1, '>%s\n%s' % (record.name, record.sequence,)
            n1 += 1
        elif name.endswith('/2'):
            if is_fastq:
                print >>fp2, '@%s\n%s\n+\n%s' % (record.name, record.sequence,
                                                 record.accuracy)
            else:
                print >>fp2, '>%s\n%s' % (record.name, record.sequence,)
            n2 += 1

    print >>sys.stderr, "DONE; split %d sequences (%d left, %d right)" % \
        (n + 1, n1, n2)
    print >>sys.stderr, "/1 reads in %s" % out1
    print >>sys.stderr, "/2 reads in %s" % out2

if __name__ == '__main__':
    main()
    
