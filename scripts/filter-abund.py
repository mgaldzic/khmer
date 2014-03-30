#! /usr/bin/env python
#
# This file is part of khmer, http://github.com/ged-lab/khmer/, and is
# Copyright (C) Michigan State University, 2009-2014. It is licensed under
# the three-clause BSD license; see doc/LICENSE.txt.
# Contact: khmer-project@idyll.org
#
"""
Trim sequences at k-mers of the given abundance, based on the given counting
hash table.  Output sequences will be placed in 'infile.abundfilt'.

% python scripts/filter-abund.py <counting.kh> <data1> [ <data2> <...> ]

Use '-h' for parameter help.
"""
import os
import khmer
from khmer.thread_utils import ThreadedSequenceProcessor, verbose_loader
from khmer import threading_args as targs
from khmer.counting_args import build_counting_multifile_args
from khmer.file import check_file_status, check_space
#

DEFAULT_NORMALIZE_LIMIT = 20
DEFAULT_CUTOFF = 2


def main():
    parser = build_counting_multifile_args()
    targs.add_threading_args(parser)
    parser.add_argument('--cutoff', '-C', dest='cutoff',
                        default=DEFAULT_CUTOFF, type=int,
                        help="Trim at k-mers below this abundance.")

    parser.add_argument('-V', '--variable-coverage', action='store_true',
                        dest='variable_coverage', default=False)
    parser.add_argument('--normalize-to', '-Z', type=int, dest='normalize_to',
                        help='base variable-coverage cutoff on this median'
                        ' k-mer abundance',
                        default=DEFAULT_NORMALIZE_LIMIT)
    parser.add_argument('-o', '--out', dest='single_output_filename',
                        default='', help='only output a single'
                        ' file with the specified filename')
    args = parser.parse_args()

    counting_ht = args.input_table
    infiles = args.input_filenames
    n_threads = int(args.n_threads)

    for _ in infiles:
        check_file_status(_)

    check_space(infiles)

    print 'file with ht: %s' % counting_ht

    print 'loading hashtable'
    htable = khmer.load_counting_hash(counting_ht)
    ksize = htable.ksize()

    print "K:", ksize

    # the filtering function.
    def process_fn(record):
        name = record['name']
        seq = record['sequence']
        if 'N' in seq:
            return None, None

        if args.variable_coverage:  # only trim when sequence has high enough C
            med, _, _ = htable.get_median_count(seq)
            if med < args.normalize_to:
                return name, seq

        trim_seq, trim_at = htable.trim_on_abundance(seq, args.cutoff)

        if trim_at >= ksize:
            return name, trim_seq

        return None, None

    # the filtering loop
    for infile in infiles:
        print 'filtering', infile
        if args.single_output_filename != '':
            outfile = args.single_output_filename
            outfp = open(outfile, 'a')
        else:
            outfile = os.path.basename(infile) + '.abundfilt'
            outfp = open(outfile, 'w')

        tsp = ThreadedSequenceProcessor(process_fn, n_workers=n_threads)
        tsp.start(verbose_loader(infile), outfp)

        print 'output in', outfile

if __name__ == '__main__':
    main()
