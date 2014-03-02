'''
This file is part of khmer, http://github.com/ged-lab/khmer/, and is
Copyright (C) Michigan State University, 2009-2014. It is licensed under
the three-clause BSD license; see doc/LICENSE.txt.
Contact: khmer-project@idyll.org
'''

from cython.operator cimport dereference as deref
from libcpp.string cimport string


cdef extern from "khmer.hh" namespace "khmer":
  ctypedef unsigned long long int ExactCounterType
  ctypedef unsigned long long int HashIntoType
  ctypedef unsigned char WordLength


cdef extern from "ktable.hh" namespace "khmer":
    cdef cppclass KTable:
        KTable(long)
        ExactCounterType get_count(const char *)
        ExactCounterType get_count(HashIntoType)
        void count(const char *)
        void set_count(const char *, ExactCounterType c)
        void set_count(HashIntoType, ExactCounterType c)

        HashIntoType n_entries()
        const WordLength ksize() const
        const HashIntoType max_hash() const

        void consume_string(const string &)
        void clear()
        void update(const KTable &)
        KTable * intersect(const KTable &) const

    cdef HashIntoType _hash(const char*, const WordLength)
    cdef HashIntoType _hash(const char *, const WordLength,
                            HashIntoType&, HashIntoType&)
    cdef HashIntoType _hash_forward(const char *, WordLength)
    cdef string _revhash(HashIntoType, WordLength)


cdef class PyKTable:
    cdef KTable *thisptr

    def __cinit__(self, long size):
        self.thisptr = new KTable(size)

    cdef __update_ktable__(self, KTable *new):
        # FIXME: only used by intersect, there must be a better way
        del self.thisptr
        self.thisptr = new

    def __dealloc__(self):
        del self.thisptr

    def __len__(self):
        return self.thisptr.n_entries()

    def __getitem__(self, val):
        return self.get(val)

    def __contains__(self, const char* kmer):
        if self.thisptr.get_count(kmer) > 0:
            return True
        return False

    def forward_hash(self, const char* kmer):
        """Convert string to int"""
        if len(kmer) != self.thisptr.ksize():
            raise ValueError("k-mer length must be the same as the hashtable k-size")
        return _hash(kmer, self.thisptr.ksize())

    def forward_hash_no_rc(self, const char* kmer):
        """Convert string to int, with no reverse complement handling"""
        if len(kmer) != self.thisptr.ksize():
            raise ValueError("k-mer length must be the same as the hashtable k-size")
        return _hash_forward(kmer, self.thisptr.ksize())

    def reverse_hash(self, long val):
        """Convert int to string"""
        return _revhash(val, self.thisptr.ksize())

    def count(self, const char* kmer):
        """Count the given kmer"""
        if len(kmer) != self.thisptr.ksize():
            raise ValueError("k-mer length must be the same as the hashtable k-size")
        self.thisptr.count(kmer)
        return 1

    def consume(self, const string s):
        """Count all k-mers in the given string"""
        if len(s) < self.thisptr.ksize():
            raise ValueError("string length must >= the hashtable k-mer size")
        self.thisptr.consume_string(s)

        return len(s) - self.thisptr.ksize() + 1

    def get(self, val):
        "Get the count for the given k-mer"
        if isinstance(val, int) or isinstance(val, long):
            return self.thisptr.get_count(<HashIntoType>val);
        else:
            return self.thisptr.get_count(<char *>val);

    def max_hash(self):
        """Get the maximum hash value"""
        return self.thisptr.max_hash()

    def n_entries(self):
        """Get the number of possible entries"""
        return self.thisptr.n_entries()

    def ksize(self):
        """Get k"""
        return self.thisptr.ksize()

    def set(self, val, ExactCounterType c):
        """Set counter to a value"""
        if isinstance(val, int) or isinstance(val, long):
            self.thisptr.set_count(<HashIntoType>val, c);
        else:
            self.thisptr.set_count(<char *>val, c);

    def update(self, PyKTable other):
        """Combine another ktable with this one"""
        self.thisptr.update(deref(other.thisptr))

    def intersect(self, PyKTable other):
        """
        Create another ktable containing the intersection of two ktables:
        where both ktables have an entry, the counts will be summed.
        """
        # FIXME: there must be a better way
        cdef KTable *intersection
        intersection = self.thisptr.intersect(deref(other.thisptr))
        ktable = PyKTable(1)
        ktable.__update_ktable__(intersection)
        return ktable

    def clear(self):
        """Set all entries to 0."""
        self.thisptr.clear()


def new_ktable(size):
    """ Create an empty ktable """
    return PyKTable(size)
