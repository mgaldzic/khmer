#!/bin/sh

python ../graph-size.py 31 2104388177 4 1 200 ../../data/msb2.fq msb2.fa
python ../graph-size.py 31 2104388177 4 1 200 msb2.fa msb2a.fa
rm msb2.fa
mv msb2a.fa msb2.fa
python ../load-graph.py -k 31 --n_hashes 4 --hashsize 2104388177 msb2 msb2.fa
python ../partition-graph.py --threads 1 msb2
python ../merge-partitions.py -k 31 msb2
python ../annotate-partitions.py -k 31 msb2 msb2.fa
python ../extract-partitions.py msb2 msb2.fa.part
./assemble.sh
