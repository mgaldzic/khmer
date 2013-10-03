//
// This file is part of khmer, http://github.com/ged-lab/khmer/, and is
// Copyright (C) Michigan State University, 2009-2013. It is licensed under
// the three-clause BSD license; see doc/LICENSE.txt. Contact: ctb@msu.edu
//

// Simple C++ implementation of the 'load-graph' Python script.


#include <cstring>
#include <cstdio>
#include <cerrno>
#include <cstdlib>
#include <unistd.h>
#include <getopt.h>
#include <time.h>
#include <omp.h>

//#define HASH_TYPE_TO_TEST   1 // Counting Hash
#define HASH_TYPE_TO_TEST   2 // Bit Hash

// #define OUTPUT_HASHTABLE


#include "error.hh"
#include "read_parsers.hh"
#if HASH_TYPE_TO_TEST == 1
#  include "counting.hh"
#elif HASH_TYPE_TO_TEST == 2
#  include "hashbits.hh"
#else
#  error "No HASH_TYPE_TO_TEST macro defined."
#endif
#include "primes.hh"

using namespace std;
using namespace khmer;
using namespace khmer:: read_parsers;




int main( int argc, char * argv[ ] )
{
    unsigned long	kmer_length	    = 20;
    float		ht_size_FP	    = 1.0E8;
    unsigned long	ht_count	    = 4;
    uint64_t		cache_size	    = 4L * 1024 * 1024 * 1024;
    unsigned int	range		    = 40;
    int			rc		    = 0;
    int			opt		    = -1;
    char *		conv_residue	    = NULL;
    string		rfile_name = "/mnt/scratch/tg/w/khmer/tests/test-data/test-reads.fa";
    string		ifile_name = "/mnt/scratch/tg/w/khmer/tests/test-data/test-reads.fa";
    // FILE *		ofile		    = NULL;
    HashIntoType	    ht_size		= (HashIntoType)ht_size_FP;
    Primes primetab( ht_size );
    vector<HashIntoType> ht_sizes;
    for ( unsigned int i = 0; i < ht_count; ++i )
	ht_sizes.push_back( primetab.get_next_prime( ) );

    unsigned int	    reads_total		= 0;
    unsigned long long int  n_consumed		= 0;

    Hashbits ht( kmer_length, ht_sizes );
    ht.consume_fasta_and_tag_with_colors( ifile_name, reads_total, n_consumed );
    IParser * parser = IParser:: get_parser(rfile_name.c_str());
    Read read;
    unsigned int num_traversed;
    string seq = "";
    clock_t st;
    while(!parser->is_complete()) {
	read = parser->get_next_read();
	seq = read.sequence;
	st = clock();
	ColorPtrSet found_colors;
	num_traversed = ht.sweep_color_neighborhood(seq, found_colors, range, false, false);
	st = clock() - st;
	printf("traversed %u reads in %d ticks (%f seconds)\n", num_traversed,
								st,
								((float)st/CLOCKS_PER_SEC));
	
    }
    return rc;
}


// vim: set sts=4 sw=4 tw=80:
