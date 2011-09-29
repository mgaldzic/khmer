#! /usr/bin/env python
import sys
import khmer
from screed.fasta import fasta_iter

K = 32
HASHTABLE_SIZE=int(1e9)
N_HASHTABLES=4

ht = khmer.new_counting_hash(32, HASHTABLE_SIZE, N_HASHTABLES)

filename = sys.argv[1]

ht.consume_fasta(filename)
total, count, mean = ht.get_kmer_abund_mean(filename);
abs_dev = ht.get_kmer_abund_abs_deviation(filename, mean);

print total, count, mean, abs_dev
