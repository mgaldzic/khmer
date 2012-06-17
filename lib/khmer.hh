#define VERSION "0.4"

#define MAX_COUNT 255
#define MAX_BIGCOUNT 65535
#define DEFAULT_TAG_DENSITY 40		// must be even

#define MAX_CIRCUM 3		// @CTB remove
#define CIRCUM_RADIUS 2		// @CTB remove
#define CIRCUM_MAX_VOL 200	// @CTB remove

#define SAVED_FORMAT_VERSION 3
#define SAVED_COUNTING_HT 1
#define SAVED_HASHBITS 2
#define SAVED_TAGS 3
#define SAVED_STOPTAGS 4
#define SAVED_SUBSET 5

#define VERBOSE_REPARTITION 0

namespace khmer {
  // largest number we can count up to, exactly. (8 bytes)
  typedef unsigned long long int ExactCounterType;

  // largest number we're going to hash into. (8 bytes/64 bits/32 nt)
  typedef unsigned long long int HashIntoType;

  // largest size 'k' value for k-mer calculations.  (1 byte/255)
  typedef unsigned char WordLength;

  typedef unsigned short int BoundedCounterType;

  // A single-byte type.
  typedef unsigned char Byte;

  typedef void (*CallbackFn)(const char * info, void * callback_data,
			     unsigned long long n_reads,
			     unsigned long long other);

};
