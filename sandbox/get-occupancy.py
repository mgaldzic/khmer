import khmer
import sys
import screed
import os
import subprocess
import glob
import gzip

K = 32
HASHTABLE_SIZE=int(1e9)
#HASHTABLE_SIZE = 1000000000

print 'creating ht with size ' + str(HASHTABLE_SIZE)
ht = khmer.new_hashbits(K, HASHTABLE_SIZE, 1)

read_count = 0

files = sys.argv[1:]

for filename in files:
   print "processing file: " + filename + " reads processed: " + str(read_count)
 
   for n, record in enumerate(screed.fasta.fasta_iter(open(filename))):
      read_count += 1
      if len(record['sequence']) >= K:
         ht.consume(record['sequence'])

      if read_count % 10000 == 0:
         print str(read_count), str(ht.n_occupied())

print str(read_count), str(ht.n_occupied())

