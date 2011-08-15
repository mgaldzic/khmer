#ifndef COUNTING_HH
#define COUNTING_HH

#include <vector>
#include "hashtable.hh"
#include "hashbits.hh"

namespace khmer {
  typedef std::map<HashIntoType, BoundedCounterType> KmerCountMap;

  class CountingHashIntersect;

  class CountingHash : public khmer::Hashtable {
    friend class CountingHashIntersect;

  protected:
    bool _use_bigcount;		// keep track of counts > MAX_COUNT?
    std::vector<HashIntoType> _tablesizes;
    unsigned int _n_tables;

    Byte ** _counts;

    virtual void _allocate_counters() {
      _n_tables = _tablesizes.size();

      _counts = new Byte*[_n_tables];
      for (unsigned int i = 0; i < _n_tables; i++) {
	_counts[i] = new Byte[_tablesizes[i]];
	memset(_counts[i], 0, _tablesizes[i]);
      }
    }
  public:
    KmerCountMap _bigcounts;

    CountingHash(WordLength ksize, HashIntoType single_tablesize) :
      khmer::Hashtable(ksize), _use_bigcount(false) {
      _tablesizes.push_back(single_tablesize);
      
      _allocate_counters();
    }

    CountingHash(WordLength ksize, std::vector<HashIntoType>& tablesizes) :
      khmer::Hashtable(ksize), _use_bigcount(false), _tablesizes(tablesizes) {

      _allocate_counters();
    }

    virtual ~CountingHash() {
      if (_counts) {
	for (unsigned int i = 0; i < _n_tables; i++) {
	  delete _counts[i];
	  _counts[i] = NULL;
	}

	delete _counts;
	_counts = NULL;

	_n_tables = 0;
      }
    }

    std::vector<HashIntoType> get_tablesizes() const {
      return _tablesizes;
    }

    void set_use_bigcount(bool b) { _use_bigcount = b; }
    bool get_use_bigcount() { return _use_bigcount; }

    virtual void save(std::string);
    virtual void load(std::string);

    // accessors to get table info
    const HashIntoType n_entries() const { return _tablesizes[0]; }

    // count number of occupied bins
    virtual const HashIntoType n_occupied(HashIntoType start=0,
					  HashIntoType stop=0) const {
      HashIntoType n = 0;
      if (stop == 0) { stop = _tablesizes[0]; }
      for (HashIntoType i = start; i < stop; i++) {
	if (_counts[0][i % _tablesizes[0]]) {
	  n++;
	}
      }
      return n;
    }

    virtual void count(const char * kmer) {
      HashIntoType hash = _hash(kmer, _ksize);
      count(hash);
    }

    virtual void count(HashIntoType khash) {
      unsigned int n_full = 0;
      for (unsigned int i = 0; i < _n_tables; i++) {
	const HashIntoType bin = khash % _tablesizes[i];

	if (_counts[i][bin] < MAX_COUNT) {
	  _counts[i][bin] += 1;
	} else {
	  n_full++;
	}
      }

      if (n_full == _n_tables && _use_bigcount) {
	if (_bigcounts[khash] == 0) {
	  _bigcounts[khash] = MAX_COUNT + 1;
	} else {
	  if (_bigcounts[khash] < MAX_BIGCOUNT) {
	    _bigcounts[khash] += 1;
	  }
	}
      }
    }

    // get the count for the given k-mer.
    virtual const BoundedCounterType get_count(const char * kmer) const {
      HashIntoType hash = _hash(kmer, _ksize);
      return get_count(hash);
    }

    // get the count for the given k-mer hash.
    virtual const BoundedCounterType get_count(HashIntoType khash) const {
      BoundedCounterType min_count = MAX_COUNT;
      for (unsigned int i = 0; i < _n_tables; i++) {
	BoundedCounterType the_count = _counts[i][khash % _tablesizes[i]];
	if (the_count < min_count) {
	  min_count = the_count;
	}
      }
      if (min_count == MAX_COUNT && _use_bigcount) {
	KmerCountMap::const_iterator it = _bigcounts.find(khash);
	if (it != _bigcounts.end()) {
	  min_count = it->second;
	}
      }
      return min_count;
    }

    //

    MinMaxTable * fasta_file_to_minmax(const std::string &inputfile,
				       unsigned int total_reads,
				       ReadMaskTable * readmask = NULL,
				       CallbackFn callback = NULL,
				       void * callback_data = NULL);

    ReadMaskTable * filter_fasta_file_any(MinMaxTable &minmax,
					  BoundedCounterType threshold,
					  ReadMaskTable * readmask = NULL,
					  CallbackFn callback = NULL,
					  void * callback_data = NULL);

    ReadMaskTable * filter_fasta_file_all(MinMaxTable &minmax,
					  BoundedCounterType threshold,
					  ReadMaskTable * readmask = NULL,
					  CallbackFn callback = NULL,
					  void * callback_data = NULL);

    ReadMaskTable * filter_fasta_file_limit_n(const std::string &readsfile,
                                              MinMaxTable &minmax,
                                              BoundedCounterType threshold,
                                              BoundedCounterType n, 
                                              ReadMaskTable * old_readmask = NULL,
                                              CallbackFn callback = NULL,
                                              void * callback_data = NULL);

    ReadMaskTable * filter_fasta_file_run(const std::string &inputfile,
					  unsigned int total_reads,
					  BoundedCounterType threshold,
					  unsigned int runlength,
					  ReadMaskTable * old_readmask = NULL,
					  CallbackFn callback = NULL,
					  void * callback_data = NULL);

    void output_fasta_kmer_pos_freq(const std::string &inputfile,
                                    const std::string &outputfile);

    BoundedCounterType get_min_count(const std::string &s,
				     HashIntoType lower_bound = 0,
				     HashIntoType upper_bound = 0);
				     
    BoundedCounterType get_max_count(const std::string &s,
				     HashIntoType lower_bound = 0,
				     HashIntoType upper_bound = 0);

    void get_median_count(const std::string &s,
			  BoundedCounterType &median,
			  float &average,
			  float &stddev);

    HashIntoType * abundance_distribution(std::string filename,
					  Hashbits * tracking,
					  CallbackFn callback = NULL,
					  void * callback_data = NULL) const;

    HashIntoType * fasta_count_kmers_by_position(const std::string &inputfile,
					 const unsigned int max_read_len,
					 ReadMaskTable * old_readmask = NULL,
					 BoundedCounterType limit_by_count=0,
						 CallbackFn callback = NULL,
						 void * callback_data = NULL);

    void fasta_dump_kmers_by_abundance(const std::string &inputfile,
				       ReadMaskTable * readmask,
				       BoundedCounterType limit_by_count,
				       CallbackFn callback = NULL,
				       void * callback_data = NULL);

    void get_kmer_abund_mean(const std::string &inputfile,
			     unsigned long long &total,
			     unsigned long long &count,
			     float &mean) const;

    void get_kmer_abund_abs_deviation(const std::string &inputfile,
				      float mean, float &abs_deviation) const;

    unsigned int max_hamming1_count(const std::string kmer);

    unsigned int trim_on_abundance(std::string seq,
				   BoundedCounterType min_abund) const;
  };
};

#endif // COUNTING_HH
