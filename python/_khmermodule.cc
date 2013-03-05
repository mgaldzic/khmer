//
// A module for Python that exports khmer C++ library functions.
//

#include <iostream>

#include "Python.h"
#include "khmer.hh"
#include "khmer_config.hh"
#include "ktable.hh"
#include "hashtable.hh"
#include "hashbits.hh"
#include "counting.hh"
#include "storage.hh"

//
// Function necessary for Python loading:
//

extern "C" {
  void init_khmer();
}

class _khmer_exception {
private:
  std::string _message;
public:
  _khmer_exception(std::string message) : _message(message) { };
  inline const std::string get_message() const { return _message; };
};

class _khmer_signal : public _khmer_exception {
public:
  _khmer_signal(std::string message) : _khmer_exception(message) { };
};

typedef khmer:: pre_partition_info _pre_partition_info;

// Python exception to raise
static PyObject *KhmerError;

// default callback obj;
static PyObject *_callback_obj = NULL;

// callback function to pass into C++ functions

void _report_fn(const char * info, void * data, unsigned long long n_reads,
		unsigned long long other)
{
  // handle signals etc. (like CTRL-C)
  if (PyErr_CheckSignals() != 0) {
    throw _khmer_signal("PyErr_CheckSignals received a signal");
  }

  // set data to default?
  if (!data && _callback_obj) {
    data = _callback_obj;
  }

  // if 'data' is set, it is None, or a Python callable
  if (data) {
    PyObject * obj = (PyObject *) data;
    if (obj != Py_None) {
      PyObject * args = Py_BuildValue("sLL", info, n_reads, other);
      PyObject * r = PyObject_Call(obj, args, NULL);
      Py_XDECREF(r);
      Py_DECREF(args);
    }
  }

  if (PyErr_Occurred()) {
    throw _khmer_signal("PyErr_Occurred is set");
  }

  // ...allow other Python threads to do stuff...
  Py_BEGIN_ALLOW_THREADS;
  Py_END_ALLOW_THREADS;
}


/***********************************************************************/

//
// Config object -- configuration of khmer internals
//

/*
// For bookkeeping purposes.
static khmer:: Config *	    the_active_config	  = NULL;
*/

typedef struct
{
  PyObject_HEAD
  khmer:: Config *    config;
} khmer_ConfigObject;

static void	  khmer_config_dealloc( PyObject * );
static PyObject * khmer_config_getattr( PyObject * obj, char * name );

static PyTypeObject khmer_ConfigType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "Config", sizeof(khmer_ConfigObject),
    0,
    khmer_config_dealloc,	/*tp_dealloc*/
    0,				/*tp_print*/
    khmer_config_getattr,	/*tp_getattr*/
    0,				/*tp_setattr*/
    0,				/*tp_compare*/
    0,				/*tp_repr*/
    0,				/*tp_as_number*/
    0,				/*tp_as_sequence*/
    0,				/*tp_as_mapping*/
    0,				/*tp_hash */
    0,				/*tp_call*/
    0,				/*tp_str*/
    0,				/*tp_getattro*/
    0,				/*tp_setattro*/
    0,				/*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,		/*tp_flags*/
    "config object",            /* tp_doc */
};

/*
static
PyObject *
new_config( PyObject * self, PyObject * args )
{
  // TODO: Take a dictionary to initialize config values.
  //	   Need khmer:: Config constructor which supports this first.

  khmer_ConfigObject * obj = 
    (khmer_ConfigObject *)PyObject_New(khmer_ConfigObject, &khmer_ConfigType);

  obj->config = new khmer:: Config( );

  return (PyObject *)obj;
}
*/

static
PyObject *
get_config( PyObject * self, PyObject * args )
{
  khmer_ConfigObject *	obj = 
    (khmer_ConfigObject *)PyObject_New(khmer_ConfigObject, &khmer_ConfigType);

  khmer:: Config *	config_new      = &(khmer:: get_active_config( ));
  obj->config	    = config_new;
//  the_active_config = config_new;

  return (PyObject *)obj;
}

/*
static
PyObject *
set_config( PyObject * self, PyObject * args )
{
  khmer_ConfigObject *	  obj	  = NULL;

  if (!PyArg_ParseTuple( args, "O!", &khmer_ConfigType, &obj ))
    return NULL;

  khmer:: Config *	  config = obj->config;
  // TODO? Add sanity check to ensure that 'config' is valid.
  khmer:: set_active_config( *config );
  the_active_config = config;

  Py_INCREF(Py_None);
  return Py_None;
}
*/

static
void
khmer_config_dealloc( PyObject* self )
{
//  khmer_ConfigObject * obj = (khmer_ConfigObject *) self;
//  if (the_active_config != obj->config)
//  {
//    delete obj->config;
//    obj->config = NULL;
//  }
  
  PyObject_Del( self );
}

static
PyObject *
config_has_extra_sanity_checks( PyObject * self, PyObject * args )
{
  khmer_ConfigObject *	  me	    = (khmer_ConfigObject *) self;
  khmer::Config *	  config    = me->config;
  if (config->has_extra_sanity_checks( )) Py_RETURN_TRUE;
  Py_RETURN_FALSE;
}

static
PyObject *
config_get_number_of_threads( PyObject * self, PyObject * args )
{
  khmer_ConfigObject *	  me	    = (khmer_ConfigObject *) self;
  khmer::Config *	  config    = me->config;
  return PyInt_FromSize_t( (size_t)config->get_number_of_threads( ) );
}

static
PyObject *
config_set_number_of_threads( PyObject * self, PyObject * args )
{
  int	  number_of_threads;

  if (!PyArg_ParseTuple( args, "i", &number_of_threads ))
    return NULL;

  khmer_ConfigObject *	  me	    = (khmer_ConfigObject *) self;
  khmer::Config *	  config    = me->config;
  // TODO: Catch exceptions and set errors as appropriate.
  config->set_number_of_threads( number_of_threads );

  Py_INCREF(Py_None);
  return Py_None;
}


static
PyObject *
config_get_reads_input_buffer_size( PyObject * self, PyObject * args )
{
  khmer_ConfigObject *	  me	    = (khmer_ConfigObject *) self;
  khmer::Config *	  config    = me->config;
  // TODO: More safely match type with uint64_t.
  return PyLong_FromUnsignedLongLong( config->get_reads_input_buffer_size( ) );
}


static
PyObject *
config_set_reads_input_buffer_size( PyObject * self, PyObject * args )
{
  unsigned long long reads_input_buffer_size;

  if (!PyArg_ParseTuple( args, "K", &reads_input_buffer_size ))
    return NULL;

  khmer_ConfigObject *	  me	    = (khmer_ConfigObject *) self;
  khmer::Config *	  config    = me->config;
  // TODO: Catch exceptions and set errors as appropriate.
  config->set_reads_input_buffer_size( reads_input_buffer_size );

  Py_INCREF(Py_None);
  return Py_None;
}


static
PyObject *
config_get_input_buffer_trace_level( PyObject * self, PyObject * args )
{
  khmer_ConfigObject *	  me	    = (khmer_ConfigObject *) self;
  khmer::Config *	  config    = me->config;
  return PyInt_FromSize_t( (size_t)config->get_input_buffer_trace_level( ) );
}


static
PyObject *
config_set_input_buffer_trace_level( PyObject * self, PyObject * args )
{
  unsigned char trace_level;

  if (!PyArg_ParseTuple( args, "B", &trace_level )) return NULL;

  khmer_ConfigObject *	  me	    = (khmer_ConfigObject *) self;
  khmer::Config *	  config    = me->config;
  // TODO: Catch exceptions and set errors as appropriate.
  config->set_input_buffer_trace_level( (uint8_t)trace_level );

  Py_INCREF(Py_None);
  return Py_None;
}


static
PyObject *
config_get_reads_parser_trace_level( PyObject * self, PyObject * args )
{
  khmer_ConfigObject *	  me	    = (khmer_ConfigObject *) self;
  khmer::Config *	  config    = me->config;
  return PyInt_FromSize_t( (size_t)config->get_reads_parser_trace_level( ) );
}


static
PyObject *
config_set_reads_parser_trace_level( PyObject * self, PyObject * args )
{
  unsigned char trace_level;

  if (!PyArg_ParseTuple( args, "B", &trace_level )) return NULL;

  khmer_ConfigObject *	  me	    = (khmer_ConfigObject *) self;
  khmer::Config *	  config    = me->config;
  // TODO: Catch exceptions and set errors as appropriate.
  config->set_reads_parser_trace_level( (uint8_t)trace_level );

  Py_INCREF(Py_None);
  return Py_None;
}


static PyMethodDef khmer_config_methods[] = {
  { "has_extra_sanity_checks", config_has_extra_sanity_checks,
    METH_VARARGS, "Compiled with extra sanity checking?" },
  { "get_number_of_threads", config_get_number_of_threads,
    METH_VARARGS, "Get the number of threads to use." },
  { "set_number_of_threads", config_set_number_of_threads,
    METH_VARARGS, "Set the number of threads to use." },
  { "get_reads_input_buffer_size", config_get_reads_input_buffer_size, 
    METH_VARARGS, "Get the buffer size used by the reads file parser." },
  { "set_reads_input_buffer_size", config_set_reads_input_buffer_size,
    METH_VARARGS, "Set the buffer size used by the reads file parser." },
  { "get_input_buffer_trace_level", config_get_input_buffer_trace_level,
    METH_VARARGS, "Get the trace level of the input buffer manager." },
  { "set_input_buffer_trace_level", config_set_input_buffer_trace_level,
    METH_VARARGS, "Set the trace level of the input buffer manager." },
  { "get_reads_parser_trace_level", config_get_reads_parser_trace_level,
    METH_VARARGS, "Get the trace level of the reads file parser." },
  { "set_reads_parser_trace_level", config_set_reads_parser_trace_level,
    METH_VARARGS, "Set the trace level of the reads file parser." },
  {NULL, NULL, 0, NULL}           /* sentinel */
};

static
PyObject *
khmer_config_getattr( PyObject * obj, char * name )
{
  return Py_FindMethod(khmer_config_methods, obj, name);
}

/***********************************************************************/

//
// Read object -- name, sequence, and FASTQ stuff
//

typedef struct
{
  PyObject_HEAD
  khmer:: read_parsers:: Read *  read;
} khmer_ReadObject;


static void	    khmer_read_dealloc( PyObject * obj );
static PyObject *   khmer_read_getattr( PyObject * obj, char * name );

static PyTypeObject khmer_ReadType =
{
    PyObject_HEAD_INIT(NULL)
    0,
    "Read",
    sizeof( khmer_ReadObject ),
    0,
    khmer_read_dealloc,		/*tp_dealloc*/
    0,				/*tp_print*/
    khmer_read_getattr,		/*tp_getattr*/
    0,				/*tp_setattr*/
    0,				/*tp_compare*/
    0,				/*tp_repr*/
    0,				/*tp_as_number*/
    0,				/*tp_as_sequence*/
    0,				/*tp_as_mapping*/
    0,				/*tp_hash */
    0,				/*tp_call*/
    0,				/*tp_str*/
    0,				/*tp_getattro*/
    0,				/*tp_setattro*/
    0,				/*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,		/*tp_flags*/
    "FASTA/FASTQ read object",	/* tp_doc */
};


static
void
khmer_read_dealloc( PyObject * self )
{

  khmer_ReadObject *  obj = (khmer_ReadObject *)self;
  delete obj->read; obj->read = NULL;
  PyObject_Del( self );

}

static
PyObject *
khmer_read_getattr( PyObject * self, char * attr_name )
{
  khmer_ReadObject *		me	      = (khmer_ReadObject *)self;
  khmer:: read_parsers:: Read *	read	      = me->read;
  PyObject *			value_OBJECT  = NULL;

  assert( read );

  if	  (!strcmp( attr_name, "name" ))
    value_OBJECT = PyString_FromString( (read->name).c_str( ) );
  else if (!strcmp( attr_name, "sequence" ))
    value_OBJECT = PyString_FromString( (read->sequence).c_str( ) );
  else if (!strcmp( attr_name, "annotations" ))
    value_OBJECT = PyString_FromString( (read->annotations).c_str( ) );
  else if (!strcmp( attr_name, "accuracy" ))
    value_OBJECT = PyString_FromString( (read->accuracy).c_str( ) );
  // TODO? Handle other fields.
  else
  {
    PyErr_SetString( PyExc_KeyError, "invalid member attribute name" );
    return NULL;
  }

  return value_OBJECT;
}

/***********************************************************************/

//
// ReadParser object -- parse reads directly from streams
//

typedef struct
{
  PyObject_HEAD
  khmer:: read_parsers:: IParser *  parser;
} khmer_ReadParserObject;


static void	    khmer_read_parser_dealloc( PyObject * );
static PyObject *   khmer_read_parser_getattr( PyObject * obj, char * name );
static PyObject *   khmer_read_parser_iter( PyObject * self );
static PyObject *   khmer_read_parser_iternext( PyObject * self );

static PyTypeObject khmer_ReadParserType =
{
    PyObject_HEAD_INIT(NULL)
    0,
    "ReadParser",
    sizeof( khmer_ReadParserObject ),
    0,
    khmer_read_parser_dealloc,	/*tp_dealloc*/
    0,				/*tp_print*/
    khmer_read_parser_getattr,	/*tp_getattr*/
    0,				/*tp_setattr*/
    0,				/*tp_compare*/
    0,				/*tp_repr*/
    0,				/*tp_as_number*/
    0,				/*tp_as_sequence*/
    0,				/*tp_as_mapping*/
    0,				/*tp_hash */
    0,				/*tp_call*/
    0,				/*tp_str*/
    0,				/*tp_getattro*/
    0,				/*tp_setattro*/
    0,				/*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,		/*tp_flags*/
    "read parser object",	/*tp_doc*/
    0,				/*tp_traverse*/
    0,				/*tp_clear*/
    0,				/*tp_richcompare*/
    0,				/*tp_weaklistoffset*/
    khmer_read_parser_iter,	/*tp_iter*/
    khmer_read_parser_iternext,	/*tp_iternext*/
};


static
PyObject *
new_read_parser( PyObject * self, PyObject * args )
{
  char *      ifile_name_CSTR;
  khmer:: Config  &the_config	  = khmer:: get_active_config( );
  uint32_t    number_of_threads	  = the_config.get_number_of_threads( );
  uint64_t    cache_size	  = the_config.get_reads_input_buffer_size( );
  uint8_t     trace_level	  = the_config.get_reads_parser_trace_level( );

  if (!PyArg_ParseTuple(
	args, "s|IKH",
	&ifile_name_CSTR, &number_of_threads, &cache_size, &trace_level
      )) return NULL;
  std:: string	ifile_name( ifile_name_CSTR );

  khmer_ReadParserObject * obj = 
  (khmer_ReadParserObject *)PyObject_New(
    khmer_ReadParserObject, &khmer_ReadParserType
  );

  try
  {
    obj->parser = 
    khmer:: read_parsers:: IParser:: get_parser(
      ifile_name, number_of_threads, cache_size, trace_level
    );
  }
  catch (khmer:: InvalidStreamHandle &exc)
  {
    PyErr_SetString( PyExc_ValueError, "invalid input file name" );
    return NULL;
  }

  return (PyObject *)obj;
}


static
void
khmer_read_parser_dealloc( PyObject * self )
{

  khmer_ReadParserObject *  obj = (khmer_ReadParserObject *)self;
  delete obj->parser; obj->parser = NULL;
  PyObject_Del( self );

}


static
PyObject *
khmer_read_parser_iter( PyObject * self )
{
  Py_INCREF( self );
  return self;
}


static
PyObject *
khmer_read_parser_iternext( PyObject * self )
{
  khmer_ReadParserObject *	    myself  = (khmer_ReadParserObject *) self;
  khmer:: read_parsers:: IParser *  parser  = myself->parser;

  bool				    stop_iteration	= false;
  bool				    invalid_file_format	= false;
  khmer:: read_parsers:: Read *	    the_read		= 
  new khmer:: read_parsers:: Read( );

  Py_BEGIN_ALLOW_THREADS
  stop_iteration = parser->is_complete( );
  if (!stop_iteration)
    try
    {
      // TODO: Assign a pointer rather than copy by value.
      *the_read = parser->get_next_read( );
    }
    catch (khmer:: read_parsers:: InvalidFASTAFileFormat &exc)
    {
      invalid_file_format = true;
    }
  Py_END_ALLOW_THREADS

  // Note: Can simply return NULL instead of setting the StopIteration 
  //	   exception.
  if (stop_iteration)
    return NULL;

  if (invalid_file_format)
  {
    PyErr_SetString( PyExc_ValueError, "invalid FASTA file" );
    return NULL;
  }

  khmer_ReadObject *		    the_read_PYTHON = 
  (khmer_ReadObject *)PyObject_New( khmer_ReadObject, &khmer_ReadType );
  the_read_PYTHON->read = the_read;
  return (PyObject *)the_read_PYTHON;
}


static PyMethodDef khmer_read_parser_methods[ ] =
{
  { NULL,	      NULL,
      0,	      NULL }  /* sentinel */
};


static
PyObject *
khmer_read_parser_getattr( PyObject * obj, char * name )
{ return Py_FindMethod(khmer_read_parser_methods, obj, name); }


static
khmer:: read_parsers:: IParser *
_PyObject_to_khmer_read_parser( PyObject * py_object )
{
  // TODO: Add type-checking.

  return ((khmer_ReadParserObject *)py_object)->parser;
}


/***********************************************************************/

//
// KTable object -- exact counting of k-mers.
//

typedef struct {
  PyObject_HEAD
  khmer::KTable * ktable;
} khmer_KTableObject;

static void khmer_ktable_dealloc(PyObject *);

//
// ktable_forward_hash - hash k-mers into numbers
//

static PyObject * ktable_forward_hash(PyObject * self, PyObject * args)
{
  khmer_KTableObject * me = (khmer_KTableObject *) self;
  khmer::KTable * ktable = me->ktable;

  char * kmer;

  if (!PyArg_ParseTuple(args, "s", &kmer)) {
    return NULL;
  }

  if (strlen(kmer) != ktable->ksize()) {
    PyErr_SetString(PyExc_ValueError,
		    "k-mer length must be the same as the hashtable k-size");
    return NULL;
  }

  return PyLong_FromUnsignedLongLong(khmer::_hash(kmer, ktable->ksize()));
}

//
// ktable_forward_hash_no_rc -- hash k-mers into numbers, with no RC handling
//

static PyObject * ktable_forward_hash_no_rc(PyObject * self, PyObject * args)
{
  khmer_KTableObject * me = (khmer_KTableObject *) self;
  khmer::KTable * ktable = me->ktable;

  char * kmer;

  if (!PyArg_ParseTuple(args, "s", &kmer)) {
    return NULL;
  }

  if (strlen(kmer) != ktable->ksize()) {
    PyErr_SetString(PyExc_ValueError,
		    "k-mer length must be the same as the hashtable k-size");
    return NULL;
  }

  return PyLong_FromUnsignedLongLong(khmer::_hash_forward(kmer, ktable->ksize()));
}

//
// ktable_reverse_hash -- reverse-hash numbers into DNA strings
//

static PyObject * ktable_reverse_hash(PyObject * self, PyObject * args)
{
  khmer_KTableObject * me = (khmer_KTableObject *) self;
  khmer::KTable * ktable = me->ktable;

  unsigned int val;
  
  if (!PyArg_ParseTuple(args, "I", &val)) {
    return NULL;
  }

  return PyString_FromString(khmer::_revhash(val, ktable->ksize()).c_str());
}

//
// ktable_count -- count the given k-mer in the ktable
//

static PyObject * ktable_count(PyObject * self, PyObject * args)
{
  khmer_KTableObject * me = (khmer_KTableObject *) self;
  khmer::KTable * ktable = me->ktable;

  char * kmer;

  if (!PyArg_ParseTuple(args, "s", &kmer)) {
    return NULL;
  }

  if (strlen(kmer) != ktable->ksize()) {
    PyErr_SetString(PyExc_ValueError,
		    "k-mer length must be the same as the hashtable k-size");
    return NULL;
  }

  ktable->count(kmer);

  return PyInt_FromLong(1);
}

//
// ktable_consume -- count all of the k-mers in the given string
//

static PyObject * ktable_consume(PyObject * self, PyObject * args)
{
  khmer_KTableObject * me = (khmer_KTableObject *) self;
  khmer::KTable * ktable = me->ktable;

  char * long_str;

  if (!PyArg_ParseTuple(args, "s", &long_str)) {
    return NULL;
  }

  if (strlen(long_str) < ktable->ksize()) {
    PyErr_SetString(PyExc_ValueError,
		    "string length must >= the hashtable k-mer size");
    return NULL;
  }

  ktable->consume_string(long_str);

  unsigned int n_consumed = strlen(long_str) - ktable->ksize() + 1;

  return PyInt_FromLong(n_consumed);
}

static PyObject * ktable_get(PyObject * self, PyObject * args)
{
  khmer_KTableObject * me = (khmer_KTableObject *) self;
  khmer::KTable * ktable = me->ktable;

  PyObject * arg;

  if (!PyArg_ParseTuple(args, "O", &arg)) {
    return NULL;
  }

  unsigned long count = 0;

  if (PyInt_Check(arg)) {
    long pos = PyInt_AsLong(arg);
    count = ktable->get_count((unsigned int) pos);
  } else if (PyString_Check(arg)) {
    std::string s = PyString_AsString(arg);
    count = ktable->get_count(s.c_str());
  }

  return PyInt_FromLong(count);
}

static PyObject * ktable__getitem__(PyObject * self, Py_ssize_t index)
{
  khmer_KTableObject * me = (khmer_KTableObject *) self;
  khmer::KTable * ktable = me->ktable;

  khmer::ExactCounterType count = ktable->get_count(index);

  return PyInt_FromLong(count);
}

static int ktable__contains__( PyObject * self, PyObject * val )
{
  khmer_KTableObject * me = (khmer_KTableObject *) self;
  khmer::KTable * ktable = me->ktable;
  // TODO: Consider other object types.
  char * kmer_str = PyString_AsString( val );

  if (kmer_str) return (int)(bool)ktable->get_count( kmer_str );
  return -1;
}

static PyObject * ktable_set(PyObject * self, PyObject * args)
{
  khmer_KTableObject * me = (khmer_KTableObject *) self;
  khmer::KTable * ktable = me->ktable;

  PyObject * arg;
  khmer::ExactCounterType count;

  if (!PyArg_ParseTuple(args, "OK", &arg, &count)) {
    return NULL;
  }

  if (PyInt_Check(arg)) {
    long pos = PyInt_AsLong(arg);
    ktable->set_count((unsigned int) pos, count);
  } else if (PyString_Check(arg)) {
    std::string s = PyString_AsString(arg);
    ktable->set_count(s.c_str(), count);
  }
  
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject * ktable_max_hash(PyObject * self, PyObject * args)
{
  khmer_KTableObject * me = (khmer_KTableObject *) self;
  khmer::KTable * ktable = me->ktable;

  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  return PyInt_FromLong(ktable->max_hash());
}

static PyObject * ktable_n_entries(PyObject * self, PyObject * args)
{
  khmer_KTableObject * me = (khmer_KTableObject *) self;
  khmer::KTable * ktable = me->ktable;

  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  return PyInt_FromLong(ktable->n_entries());
}

Py_ssize_t ktable__len__(PyObject * self)
{
  khmer_KTableObject * me = (khmer_KTableObject *) self;
  khmer::KTable * ktable = me->ktable;

  return ktable->n_entries();
}

static PyObject * ktable_ksize(PyObject * self, PyObject * args)
{
  khmer_KTableObject * me = (khmer_KTableObject *) self;
  khmer::KTable * ktable = me->ktable;

  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  return PyInt_FromLong(ktable->ksize());
}

static PyObject * ktable_clear(PyObject * self, PyObject * args)
{
  khmer_KTableObject * me = (khmer_KTableObject *) self;
  khmer::KTable * ktable = me->ktable;

  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  ktable->clear();

  Py_INCREF(Py_None);
  return Py_None;
}


// fwd decl --> defined below
static PyObject * ktable_update(PyObject * self, PyObject * args);
static PyObject * ktable_intersect(PyObject * self, PyObject * args);

static PyMethodDef khmer_ktable_methods[] = {
  { "forward_hash", ktable_forward_hash, METH_VARARGS, "Convert string to int" },
  { "forward_hash_no_rc", ktable_forward_hash_no_rc, METH_VARARGS, "Convert string to int, with no reverse complement handling" },
  { "reverse_hash", ktable_reverse_hash, METH_VARARGS, "Convert int to string" },
  { "count", ktable_count, METH_VARARGS, "Count the given kmer" },
  { "consume", ktable_consume, METH_VARARGS, "Count all k-mers in the given string" },
  { "get", ktable_get, METH_VARARGS, "Get the count for the given k-mer" },
  { "max_hash", ktable_max_hash, METH_VARARGS, "Get the maximum hash value"},
  { "n_entries", ktable_n_entries, METH_VARARGS, "Get the number of possible entries"},
  { "ksize", ktable_ksize, METH_VARARGS, "Get k"},
  { "set", ktable_set, METH_VARARGS, "Set counter to a value"},
  { "update", ktable_update, METH_VARARGS, "Combine another ktable with this one"},
  { "intersect", ktable_intersect, METH_VARARGS,
    "Create another ktable containing the intersection of two ktables:"
    "where both ktables have an entry, the counts will be summed."},
  { "clear", ktable_clear, METH_VARARGS, "Set all entries to 0." },

  {NULL, NULL, 0, NULL}           /* sentinel */
};

static PyObject *
khmer_ktable_getattr(PyObject * obj, char * name)
{
  return Py_FindMethod(khmer_ktable_methods, obj, name);
}

#define is_ktable_obj(v)  ((v)->ob_type == &khmer_KTableType)

static PySequenceMethods khmer_KTable_SequenceMethods = {
  ktable__len__,
  0,
  0,
  ktable__getitem__,
  0,
  0,
  0,
  ktable__contains__,
  0,
  0
};

static PyTypeObject khmer_KTableType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "KTable", sizeof(khmer_KTableObject),
    0,
    khmer_ktable_dealloc,	/*tp_dealloc*/
    0,				/*tp_print*/
    khmer_ktable_getattr,	/*tp_getattr*/
    0,				/*tp_setattr*/
    0,				/*tp_compare*/
    0,				/*tp_repr*/
    0,				/*tp_as_number*/
    &khmer_KTable_SequenceMethods, /*tp_as_sequence*/
    0,				/*tp_as_mapping*/
    0,				/*tp_hash */
    0,				/*tp_call*/
    0,				/*tp_str*/
    0,				/*tp_getattro*/
    0,				/*tp_setattro*/
    0,				/*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,		/*tp_flags*/
    "ktable object",           /* tp_doc */
};

//
// new_ktable
//

static PyObject* new_ktable(PyObject * self, PyObject * args)
{
  unsigned int size = 0;

  if (!PyArg_ParseTuple(args, "I", &size)) {
    return NULL;
  }

  khmer_KTableObject * ktable_obj = (khmer_KTableObject *) \
    PyObject_New(khmer_KTableObject, &khmer_KTableType);

  ktable_obj->ktable = new khmer::KTable(size);

  return (PyObject *) ktable_obj;
}

//
// khmer_ktable_dealloc -- clean up a table object.
//

static void khmer_ktable_dealloc(PyObject* self)
{
  khmer_KTableObject * obj = (khmer_KTableObject *) self;
  delete obj->ktable;
  obj->ktable = NULL;
  
  PyObject_Del((PyObject *) obj);
}

static PyObject * ktable_update(PyObject * self, PyObject * args)
{
  khmer_KTableObject * me = (khmer_KTableObject *) self;
  khmer::KTable * ktable = me->ktable;

  PyObject * other_o;

  PyArg_ParseTuple(args, "O", &other_o);

  assert(is_ktable_obj(other_o));

  khmer::KTable * other = ((khmer_KTableObject*) other_o)->ktable;

  ktable->update(*other);

  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject * ktable_intersect(PyObject * self, PyObject * args)
{
  khmer_KTableObject * me = (khmer_KTableObject *) self;
  khmer::KTable * ktable = me->ktable;

  PyObject * other_o;

  PyArg_ParseTuple(args, "O", &other_o);

  assert(is_ktable_obj(other_o));

  khmer::KTable * other = ((khmer_KTableObject*) other_o)->ktable;

  khmer::KTable * intersection = ktable->intersect(*other);

  khmer_KTableObject * ktable_obj = (khmer_KTableObject *) \
    PyObject_New(khmer_KTableObject, &khmer_KTableType);

  ktable_obj->ktable = intersection;

  return (PyObject *) ktable_obj;
}

/***********************************************************************/

//
// KCountingHash object
//

typedef struct {
  PyObject_HEAD
  khmer::CountingHash * counting;
} khmer_KCountingHashObject;

typedef struct {
  PyObject_HEAD
  khmer::Hashbits * hashbits;
} khmer_KHashbitsObject;

static void khmer_hashbits_dealloc(PyObject *);
static PyObject * khmer_hashbits_getattr(PyObject * obj, char * name);

static PyTypeObject khmer_KHashbitsType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "KHashbits", sizeof(khmer_KHashbitsObject),
    0,
    khmer_hashbits_dealloc,	/*tp_dealloc*/
    0,				/*tp_print*/
    khmer_hashbits_getattr,	/*tp_getattr*/
    0,				/*tp_setattr*/
    0,				/*tp_compare*/
    0,				/*tp_repr*/
    0,				/*tp_as_number*/
    0,				/*tp_as_sequence*/
    0,				/*tp_as_mapping*/
    0,				/*tp_hash */
    0,				/*tp_call*/
    0,				/*tp_str*/
    0,				/*tp_getattro*/
    0,				/*tp_setattro*/
    0,				/*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,		/*tp_flags*/
    "hashbits object",           /* tp_doc */
};

#define is_hashbits_obj(v)  ((v)->ob_type == &khmer_KHashbitsType)

typedef struct {
  PyObject_HEAD
  khmer::MinMaxTable * mmt;
} khmer_MinMaxObject;


#define is_minmax_obj(v)  ((v)->ob_type == &khmer_MinMaxType)

static void khmer_minmax_dealloc(PyObject* self);
static PyObject * khmer_minmax_getattr(PyObject *, char *);

static PyTypeObject khmer_MinMaxType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "MinMax", sizeof(khmer_MinMaxObject),
    0,
    khmer_minmax_dealloc,	/*tp_dealloc*/
    0,				/*tp_print*/
    khmer_minmax_getattr,	/*tp_getattr*/
    0,				/*tp_setattr*/
    0,				/*tp_compare*/
    0,				/*tp_repr*/
    0,				/*tp_as_number*/
    0,				/*tp_as_sequence*/
    0,				/*tp_as_mapping*/
    0,				/*tp_hash */
    0,				/*tp_call*/
    0,				/*tp_str*/
    0,				/*tp_getattro*/
    0,				/*tp_setattro*/
    0,				/*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,		/*tp_flags*/
    "minmax object",           /* tp_doc */
};



static void khmer_counting_dealloc(PyObject *);

static PyObject * hash_set_use_bigcount(PyObject * self, PyObject * args)
{
  khmer_KCountingHashObject * me = (khmer_KCountingHashObject *) self;
  khmer::CountingHash * counting = me->counting;

  PyObject * x;
  if (!PyArg_ParseTuple(args, "O", &x)) {
    return NULL;
  }

  bool setme = PyObject_IsTrue(x);
  counting->set_use_bigcount(setme);

  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject * hash_get_use_bigcount(PyObject * self, PyObject * args)
{
  khmer_KCountingHashObject * me = (khmer_KCountingHashObject *) self;
  khmer::CountingHash * counting = me->counting;

  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  bool val = counting->get_use_bigcount();

  return PyBool_FromLong((int)val);
}

static PyObject * hash_n_occupied(PyObject * self, PyObject * args)
{
  khmer_KCountingHashObject * me = (khmer_KCountingHashObject *) self;
  khmer::CountingHash * counting = me->counting;

  khmer::HashIntoType start = 0, stop = 0;

  if (!PyArg_ParseTuple(args, "|LL", &start, &stop)) {
    return NULL;
  }

  khmer::HashIntoType n = counting->n_occupied(start, stop);

  return PyInt_FromLong(n);
}

static PyObject * hash_n_entries(PyObject * self, PyObject * args)
{
  khmer_KCountingHashObject * me = (khmer_KCountingHashObject *) self;
  khmer::CountingHash * counting = me->counting;

  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  return PyInt_FromLong(counting->n_entries());
}

static PyObject * hash_count(PyObject * self, PyObject * args)
{
  khmer_KCountingHashObject * me = (khmer_KCountingHashObject *) self;
  khmer::CountingHash * counting = me->counting;

  char * kmer;

  if (!PyArg_ParseTuple(args, "s", &kmer)) {
    return NULL;
  }

  if (strlen(kmer) != counting->ksize()) {
    PyErr_SetString(PyExc_ValueError,
		    "k-mer length must be the same as the hashtable k-size");
    return NULL;
  }

  counting->count(kmer);

  return PyInt_FromLong(1);
}

static PyObject * hash_output_fasta_kmer_pos_freq(PyObject * self, PyObject *args)
{
  khmer_KCountingHashObject * me = (khmer_KCountingHashObject *) self;
  khmer::CountingHash * counting = me->counting;

  char * infile;
  char * outfile;

  if (!PyArg_ParseTuple(args, "ss", &infile, &outfile)) {
    return NULL;
  }

  counting->output_fasta_kmer_pos_freq(infile, outfile);

  return PyInt_FromLong(0);
}

static PyObject * hash_fasta_file_to_minmax(PyObject * self, PyObject *args)
{
  khmer_KCountingHashObject * me = (khmer_KCountingHashObject *) self;
  khmer::CountingHash * counting = me->counting;

  char * filename;
  unsigned int total_reads;
  PyObject * callback_obj = NULL;

  if (!PyArg_ParseTuple(args, "si|O", &filename, &total_reads,
			&callback_obj)) {
    return NULL;
  }

  khmer::MinMaxTable * mmt;
  try {
    mmt = counting->fasta_file_to_minmax(filename, total_reads, 
					  _report_fn, callback_obj);
  } catch (_khmer_signal &e) {
    return NULL;
  }
  
  khmer_MinMaxObject * minmax_obj = (khmer_MinMaxObject *) \
    PyObject_New(khmer_MinMaxObject, &khmer_MinMaxType);

  minmax_obj->mmt = mmt;

  return (PyObject *) minmax_obj;
}

static PyObject * hash_consume_fasta(PyObject * self, PyObject * args)
{
  khmer_KCountingHashObject * me  = (khmer_KCountingHashObject *) self;
  khmer::CountingHash * counting  = me->counting;

  char * filename;
  khmer::HashIntoType lower_bound = 0, upper_bound = 0;
  PyObject * callback_obj = NULL;

  if (!PyArg_ParseTuple(
    args, "s|iiO", &filename, &lower_bound, &upper_bound, &callback_obj
  )) {
      return NULL;
  }

  // call the C++ function, and trap signals => Python
  unsigned long long  n_consumed    = 0;
  unsigned int	      total_reads   = 0;
  try {
    counting->consume_fasta(filename, total_reads, n_consumed,
			     lower_bound, upper_bound, 
			     _report_fn, callback_obj);
  } catch (_khmer_signal &e) {
    return NULL;
  }

  return Py_BuildValue("iL", total_reads, n_consumed);
}

static PyObject * hash_consume_fasta_with_reads_parser(
  PyObject * self, PyObject * args
)
{
  khmer_KCountingHashObject * me  = (khmer_KCountingHashObject *) self;
  khmer::CountingHash * counting  = me->counting;

  PyObject * rparser_obj = NULL;
  khmer::HashIntoType lower_bound = 0, upper_bound = 0;
  PyObject * callback_obj = NULL;

  if (!PyArg_ParseTuple(
    args, "O|iiO", &rparser_obj, &lower_bound, &upper_bound, &callback_obj
  )) {
      return NULL;
  }

  khmer:: read_parsers::IParser * rparser = 
  _PyObject_to_khmer_read_parser( rparser_obj );

  // call the C++ function, and trap signals => Python
  unsigned long long  n_consumed  = 0;
  unsigned int	      total_reads = 0;
  bool		      exc_raised  = false;
  Py_BEGIN_ALLOW_THREADS
  try {
    counting->consume_fasta(rparser, total_reads, n_consumed,
			     lower_bound, upper_bound, 
			     _report_fn, callback_obj);
  } catch (_khmer_signal &e) {
    exc_raised = true;
  }
  Py_END_ALLOW_THREADS
  if (exc_raised) return NULL;

  return Py_BuildValue("iL", total_reads, n_consumed);
}

static PyObject * hash_consume(PyObject * self, PyObject * args)
{
  khmer_KCountingHashObject * me = (khmer_KCountingHashObject *) self;
  khmer::CountingHash * counting = me->counting;

  char * long_str;
  khmer::HashIntoType lower_bound = 0, upper_bound = 0;

  if (!PyArg_ParseTuple(args, "s|ll", &long_str, &lower_bound, &upper_bound)) {
    return NULL;
  }
  
  if (strlen(long_str) < counting->ksize()) {
    PyErr_SetString(PyExc_ValueError,
		    "string length must >= the hashtable k-mer size");
    return NULL;
  }

  unsigned int n_consumed;
  n_consumed = counting->consume_string(long_str, lower_bound, upper_bound);

  return PyInt_FromLong(n_consumed);
}

static PyObject * hash_get_min_count(PyObject * self, PyObject * args)
{
  khmer_KCountingHashObject * me = (khmer_KCountingHashObject *) self;
  khmer::CountingHash * counting = me->counting;
  khmer::HashIntoType lower_bound = 0, upper_bound = 0;

  char * long_str;

  if (!PyArg_ParseTuple(args, "s|ll", &long_str, &lower_bound, &upper_bound)) {
    return NULL;
  }

  if (strlen(long_str) < counting->ksize()) {
    PyErr_SetString(PyExc_ValueError,
		    "string length must >= the hashtable k-mer size");
    return NULL;
  }

  khmer::BoundedCounterType c = counting->get_min_count(long_str,
							 lower_bound,
							 upper_bound);
  unsigned int N = c;

  return PyInt_FromLong(N);
}

static PyObject * hash_get_max_count(PyObject * self, PyObject * args)
{
  khmer_KCountingHashObject * me = (khmer_KCountingHashObject *) self;
  khmer::CountingHash * counting = me->counting;
  khmer::HashIntoType lower_bound = 0, upper_bound = 0;

  char * long_str;

  if (!PyArg_ParseTuple(args, "s|ll", &long_str, &lower_bound, &upper_bound)) {
    return NULL;
  }

  if (strlen(long_str) < counting->ksize()) {
    PyErr_SetString(PyExc_ValueError,
		    "string length must >= the hashtable k-mer size");
    return NULL;
  }

  khmer::BoundedCounterType c = counting->get_max_count(long_str,
							 lower_bound,
							 upper_bound);
  unsigned int N = c;

  return PyInt_FromLong(N);
}

static PyObject * hash_get_median_count(PyObject * self, PyObject * args)
{
  khmer_KCountingHashObject * me = (khmer_KCountingHashObject *) self;
  khmer::CountingHash * counting = me->counting;

  char * long_str;

  if (!PyArg_ParseTuple(args, "s", &long_str)) {
    return NULL;
  }

  if (strlen(long_str) < counting->ksize()) {
    PyErr_SetString(PyExc_ValueError,
		    "string length must >= the hashtable k-mer size");
    return NULL;
  }

  khmer::BoundedCounterType med = 0;
  float average = 0, stddev = 0;

  counting->get_median_count(long_str, med, average, stddev);

  return Py_BuildValue("iff", med, average, stddev);
}

static PyObject * hash_get_kadian_count(PyObject * self, PyObject * args)
{
  khmer_KCountingHashObject * me = (khmer_KCountingHashObject *) self;
  khmer::CountingHash * counting = me->counting;

  char * long_str;
  unsigned int nk = 1;

  if (!PyArg_ParseTuple(args, "s|I", &long_str, &nk)) {
    return NULL;
  }

  if (strlen(long_str) < counting->ksize()) {
    PyErr_SetString(PyExc_ValueError,
		    "string length must >= the hashtable k-mer size");
    return NULL;
  }

  khmer::BoundedCounterType kad = 0;

  counting->get_kadian_count(long_str, kad, nk);

  return Py_BuildValue("i", kad);
}

static PyObject * hash_get_kmer_abund_mean(PyObject * self, PyObject * args)
{
  khmer_KCountingHashObject * me = (khmer_KCountingHashObject *) self;
  khmer::CountingHash * counting = me->counting;

  char * filename = NULL;

  if (!PyArg_ParseTuple(args, "s", &filename)) {
    return NULL;
  }

  unsigned long long total = 0;
  unsigned long long count = 0;
  float mean = 0.0;
  counting->get_kmer_abund_mean(filename, total, count, mean);

  return Py_BuildValue("LLf", total, count, mean);
}

static PyObject * hash_get_kmer_abund_abs_deviation(PyObject * self, PyObject * args)
{
  khmer_KCountingHashObject * me = (khmer_KCountingHashObject *) self;
  khmer::CountingHash * counting = me->counting;

  char * filename = NULL;
  float mean = 0.0;

  if (!PyArg_ParseTuple(args, "sf", &filename, &mean)) {
    return NULL;
  }

  float abs_dev = 0.0;
  counting->get_kmer_abund_abs_deviation(filename, mean, abs_dev);

  return Py_BuildValue("f", abs_dev);
}

static PyObject * hash_get(PyObject * self, PyObject * args)
{
  khmer_KCountingHashObject * me = (khmer_KCountingHashObject *) self;
  khmer::CountingHash * counting = me->counting;

  PyObject * arg;

  if (!PyArg_ParseTuple(args, "O", &arg)) {
    return NULL;
  }

  unsigned long count = 0;

  if (PyInt_Check(arg)) {
    long pos = PyInt_AsLong(arg);
    count = counting->get_count((unsigned int) pos);
  } else if (PyString_Check(arg)) {
    std::string s = PyString_AsString(arg);
    count = counting->get_count(s.c_str());
  }

  return PyInt_FromLong(count);
}

static PyObject * hash_max_hamming1_count(PyObject * self, PyObject * args)
{
  khmer_KCountingHashObject * me = (khmer_KCountingHashObject *) self;
  khmer::CountingHash * counting = me->counting;

  char * kmer;

  if (!PyArg_ParseTuple(args, "s", &kmer)) {
    return NULL;
  }

  unsigned long count = counting->max_hamming1_count(kmer);

  return PyInt_FromLong(count);
}

static PyObject * count_trim_on_abundance(PyObject * self, PyObject * args)
{
  khmer_KCountingHashObject * me = (khmer_KCountingHashObject *) self;
  khmer::CountingHash * counting = me->counting;

  char * seq = NULL;
  unsigned int min_count_i = 0;

  if (!PyArg_ParseTuple(args, "sI", &seq, &min_count_i)) {
    return NULL;
  }

  unsigned int trim_at;
  Py_BEGIN_ALLOW_THREADS

    khmer::BoundedCounterType min_count = min_count_i;

    trim_at = counting->trim_on_abundance(seq, min_count);

  Py_END_ALLOW_THREADS;

  PyObject * trim_seq = PyString_FromStringAndSize(seq, trim_at);
  PyObject * ret = Py_BuildValue("Oi", trim_seq, trim_at);
  Py_DECREF(trim_seq);

  return ret;
}
static PyObject * count_trim_below_abundance(PyObject * self, PyObject * args)
{
  khmer_KCountingHashObject * me = (khmer_KCountingHashObject *) self;
  khmer::CountingHash * counting = me->counting;

  char * seq = NULL;
  unsigned int max_count_i = 0;

  if (!PyArg_ParseTuple(args, "sI", &seq, &max_count_i)) {
    return NULL;
  }

  unsigned int trim_at;
  Py_BEGIN_ALLOW_THREADS

    khmer::BoundedCounterType max_count = max_count_i;

    trim_at = counting->trim_below_abundance(seq, max_count);

  Py_END_ALLOW_THREADS;

  PyObject * trim_seq = PyString_FromStringAndSize(seq, trim_at);
  PyObject * ret = Py_BuildValue("Oi", trim_seq, trim_at);
  Py_DECREF(trim_seq);

  return ret;
}

static PyObject * hash_abundance_distribution(PyObject * self, PyObject * args)
{
  khmer_KCountingHashObject * me = (khmer_KCountingHashObject *) self;
  khmer::CountingHash * counting = me->counting;

  char * filename = NULL;
  PyObject * tracking_obj = NULL;
  PyObject * callback_obj = NULL;
  if (!PyArg_ParseTuple(args, "sO|O", &filename, &tracking_obj,
			&callback_obj)) {
    return NULL;
  }

  assert(is_hashbits_obj(tracking_obj));

  khmer_KHashbitsObject * tracking_o = (khmer_KHashbitsObject *) tracking_obj;
  khmer::Hashbits * hashbits = tracking_o->hashbits;


  khmer::HashIntoType * dist;
  dist = counting->abundance_distribution(filename, hashbits,
					  _report_fn, callback_obj);
  
  PyObject * x = PyList_New(MAX_BIGCOUNT + 1);
  for (int i = 0; i < MAX_BIGCOUNT + 1; i++) {
    PyList_SET_ITEM(x, i, PyInt_FromLong(dist[i]));
  }

  delete dist;

  return x;
}

static PyObject * hash_fasta_count_kmers_by_position(PyObject * self, PyObject * args)
{
  khmer_KCountingHashObject * me = (khmer_KCountingHashObject *) self;
  khmer::CountingHash * counting = me->counting;

  char * inputfile;
  int max_read_len;
  int limit_by = 0;
  PyObject * callback_obj = NULL;

  if (!PyArg_ParseTuple(args, "sii|O", &inputfile, &max_read_len, &limit_by,
			&callback_obj)) {
    return NULL;
  }

  unsigned long long * counts;
  counts = counting->fasta_count_kmers_by_position(inputfile, max_read_len,
						    limit_by, 
						    _report_fn, callback_obj);
					 
  PyObject * x = PyList_New(max_read_len);
  for (int i = 0; i < max_read_len; i++) {
    PyList_SET_ITEM(x, i, PyInt_FromLong(counts[i]));
  }

  delete counts;

  return x;
}

static PyObject * hash_fasta_dump_kmers_by_abundance(PyObject * self, PyObject * args)
{
  khmer_KCountingHashObject * me = (khmer_KCountingHashObject *) self;
  khmer::CountingHash * counting = me->counting;

  char * inputfile;
  int limit_by = 0;
  PyObject * callback_obj = NULL;

  if (!PyArg_ParseTuple(args, "si|O", &inputfile, &limit_by,
			&callback_obj)) {
    return NULL;
  }

  counting->fasta_dump_kmers_by_abundance(inputfile,
					   limit_by,
					   _report_fn, callback_obj);
					 

  Py_INCREF(Py_None);
  return Py_None;
}

// callback function to pass into dump function

void _dump_report_fn(const char * info, unsigned int count, void * data)
{
  // handle signals etc. (like CTRL-C)
  if (PyErr_CheckSignals() != 0) {
    throw _khmer_signal("PyErr_CheckSignals received a signal");
  }

  // if 'data' is set, it is a Python callable
  if (data) {
    PyObject * obj = (PyObject *) data;
    if (obj != Py_None) {
      PyObject * args = Py_BuildValue("si", info, count);

      PyObject * r = PyObject_Call(obj, args, NULL);
      Py_XDECREF(r);
      Py_DECREF(args);
    }
  }

  if (PyErr_Occurred()) {
    throw _khmer_signal("PyErr_Occurred is set");
  }

  // ...allow other Python threads to do stuff...
  Py_BEGIN_ALLOW_THREADS;
  Py_END_ALLOW_THREADS;
}


static PyObject * hash_load(PyObject * self, PyObject * args)
{
  khmer_KCountingHashObject * me = (khmer_KCountingHashObject *) self;
  khmer::CountingHash * counting = me->counting;

  char * filename = NULL;

  if (!PyArg_ParseTuple(args, "s", &filename)) {
    return NULL;
  }

  counting->load(filename);

  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject * hash_save(PyObject * self, PyObject * args)
{
  khmer_KCountingHashObject * me = (khmer_KCountingHashObject *) self;
  khmer::CountingHash * counting = me->counting;

  char * filename = NULL;

  if (!PyArg_ParseTuple(args, "s", &filename)) {
    return NULL;
  }

  counting->save(filename);

  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject * hash_get_ksize(PyObject * self, PyObject * args)
{
  khmer_KCountingHashObject * me = (khmer_KCountingHashObject *) self;
  khmer::CountingHash * counting = me->counting;

  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  unsigned int k = counting->ksize();

  return PyInt_FromLong(k);
}

static PyObject * hash_get_hashsizes(PyObject * self, PyObject * args)
{
  khmer_KCountingHashObject * me = (khmer_KCountingHashObject *) self;
  khmer::CountingHash * counting = me->counting;


  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  std::vector<khmer::HashIntoType> ts = counting->get_tablesizes();

  PyObject * x = PyList_New(ts.size());
  for (unsigned int i = 0; i < ts.size(); i++) {
    PyList_SET_ITEM(x, i, PyInt_FromLong(ts[i]));
  }

  return x;
}

static PyObject * hash_collect_high_abundance_kmers(PyObject * self,
						    PyObject * args);

static PyMethodDef khmer_counting_methods[] = {
  { "ksize", hash_get_ksize, METH_VARARGS, "" },
  { "hashsizes", hash_get_hashsizes, METH_VARARGS, "" },
  { "set_use_bigcount", hash_set_use_bigcount, METH_VARARGS, "" },
  { "get_use_bigcount", hash_get_use_bigcount, METH_VARARGS, "" },
  { "n_occupied", hash_n_occupied, METH_VARARGS, "Count the number of occupied bins" },
  { "n_entries", hash_n_entries, METH_VARARGS, "" },
  { "count", hash_count, METH_VARARGS, "Count the given kmer" },
  { "consume", hash_consume, METH_VARARGS, "Count all k-mers in the given string" },
  { "consume_fasta", hash_consume_fasta, METH_VARARGS, "Count all k-mers in a given file" },
  { "consume_fasta_with_reads_parser", hash_consume_fasta_with_reads_parser, 
    METH_VARARGS, "Count all k-mers in a given file" },
  { "fasta_file_to_minmax", hash_fasta_file_to_minmax, METH_VARARGS, "" },
  { "output_fasta_kmer_pos_freq", hash_output_fasta_kmer_pos_freq, METH_VARARGS, "" },
  { "get", hash_get, METH_VARARGS, "Get the count for the given k-mer" },
  { "max_hamming1_count", hash_max_hamming1_count, METH_VARARGS, "Get the count for the given k-mer" },
  { "get_min_count", hash_get_min_count, METH_VARARGS, "Get the smallest count of all the k-mers in the string" },
  { "get_max_count", hash_get_max_count, METH_VARARGS, "Get the largest count of all the k-mers in the string" },
  { "get_median_count", hash_get_median_count, METH_VARARGS, "Get the median, average, and stddev of the k-mer counts in the string" },
  { "get_kadian_count", hash_get_kadian_count, METH_VARARGS, "Get the kadian (abundance of k-th rank-ordered k-mer) of the k-mer counts in the string" },
  { "trim_on_abundance", count_trim_on_abundance, METH_VARARGS, "Trim on >= abundance" },
  { "trim_below_abundance", count_trim_below_abundance, METH_VARARGS, "Trim on >= abundance" },
  { "abundance_distribution", hash_abundance_distribution, METH_VARARGS, "" },
  { "fasta_count_kmers_by_position", hash_fasta_count_kmers_by_position, METH_VARARGS, "" },
  { "fasta_dump_kmers_by_abundance", hash_fasta_dump_kmers_by_abundance, METH_VARARGS, "" },
  { "load", hash_load, METH_VARARGS, "" },
  { "save", hash_save, METH_VARARGS, "" },
  { "get_kmer_abund_abs_deviation", hash_get_kmer_abund_abs_deviation, METH_VARARGS, "" },
  { "get_kmer_abund_mean", hash_get_kmer_abund_mean, METH_VARARGS, "" },
  { "collect_high_abundance_kmers", hash_collect_high_abundance_kmers,
    METH_VARARGS, "" },

  {NULL, NULL, 0, NULL}           /* sentinel */
};

static PyObject *
khmer_counting_getattr(PyObject * obj, char * name)
{
  return Py_FindMethod(khmer_counting_methods, obj, name);
}

#define is_counting_obj(v)  ((v)->ob_type == &khmer_KCountingHashType)

static PyTypeObject khmer_KCountingHashType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "KCountingHash", sizeof(khmer_KCountingHashObject),
    0,
    khmer_counting_dealloc,	/*tp_dealloc*/
    0,				/*tp_print*/
    khmer_counting_getattr,	/*tp_getattr*/
    0,				/*tp_setattr*/
    0,				/*tp_compare*/
    0,				/*tp_repr*/
    0,				/*tp_as_number*/
    0,				/*tp_as_sequence*/
    0,				/*tp_as_mapping*/
    0,				/*tp_hash */
    0,				/*tp_call*/
    0,				/*tp_str*/
    0,				/*tp_getattro*/
    0,				/*tp_setattro*/
    0,				/*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,		/*tp_flags*/
    "counting hash object",           /* tp_doc */
};

//
// new_hashtable
//

static PyObject* new_hashtable(PyObject * self, PyObject * args)
{
  unsigned int k = 0;
  unsigned long long size = 0;

  if (!PyArg_ParseTuple(args, "IL", &k, &size)) {
    return NULL;
  }

  khmer_KCountingHashObject * kcounting_obj = (khmer_KCountingHashObject *) \
    PyObject_New(khmer_KCountingHashObject, &khmer_KCountingHashType);

  kcounting_obj->counting = new khmer::CountingHash(k, size);

  return (PyObject *) kcounting_obj;
}

//
// new_counting_hash
//

static PyObject* _new_counting_hash(PyObject * self, PyObject * args)
{
  unsigned int k = 0;
  PyObject* sizes_list_o = NULL;
  unsigned int n_threads = 1;

  if (!PyArg_ParseTuple(args, "IO|I", &k, &sizes_list_o, &n_threads)) {
    return NULL;
  }

  std::vector<khmer::HashIntoType> sizes;
  for (int i = 0; i < PyObject_Length(sizes_list_o); i++) {
    PyObject * size_o = PyList_GET_ITEM(sizes_list_o, i);
    sizes.push_back(PyLong_AsLongLong(size_o));
  }

  khmer_KCountingHashObject * kcounting_obj = (khmer_KCountingHashObject *) \
    PyObject_New(khmer_KCountingHashObject, &khmer_KCountingHashType);

  kcounting_obj->counting = new khmer::CountingHash(k, sizes, n_threads);

  return (PyObject *) kcounting_obj;
}

//
// hashbits stuff
//

static PyObject * hashbits_n_unique_kmers(PyObject * self, PyObject * args)
{
    khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
    khmer::Hashbits * hashbits = me->hashbits;
    
    khmer::HashIntoType start = 0, stop = 0;
    
    if (!PyArg_ParseTuple(args, "|LL", &start, &stop)) {
        return NULL;
    }
    
    khmer::HashIntoType n = hashbits->n_kmers(start, stop);
    
    return PyInt_FromLong(n);
}


static PyObject * hashbits_count_overlap(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;
  khmer_KHashbitsObject * ht2_argu;
  char * filename;
  khmer::HashIntoType lower_bound = 0, upper_bound = 0;
  PyObject * callback_obj = NULL;
  khmer::Hashbits * ht2;

  if (!PyArg_ParseTuple(args, "sO|iiO", &filename, &ht2_argu,&lower_bound, &upper_bound,
			&callback_obj)) {
    return NULL;
  }

  ht2 = ht2_argu->hashbits;

  // call the C++ function, and trap signals => Python

  unsigned long long n_consumed;
  unsigned int total_reads;
  khmer::HashIntoType curve[2][100];

  try {
    hashbits->consume_fasta_overlap(filename, curve, *ht2, total_reads, n_consumed,
			     lower_bound, upper_bound, _report_fn, callback_obj);
  } catch (_khmer_signal &e) {
    return NULL;
  }

    khmer::HashIntoType start = 0, stop = 0;

    khmer::HashIntoType n = hashbits->n_kmers(start, stop);
    khmer::HashIntoType n_overlap = hashbits->n_overlap_kmers(start,stop);

  PyObject * x = PyList_New(200);

  for (unsigned int i = 0; i < 100; i++) {
    PyList_SetItem(x, i, Py_BuildValue("i", curve[0][i]));
  }
  for (unsigned int i = 0; i < 100; i++) {
    PyList_SetItem(x, i+100, Py_BuildValue("i", curve[1][i]));
  }
  return Py_BuildValue("LLO", n, n_overlap,x);
}

static PyObject * hashbits_n_occupied(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  khmer::HashIntoType start = 0, stop = 0;

  if (!PyArg_ParseTuple(args, "|LL", &start, &stop)) {
    return NULL;
  }

  khmer::HashIntoType n = hashbits->n_occupied(start, stop);

  return PyInt_FromLong(n);
}

static PyObject * hashbits_n_tags(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  return PyInt_FromLong(hashbits->n_tags());
}

static PyObject * hashbits_count(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  char * kmer;

  if (!PyArg_ParseTuple(args, "s", &kmer)) {
    return NULL;
  }

  if (strlen(kmer) != hashbits->ksize()) {
    PyErr_SetString(PyExc_ValueError,
		    "k-mer length must be the same as the hashbits k-size");
    return NULL;
  }

  hashbits->count(kmer);

  return PyInt_FromLong(1);
}

static PyObject * hashbits_consume(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  char * long_str;
  khmer::HashIntoType lower_bound = 0, upper_bound = 0;

  if (!PyArg_ParseTuple(args, "s|ll", &long_str, &lower_bound, &upper_bound)) {
    return NULL;
  }
  
  if (strlen(long_str) < hashbits->ksize()) {
    PyErr_SetString(PyExc_ValueError,
		    "string length must >= the hashbits k-mer size");
    return NULL;
  }

  unsigned int n_consumed;
  n_consumed = hashbits->consume_string(long_str, lower_bound, upper_bound);

  return PyInt_FromLong(n_consumed);
}

static PyObject * hashbits_print_stop_tags(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  char * filename = NULL;

  if (!PyArg_ParseTuple(args, "s", &filename)) {
    return NULL;
  }

  hashbits->print_stop_tags(filename);
  
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject * hashbits_print_tagset(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  char * filename = NULL;

  if (!PyArg_ParseTuple(args, "s", &filename)) {
    return NULL;
  }

  hashbits->print_tagset(filename);
  
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject * hashbits_load_stop_tags(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  char * filename = NULL;
  PyObject * clear_tags_o = NULL;

  if (!PyArg_ParseTuple(args, "s|O", &filename, &clear_tags_o)) {
    return NULL;
  }

  bool clear_tags = true;
  if (clear_tags_o && !PyObject_IsTrue(clear_tags_o)) {
    clear_tags = false;
  }
  hashbits->load_stop_tags(filename, clear_tags);
  
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject * hashbits_save_stop_tags(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  char * filename = NULL;

  if (!PyArg_ParseTuple(args, "s", &filename)) {
    return NULL;
  }

  hashbits->save_stop_tags(filename);
  
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject * hashbits_traverse_from_tags(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  PyObject * counting_o = NULL;
  unsigned int distance, threshold, frequency;

  if (!PyArg_ParseTuple(args, "OIII", &counting_o, &distance, &threshold, &frequency)) {
    return NULL;
  }

  khmer::CountingHash * counting = ((khmer_KCountingHashObject *) counting_o)->counting;

  hashbits->traverse_from_tags(distance, threshold, frequency, *counting);

  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject * hashbits_repartition_largest_partition(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  PyObject * counting_o = NULL;
  PyObject * subset_o = NULL;
  unsigned int distance, threshold, frequency;

  if (!PyArg_ParseTuple(args, "OOIII", &subset_o, &counting_o, &distance, &threshold, &frequency)) {
    return NULL;
  }

  khmer::SubsetPartition * subset_p;
  if (subset_o != Py_None) {
    subset_p = (khmer::SubsetPartition *) PyCObject_AsVoidPtr(subset_o);
  } else {
    subset_p = hashbits->partition;
  }

  khmer::CountingHash * counting = ((khmer_KCountingHashObject *) counting_o)->counting;

  unsigned int next_largest = subset_p->repartition_largest_partition(distance, threshold, frequency, *counting);

  return PyInt_FromLong(next_largest);
}

static PyObject * hashbits_hitraverse_to_stoptags(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  PyObject * counting_o = NULL;
  unsigned int cutoff = 0;
  char * filename = NULL;

  if (!PyArg_ParseTuple(args, "sOI", &filename, &counting_o, &cutoff)) {
    return NULL;
  }

  khmer::CountingHash * counting = ((khmer_KCountingHashObject *) counting_o)->counting;

  hashbits->hitraverse_to_stoptags(filename, *counting, cutoff);
  
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject * hashbits_get(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  PyObject * arg;

  if (!PyArg_ParseTuple(args, "O", &arg)) {
    return NULL;
  }

  unsigned long count = 0;

  if (PyInt_Check(arg)) {
    long pos = PyInt_AsLong(arg);
    count = hashbits->get_count((unsigned int) pos);
  } else if (PyString_Check(arg)) {
    std::string s = PyString_AsString(arg);
    count = hashbits->get_count(s.c_str());
  }

  return PyInt_FromLong(count);
}

static PyObject * hashbits_calc_connected_graph_size(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  char * _kmer;
  unsigned int max_size = 0;
  PyObject * break_on_circum_o = NULL;
  if (!PyArg_ParseTuple(args, "s|iO", &_kmer, &max_size, &break_on_circum_o)) {
    return NULL;
  }

  bool break_on_circum = false;
  if (break_on_circum_o && PyObject_IsTrue(break_on_circum_o)) {
    break_on_circum = true;
  }

  unsigned long long size = 0;

  Py_BEGIN_ALLOW_THREADS
  khmer::SeenSet keeper;
  hashbits->calc_connected_graph_size(_kmer, size, keeper, max_size,
				      break_on_circum);
  Py_END_ALLOW_THREADS

  return PyInt_FromLong(size);
}

static PyObject * hashbits_kmer_degree(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  char * kmer_s = NULL;
  PyObject * callback_obj = NULL;

  if (!PyArg_ParseTuple(args, "s|O", &kmer_s, &callback_obj)) {
    return NULL;
  }

  return PyInt_FromLong(hashbits->kmer_degree(kmer_s));
}

static PyObject * hashbits_trim_on_degree(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  char * seq = NULL;
  unsigned int max_degree = 0;

  if (!PyArg_ParseTuple(args, "si", &seq, &max_degree)) {
    return NULL;
  }

  unsigned int trim_at;
  Py_BEGIN_ALLOW_THREADS

    trim_at = hashbits->trim_on_degree(seq, max_degree);

  Py_END_ALLOW_THREADS;

  PyObject * trim_seq = PyString_FromStringAndSize(seq, trim_at);
  PyObject * ret = Py_BuildValue("Oi", trim_seq, trim_at);
  Py_DECREF(trim_seq);

  return ret;
}

static PyObject * hashbits_trim_on_sodd(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  char * seq = NULL;
  unsigned int max_sodd = 0;

  if (!PyArg_ParseTuple(args, "si", &seq, &max_sodd)) {
    return NULL;
  }

  unsigned int trim_at;
  Py_BEGIN_ALLOW_THREADS

    trim_at = hashbits->trim_on_sodd(seq, max_sodd);

  Py_END_ALLOW_THREADS;

  PyObject * trim_seq = PyString_FromStringAndSize(seq, trim_at);
  PyObject * ret = Py_BuildValue("Oi", trim_seq, trim_at);
  Py_DECREF(trim_seq);

  return ret;
}

static PyObject * hashbits_trim_on_stoptags(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  char * seq = NULL;

  if (!PyArg_ParseTuple(args, "s", &seq)) {
    return NULL;
  }

  unsigned int trim_at;
  Py_BEGIN_ALLOW_THREADS

    trim_at = hashbits->trim_on_stoptags(seq);

  Py_END_ALLOW_THREADS;

  PyObject * trim_seq = PyString_FromStringAndSize(seq, trim_at);
  PyObject * ret = Py_BuildValue("Oi", trim_seq, trim_at);
  Py_DECREF(trim_seq);

  return ret;
}

static PyObject * hashbits_identify_stoptags_by_position(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  char * seq = NULL;

  if (!PyArg_ParseTuple(args, "s", &seq)) {
    return NULL;
  }

  std::vector<unsigned int> posns;
  Py_BEGIN_ALLOW_THREADS

    hashbits->identify_stop_tags_by_position(seq, posns);

  Py_END_ALLOW_THREADS;

  PyObject * x = PyList_New(posns.size());
  
  for (unsigned int i = 0; i < posns.size(); i++) {
    PyList_SET_ITEM(x, i, Py_BuildValue("i", posns[i]));
  }

  return x;
}

void free_subset_partition_info(void * p)
{
  khmer::SubsetPartition * subset_p = (khmer::SubsetPartition *) p;
  delete subset_p;
}

static PyObject * hashbits_do_subset_partition(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  PyObject * callback_obj = NULL;
  khmer::HashIntoType start_kmer = 0, end_kmer = 0;
  PyObject * break_on_stop_tags_o = NULL;
  PyObject * stop_big_traversals_o = NULL;

  if (!PyArg_ParseTuple(args, "|KKOOO", &start_kmer, &end_kmer,
			&break_on_stop_tags_o,
			&stop_big_traversals_o,
			&callback_obj)) {
    return NULL;
  }

  bool break_on_stop_tags = false;
  if (break_on_stop_tags_o && PyObject_IsTrue(break_on_stop_tags_o)) {
    break_on_stop_tags = true;
  }
  bool stop_big_traversals = false;
  if (stop_big_traversals_o && PyObject_IsTrue(stop_big_traversals_o)) {
    stop_big_traversals = true;
  }

  khmer::SubsetPartition * subset_p = NULL;
  try {
    Py_BEGIN_ALLOW_THREADS
    subset_p = new khmer::SubsetPartition(hashbits);
    subset_p->do_partition(start_kmer, end_kmer, break_on_stop_tags,
			   stop_big_traversals,
			   _report_fn, callback_obj);
    Py_END_ALLOW_THREADS
  } catch (_khmer_signal &e) {
    return NULL;
  }

  return PyCObject_FromVoidPtr(subset_p, free_subset_partition_info);
}

static PyObject * hashbits_join_partitions_by_path(PyObject * self, PyObject *args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  char * sequence = NULL;
  if (!PyArg_ParseTuple(args, "s", &sequence)) {
    return NULL;
  }

  hashbits->partition->join_partitions_by_path(sequence);

  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject * hashbits_merge_subset(PyObject * self, PyObject *args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  PyObject * subset_obj;
  if (!PyArg_ParseTuple(args, "O", &subset_obj)) {
    return NULL;
  }

  if (!PyCObject_Check(subset_obj)) {
    return NULL;
  }

  khmer::SubsetPartition * subset_p;
  subset_p = (khmer::SubsetPartition *) PyCObject_AsVoidPtr(subset_obj);

  hashbits->partition->merge(subset_p);

  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject * hashbits_merge_from_disk(PyObject * self, PyObject *args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  char * filename = NULL;
  if (!PyArg_ParseTuple(args, "s", &filename)) {
    return NULL;
  }

  hashbits->partition->merge_from_disk(filename);

  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject * hashbits_consume_fasta(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  char * filename;
  khmer::HashIntoType lower_bound = 0, upper_bound = 0;
  PyObject * callback_obj = NULL;

  if (!PyArg_ParseTuple(args, "s|iiO", &filename, &lower_bound, &upper_bound,
			&callback_obj)) {
    return NULL;
  }

  // call the C++ function, and trap signals => Python

  unsigned long long n_consumed;
  unsigned int total_reads;

  try {
    hashbits->consume_fasta(filename, total_reads, n_consumed,
			     lower_bound, upper_bound, 
			     _report_fn, callback_obj);
  } catch (_khmer_signal &e) {
    return NULL;
  }

  return Py_BuildValue("iL", total_reads, n_consumed);
}

static PyObject * hashbits_traverse_from_reads(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  char * filename;
  unsigned int radius, big_threshold, transfer_threshold;
  PyObject * counting_o = NULL;

  if (!PyArg_ParseTuple(args, "siiiO", &filename,
			&radius, &big_threshold, &transfer_threshold,
			&counting_o)) {
    return NULL;
  }

  khmer::CountingHash * counting = ((khmer_KCountingHashObject *) counting_o)->counting;

  hashbits->traverse_from_reads(filename, radius, big_threshold,
				transfer_threshold, *counting);
      

  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject * hashbits_consume_fasta_and_traverse(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  char * filename;
  unsigned int radius, big_threshold, transfer_threshold;
  PyObject * counting_o = NULL;

  if (!PyArg_ParseTuple(args, "siiiO", &filename,
			&radius, &big_threshold, &transfer_threshold,
			&counting_o)) {
    return NULL;
  }

  khmer::CountingHash * counting = ((khmer_KCountingHashObject *) counting_o)->counting;

  hashbits->consume_fasta_and_traverse(filename, radius, big_threshold,
				       transfer_threshold, *counting);
      

  Py_INCREF(Py_None);
  return Py_None;
}

void sig(unsigned int total_reads, unsigned int n_consumed)
{
   std::cout << total_reads << " " << n_consumed << std::endl;
}

static PyObject * hashbits_consume_fasta_and_tag(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  char * filename;
  PyObject * callback_obj = NULL;

  if (!PyArg_ParseTuple(args, "s|O", &filename, &callback_obj)) {
    return NULL;
  }

  // call the C++ function, and trap signals => Python

  unsigned long long n_consumed;
  unsigned int total_reads;

  try {
    hashbits->consume_fasta_and_tag(filename, total_reads, n_consumed,
				     _report_fn, callback_obj);
  } catch (_khmer_signal &e) {
    return NULL;
  }

  return Py_BuildValue("iL", total_reads, n_consumed);
}

static PyObject * hashbits_consume_fasta_and_tag_with_reads_parser(
  PyObject * self, PyObject * args
)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  PyObject * rparser_obj = NULL;
  PyObject * callback_obj = NULL;

  if (!PyArg_ParseTuple( args, "O|O", &rparser_obj, &callback_obj ))
    return NULL;

  // TODO: Add type-checking.
  khmer_ReadParserObject * my_rparser	    = 
  (khmer_ReadParserObject *)rparser_obj;
  khmer:: read_parsers:: IParser * rparser  = my_rparser->parser;

  // TODO: Finish implementing.

}

static PyObject * hashbits_consume_fasta_and_tag_with_stoptags(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  char * filename;
  PyObject * callback_obj = NULL;

  if (!PyArg_ParseTuple(args, "s|O", &filename, &callback_obj)) {
    return NULL;
  }

  // call the C++ function, and trap signals => Python

  unsigned long long n_consumed;
  unsigned int total_reads;

  try {
    hashbits->consume_fasta_and_tag_with_stoptags(filename,
						  total_reads, n_consumed,
						  _report_fn, callback_obj);
  } catch (_khmer_signal &e) {
    return NULL;
  }

  return Py_BuildValue("iL", total_reads, n_consumed);
}

static PyObject * hashbits_consume_partitioned_fasta(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  char * filename;
  PyObject * callback_obj = NULL;

  if (!PyArg_ParseTuple(args, "s|O", &filename, &callback_obj)) {
    return NULL;
  }

  // call the C++ function, and trap signals => Python

  unsigned long long n_consumed;
  unsigned int total_reads;

  try {
    hashbits->consume_partitioned_fasta(filename, total_reads, n_consumed,
					 _report_fn, callback_obj);
  } catch (_khmer_signal &e) {
    return NULL;
  }

  return Py_BuildValue("iL", total_reads, n_consumed);
}

void free_pre_partition_info(void * p)
{
  _pre_partition_info * ppi = (_pre_partition_info *) p;
  delete ppi;
}

static PyObject * hashbits_find_all_tags(PyObject * self, PyObject *args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  char * kmer_s = NULL;

  if (!PyArg_ParseTuple(args, "s", &kmer_s)) {
    return NULL;
  }

  if (strlen(kmer_s) < hashbits->ksize()) { // @@
    return NULL;
  }

  _pre_partition_info * ppi = NULL;

  Py_BEGIN_ALLOW_THREADS

    khmer::HashIntoType kmer, kmer_f, kmer_r;
    kmer = khmer::_hash(kmer_s, hashbits->ksize(), kmer_f, kmer_r);

    ppi = new _pre_partition_info(kmer);
    hashbits->partition->find_all_tags(kmer_f, kmer_r, ppi->tagged_kmers,
				       hashbits->all_tags);
    hashbits->add_kmer_to_tags(kmer);

  Py_END_ALLOW_THREADS

  return PyCObject_FromVoidPtr(ppi, free_pre_partition_info);
}

static PyObject * hashbits_assign_partition_id(PyObject * self, PyObject *args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  PyObject * ppi_obj;
  if (!PyArg_ParseTuple(args, "O", &ppi_obj)) {
    return NULL;
  }

  if (!PyCObject_Check(ppi_obj)) {
    return NULL;
  }

  _pre_partition_info * ppi;
  ppi = (_pre_partition_info *) PyCObject_AsVoidPtr(ppi_obj);
  
  khmer::PartitionID p;
  p = hashbits->partition->assign_partition_id(ppi->kmer,
					       ppi->tagged_kmers);

  return PyInt_FromLong(p);
}

static PyObject * hashbits_add_tag(PyObject * self, PyObject *args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  char * kmer_s = NULL;
  if (!PyArg_ParseTuple(args, "s", &kmer_s)) {
    return NULL;
  }

  khmer::HashIntoType kmer = khmer::_hash(kmer_s, hashbits->ksize());
  hashbits->add_tag(kmer);

  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject * hashbits_add_stop_tag(PyObject * self, PyObject *args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  char * kmer_s = NULL;
  if (!PyArg_ParseTuple(args, "s", &kmer_s)) {
    return NULL;
  }

  khmer::HashIntoType kmer = khmer::_hash(kmer_s, hashbits->ksize());
  hashbits->add_stop_tag(kmer);

  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject * hashbits_get_stop_tags(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  khmer::WordLength k = hashbits->ksize();
  khmer::SeenSet::const_iterator si;

  PyObject * x = PyList_New(hashbits->stop_tags.size());
  unsigned long long i = 0;
  for (si = hashbits->stop_tags.begin(); si != hashbits->stop_tags.end(); si++)
    {
      std::string s = khmer::_revhash(*si, k);
      PyList_SET_ITEM(x, i, Py_BuildValue("s", s.c_str()));
      i++;
    }

  return x;
}

static PyObject * hashbits_get_tagset(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  khmer::WordLength k = hashbits->ksize();
  khmer::SeenSet::const_iterator si;

  PyObject * x = PyList_New(hashbits->all_tags.size());
  unsigned long long i = 0;
  for (si = hashbits->all_tags.begin(); si != hashbits->all_tags.end(); si++) {
    std::string s = khmer::_revhash(*si, k);
    PyList_SET_ITEM(x, i, Py_BuildValue("s", s.c_str()));
    i++;
  }

  return x;
}

static PyObject * hashbits_output_partitions(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  char * filename = NULL;
  char * output = NULL;
  PyObject * callback_obj = NULL;
  PyObject * output_unassigned_o = NULL;

  if (!PyArg_ParseTuple(args, "ss|OO", &filename, &output,
			&output_unassigned_o,
			&callback_obj)) {
    return NULL;
  }

  bool output_unassigned = false;
  if (output_unassigned_o != NULL && PyObject_IsTrue(output_unassigned_o)) {
    output_unassigned = true;
  }

  unsigned int n_partitions = 0;

  try {
    khmer::SubsetPartition * subset_p = hashbits->partition;
    n_partitions = subset_p->output_partitioned_file(filename,
						     output,
						     output_unassigned,
						     _report_fn,
						     callback_obj);
  } catch (_khmer_signal &e) {
    return NULL;
  }

  return PyInt_FromLong(n_partitions);
}

static PyObject * hashbits_find_unpart(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  char * filename = NULL;
  PyObject * traverse_o = NULL;
  PyObject * stop_big_traversals_o = NULL;
  PyObject * callback_obj = NULL;

  if (!PyArg_ParseTuple(args, "sOO|O", &filename, &traverse_o,
			&stop_big_traversals_o, &callback_obj)) {
    return NULL;
  }

  bool traverse = PyObject_IsTrue(traverse_o);
  bool stop_big_traversals = PyObject_IsTrue(stop_big_traversals_o);
  unsigned int n_singletons = 0;

  try {
    khmer::SubsetPartition * subset_p = hashbits->partition;
    n_singletons = subset_p->find_unpart(filename, traverse,
					 stop_big_traversals,
					 _report_fn,callback_obj);
  } catch (_khmer_signal &e) {
    return NULL;
  }

  return PyInt_FromLong(n_singletons);

  // Py_INCREF(Py_None);
  // return Py_None;
}

static PyObject * hashbits_filter_if_present(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  char * filename = NULL;
  char * output = NULL;
  PyObject * callback_obj = NULL;

  if (!PyArg_ParseTuple(args, "ss|O", &filename, &output, &callback_obj)) {
    return NULL;
  }

  try {
    hashbits->filter_if_present(filename, output, _report_fn, callback_obj);
  } catch (_khmer_signal &e) {
    return NULL;
  }
  
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject * hashbits_save_partitionmap(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  char * filename = NULL;

  if (!PyArg_ParseTuple(args, "s", &filename)) {
    return NULL;
  }

  hashbits->partition->save_partitionmap(filename);

  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject * hashbits_load_partitionmap(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  char * filename = NULL;

  if (!PyArg_ParseTuple(args, "s", &filename)) {
    return NULL;
  }

  hashbits->partition->load_partitionmap(filename);

  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject * hashbits__validate_partitionmap(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  hashbits->partition->_validate_pmap();

  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject * hashbits_count_partitions(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }
  
  unsigned int n_partitions = 0, n_unassigned = 0;
  hashbits->partition->count_partitions(n_partitions, n_unassigned);

  return Py_BuildValue("ii", n_partitions, n_unassigned);
}

static PyObject * hashbits_subset_count_partitions(PyObject * self,
					       PyObject * args)
{
  PyObject * subset_obj = NULL;

  if (!PyArg_ParseTuple(args, "O", &subset_obj)) {
    return NULL;
  }
  
  khmer::SubsetPartition * subset_p;
  subset_p = (khmer::SubsetPartition *) PyCObject_AsVoidPtr(subset_obj);

  unsigned int n_partitions = 0, n_unassigned = 0;
  subset_p->count_partitions(n_partitions, n_unassigned);

  return Py_BuildValue("ii", n_partitions, n_unassigned);
}

static PyObject * hashbits_subset_partition_size_distribution(PyObject * self,
					       PyObject * args)
{
  PyObject * subset_obj = NULL;

  if (!PyArg_ParseTuple(args, "O", &subset_obj)) {
    return NULL;
  }
  
  khmer::SubsetPartition * subset_p;
  subset_p = (khmer::SubsetPartition *) PyCObject_AsVoidPtr(subset_obj);

  khmer::PartitionCountDistribution d;

  unsigned int n_unassigned = 0;
  subset_p->partition_size_distribution(d, n_unassigned);

  PyObject * x = PyList_New(d.size());
  khmer::PartitionCountDistribution::const_iterator di;

  unsigned int i;
  for (i = 0, di = d.begin(); di != d.end(); di++, i++) {
    PyList_SET_ITEM(x, i, Py_BuildValue("LL", di->first, di->second));
  }
  assert (i == d.size());

  return Py_BuildValue("Oi", x, n_unassigned);
}

static PyObject * hashbits_load(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  char * filename = NULL;

  if (!PyArg_ParseTuple(args, "s", &filename)) {
    return NULL;
  }

  hashbits->load(filename);

  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject * hashbits_save(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  char * filename = NULL;

  if (!PyArg_ParseTuple(args, "s", &filename)) {
    return NULL;
  }

  hashbits->save(filename);

  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject * hashbits_load_tagset(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  char * filename = NULL;
  PyObject * clear_tags_o = NULL;

  if (!PyArg_ParseTuple(args, "s|O", &filename, &clear_tags_o)) {
    return NULL;
  }

  bool clear_tags = true;
  if (clear_tags_o && !PyObject_IsTrue(clear_tags_o)) {
    clear_tags = false;
  }
  hashbits->load_tagset(filename, clear_tags);

  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject * hashbits_save_tagset(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  char * filename = NULL;

  if (!PyArg_ParseTuple(args, "s", &filename)) {
    return NULL;
  }

  hashbits->save_tagset(filename);

  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject * hashbits_save_subset_partitionmap(PyObject * self, PyObject * args)
{
  char * filename = NULL;
  PyObject * subset_obj = NULL;

  if (!PyArg_ParseTuple(args, "Os", &subset_obj, &filename)) {
    return NULL;
  }

  khmer::SubsetPartition * subset_p;
  subset_p = (khmer::SubsetPartition *) PyCObject_AsVoidPtr(subset_obj);

  Py_BEGIN_ALLOW_THREADS

  subset_p->save_partitionmap(filename);

  Py_END_ALLOW_THREADS

  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject * hashbits_load_subset_partitionmap(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  char * filename = NULL;

  if (!PyArg_ParseTuple(args, "s", &filename)) {
    return NULL;
  }

  khmer::SubsetPartition * subset_p;
  subset_p = new khmer::SubsetPartition(hashbits);

  Py_BEGIN_ALLOW_THREADS

  subset_p->load_partitionmap(filename);

  Py_END_ALLOW_THREADS

  return PyCObject_FromVoidPtr(subset_p, free_subset_partition_info);
}

static PyObject * hashbits__set_tag_density(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  unsigned int d;
  if (!PyArg_ParseTuple(args, "i", &d)) {
    return NULL;
  }

  hashbits->_set_tag_density(d);

  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject * hashbits__get_tag_density(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  unsigned int d = hashbits->_get_tag_density();

  return PyInt_FromLong(d);
}

static PyObject * hashbits_merge2_subset(PyObject * self, PyObject * args)
{
  // khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  // khmer::Hashbits * hashbits = me->hashbits;

  PyObject * subset1_obj, * subset2_obj;

  if (!PyArg_ParseTuple(args, "OO", &subset1_obj, &subset2_obj)) {
    return NULL;
  }

  khmer::SubsetPartition * subset1_p;
  khmer::SubsetPartition * subset2_p;
  subset1_p = (khmer::SubsetPartition *) PyCObject_AsVoidPtr(subset1_obj);
  subset2_p = (khmer::SubsetPartition *) PyCObject_AsVoidPtr(subset2_obj);

  Py_BEGIN_ALLOW_THREADS

    subset1_p->merge(subset2_p);

  Py_END_ALLOW_THREADS

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject * hashbits_merge2_from_disk(PyObject * self, PyObject * args)
{
  // khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  // khmer::Hashbits * hashbits = me->hashbits;

  PyObject * subset1_obj;
  char * filename = NULL;

  if (!PyArg_ParseTuple(args, "Os", &subset1_obj, &filename)) {
    return NULL;
  }

  khmer::SubsetPartition * subset1_p;
  subset1_p = (khmer::SubsetPartition *) PyCObject_AsVoidPtr(subset1_obj);

  Py_BEGIN_ALLOW_THREADS

    subset1_p->merge_from_disk(filename);

  Py_END_ALLOW_THREADS

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject * hashbits__validate_subset_partitionmap(PyObject * self, PyObject * args)
{
  PyObject * subset_obj = NULL;

  if (!PyArg_ParseTuple(args, "O", &subset_obj)) {
    return NULL;
  }

  khmer::SubsetPartition * subset_p;
  subset_p = (khmer::SubsetPartition *) PyCObject_AsVoidPtr(subset_obj);
  subset_p->_validate_pmap();

  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject * hashbits_set_partition_id(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  char * kmer = NULL;
  khmer::PartitionID p = 0;

  if (!PyArg_ParseTuple(args, "si", &kmer, &p)) {
    return NULL;
  }

  hashbits->partition->set_partition_id(kmer, p);

  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject * hashbits_join_partitions(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  khmer::PartitionID p1 = 0, p2 = 0;

  if (!PyArg_ParseTuple(args, "ii", &p1, &p2)) {
    return NULL;
  }

  p1 = hashbits->partition->join_partitions(p1, p2);

  return PyInt_FromLong(p1);
}

static PyObject * hashbits_get_partition_id(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  char * kmer = NULL;

  if (!PyArg_ParseTuple(args, "s", &kmer)) {
    return NULL;
  }

  khmer::PartitionID partition_id;
  partition_id = hashbits->partition->get_partition_id(kmer);

  return PyInt_FromLong(partition_id);
}

static PyObject * hashbits_is_single_partition(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  char * seq = NULL;

  if (!PyArg_ParseTuple(args, "s", &seq)) {
    return NULL;
  }

  bool v = hashbits->partition->is_single_partition(seq);

  PyObject * val;
  if (v) {
    val = Py_True;
  } else {
    val = Py_False;
  }
  Py_INCREF(val);

  return val;
}

static PyObject * hashbits_divide_tags_into_subsets(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  unsigned int subset_size = 0;

  if (!PyArg_ParseTuple(args, "i", &subset_size)) {
    return NULL;
  }

  khmer::SeenSet divvy;
  hashbits->divide_tags_into_subsets(subset_size, divvy);

  PyObject * x = PyList_New(divvy.size());
  unsigned int i = 0;
  for (khmer::SeenSet::const_iterator si = divvy.begin(); si != divvy.end();
       si++, i++) {
    PyList_SET_ITEM(x, i, PyLong_FromUnsignedLongLong(*si));
  }

  return x;
}

static PyObject * hashbits_count_kmers_within_radius(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  char * kmer = NULL;
  unsigned long radius = 0;
  unsigned long max_count = 0;

  if (!PyArg_ParseTuple(args, "sL|L", &kmer, &radius, &max_count)) {
    return NULL;
  }

  unsigned int n;

  Py_BEGIN_ALLOW_THREADS

  khmer::HashIntoType kmer_f, kmer_r;
  khmer::_hash(kmer, hashbits->ksize(), kmer_f, kmer_r);
  n = hashbits->count_kmers_within_radius(kmer_f, kmer_r, radius,
						       max_count);

  Py_END_ALLOW_THREADS

  return PyLong_FromUnsignedLong(n);
}

static PyObject * hashbits_count_kmers_on_radius(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  char * kmer = NULL;
  unsigned long radius = 0;
  unsigned long max_volume = 0;

  if (!PyArg_ParseTuple(args, "sL|L", &kmer, &radius, &max_volume)) {
    return NULL;
  }

  unsigned int n;

  Py_BEGIN_ALLOW_THREADS

  khmer::HashIntoType kmer_f, kmer_r;
  khmer::_hash(kmer, hashbits->ksize(), kmer_f, kmer_r);
  n = hashbits->count_kmers_on_radius(kmer_f, kmer_r, radius, max_volume);

  Py_END_ALLOW_THREADS

  return PyLong_FromUnsignedLong(n);
}

static PyObject * hashbits_trim_on_density_explosion(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  char * seq = NULL;
  unsigned long radius = 0;
  unsigned long max_volume = 0;

  if (!PyArg_ParseTuple(args, "sLL", &seq, &radius, &max_volume)) {
    return NULL;
  }

  unsigned int trim_at;
  Py_BEGIN_ALLOW_THREADS

    trim_at = hashbits->trim_on_density_explosion(seq, radius, max_volume);

  Py_END_ALLOW_THREADS;

  PyObject * trim_seq = PyString_FromStringAndSize(seq, trim_at);
  PyObject * ret = Py_BuildValue("Oi", trim_seq, trim_at);
  Py_DECREF(trim_seq);

  return ret;
}

static PyObject * hashbits_find_radius_for_volume(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  char * kmer = NULL;
  unsigned long max_count = 0;
  unsigned long max_radius = 0;

  if (!PyArg_ParseTuple(args, "sLL", &kmer, &max_count, &max_radius)) {
    return NULL;
  }

  unsigned int n;

  Py_BEGIN_ALLOW_THREADS

  khmer::HashIntoType kmer_f, kmer_r;
  khmer::_hash(kmer, hashbits->ksize(), kmer_f, kmer_r);
  n = hashbits->find_radius_for_volume(kmer_f, kmer_r, max_count,
						    max_radius);

  Py_END_ALLOW_THREADS

  return PyLong_FromUnsignedLong(n);
}

static PyObject * hashbits_get_ksize(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  unsigned int k = hashbits->ksize();

  return PyInt_FromLong(k);
}


static PyObject * hashbits_get_hashsizes(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  std::vector<khmer::HashIntoType> ts = hashbits->get_tablesizes();

  PyObject * x = PyList_New(ts.size());
  for (unsigned int i = 0; i < ts.size(); i++) {
    PyList_SET_ITEM(x, i, PyInt_FromLong(ts[i]));
  }

  return x;
}

static PyObject * hashbits_extract_unique_paths(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  char * sequence = NULL;
  unsigned int min_length = 0;
  float min_unique_f = 0;
  if (!PyArg_ParseTuple(args, "sif", &sequence, &min_length, &min_unique_f)) {
    return NULL;
  }

  std::vector<std::string> results;
  hashbits->extract_unique_paths(sequence, min_length, min_unique_f, results);

  PyObject * x = PyList_New(results.size());
  for (unsigned int i = 0; i < results.size(); i++) {
    PyList_SET_ITEM(x, i, PyString_FromString(results[i].c_str()));
  }

  return x;
}

static PyObject * hashbits_get_median_count(PyObject * self, PyObject * args)
{
  khmer_KHashbitsObject * me = (khmer_KHashbitsObject *) self;
  khmer::Hashbits * hashbits = me->hashbits;

  char * long_str;

  if (!PyArg_ParseTuple(args, "s", &long_str)) {
    return NULL;
  }

  if (strlen(long_str) < hashbits->ksize()) {
    PyErr_SetString(PyExc_ValueError,
		    "string length must >= the hashtable k-mer size");
    return NULL;
  }

  khmer::BoundedCounterType med = 0;
  float average = 0, stddev = 0;

  hashbits->get_median_count(long_str, med, average, stddev);

  return Py_BuildValue("iff", med, average, stddev);
}

static PyMethodDef khmer_hashbits_methods[] = {
  { "extract_unique_paths", hashbits_extract_unique_paths, METH_VARARGS, "" },
  { "ksize", hashbits_get_ksize, METH_VARARGS, "" },
  { "hashsizes", hashbits_get_hashsizes, METH_VARARGS, "" },
  { "n_occupied", hashbits_n_occupied, METH_VARARGS, "Count the number of occupied bins" },
  { "n_unique_kmers", hashbits_n_unique_kmers,  METH_VARARGS, "Count the number of unique kmers" },
  { "count", hashbits_count, METH_VARARGS, "Count the given kmer" },
  { "count_overlap", hashbits_count_overlap,METH_VARARGS,"Count overlap kmers in two datasets" },
  { "consume", hashbits_consume, METH_VARARGS, "Count all k-mers in the given string" },
  { "load_stop_tags", hashbits_load_stop_tags, METH_VARARGS, "" },
  { "save_stop_tags", hashbits_save_stop_tags, METH_VARARGS, "" },
  { "print_stop_tags", hashbits_print_stop_tags, METH_VARARGS, "" },
  { "print_tagset", hashbits_print_tagset, METH_VARARGS, "" },
  { "get", hashbits_get, METH_VARARGS, "Get the count for the given k-mer" },
  { "calc_connected_graph_size", hashbits_calc_connected_graph_size, METH_VARARGS, "" },
  { "kmer_degree", hashbits_kmer_degree, METH_VARARGS, "" },
  { "trim_on_degree", hashbits_trim_on_degree, METH_VARARGS, "" },
  { "trim_on_sodd", hashbits_trim_on_sodd, METH_VARARGS, "" },
  { "trim_on_stoptags", hashbits_trim_on_stoptags, METH_VARARGS, "" },
  { "identify_stoptags_by_position", hashbits_identify_stoptags_by_position, METH_VARARGS, "" },
  { "trim_on_density_explosion", hashbits_trim_on_density_explosion, METH_VARARGS, "" },
  { "do_subset_partition", hashbits_do_subset_partition, METH_VARARGS, "" },
  { "find_all_tags", hashbits_find_all_tags, METH_VARARGS, "" },
  { "assign_partition_id", hashbits_assign_partition_id, METH_VARARGS, "" },
  { "output_partitions", hashbits_output_partitions, METH_VARARGS, "" },
  { "find_unpart", hashbits_find_unpart, METH_VARARGS, "" },
  { "filter_if_present", hashbits_filter_if_present, METH_VARARGS, "" },
  { "add_tag", hashbits_add_tag, METH_VARARGS, "" },
  { "add_stop_tag", hashbits_add_stop_tag, METH_VARARGS, "" },
  { "get_stop_tags", hashbits_get_stop_tags, METH_VARARGS, "" },
  { "get_tagset", hashbits_get_tagset, METH_VARARGS, "" },
  { "load", hashbits_load, METH_VARARGS, "" },
  { "save", hashbits_save, METH_VARARGS, "" },
  { "load_tagset", hashbits_load_tagset, METH_VARARGS, "" },
  { "save_tagset", hashbits_save_tagset, METH_VARARGS, "" },
  { "n_tags", hashbits_n_tags, METH_VARARGS, "" },
  { "divide_tags_into_subsets", hashbits_divide_tags_into_subsets, METH_VARARGS, "" },
  { "load_partitionmap", hashbits_load_partitionmap, METH_VARARGS, "" },
  { "save_partitionmap", hashbits_save_partitionmap, METH_VARARGS, "" },
  { "_validate_partitionmap", hashbits__validate_partitionmap, METH_VARARGS, "" },
  { "_get_tag_density", hashbits__get_tag_density, METH_VARARGS, "" },
  { "_set_tag_density", hashbits__set_tag_density, METH_VARARGS, "" },
  { "consume_fasta", hashbits_consume_fasta, METH_VARARGS, "Count all k-mers in a given file" },
  { "consume_fasta_and_tag", hashbits_consume_fasta_and_tag, METH_VARARGS, "Count all k-mers in a given file" },
  { "traverse_from_reads", hashbits_traverse_from_reads, METH_VARARGS, "" },
  { "consume_fasta_and_traverse", hashbits_consume_fasta_and_traverse, METH_VARARGS, "" },
  { "consume_fasta_and_tag_with_stoptags", hashbits_consume_fasta_and_tag_with_stoptags, METH_VARARGS, "Count all k-mers in a given file" },
  { "consume_partitioned_fasta", hashbits_consume_partitioned_fasta, METH_VARARGS, "Count all k-mers in a given file" },
  { "join_partitions_by_path", hashbits_join_partitions_by_path, METH_VARARGS, "" },
  { "merge_subset", hashbits_merge_subset, METH_VARARGS, "" },
  { "merge_subset_from_disk", hashbits_merge_from_disk, METH_VARARGS, "" },
  { "count_partitions", hashbits_count_partitions, METH_VARARGS, "" },
  { "subset_count_partitions", hashbits_subset_count_partitions, METH_VARARGS, "" },
  { "subset_partition_size_distribution", hashbits_subset_partition_size_distribution, METH_VARARGS, "" },
  { "save_subset_partitionmap", hashbits_save_subset_partitionmap, METH_VARARGS },
  { "load_subset_partitionmap", hashbits_load_subset_partitionmap, METH_VARARGS },
  { "merge2_subset", hashbits_merge2_subset, METH_VARARGS },
  { "merge2_subset_from_disk", hashbits_merge2_from_disk, METH_VARARGS },
  { "_validate_subset_partitionmap", hashbits__validate_subset_partitionmap, METH_VARARGS, "" },
  { "set_partition_id", hashbits_set_partition_id, METH_VARARGS, "" },
  { "join_partitions", hashbits_join_partitions, METH_VARARGS, "" },
  { "get_partition_id", hashbits_get_partition_id, METH_VARARGS, "" },
  { "is_single_partition", hashbits_is_single_partition, METH_VARARGS, "" },
  { "count_kmers_within_radius", hashbits_count_kmers_within_radius, METH_VARARGS, "" },
  { "count_kmers_on_radius", hashbits_count_kmers_on_radius, METH_VARARGS, "" },
  { "find_radius_for_volume", hashbits_find_radius_for_volume, METH_VARARGS, "" },
  { "hitraverse_to_stoptags", hashbits_hitraverse_to_stoptags, METH_VARARGS, "" },
  { "traverse_from_tags", hashbits_traverse_from_tags, METH_VARARGS, "" },
  { "repartition_largest_partition", hashbits_repartition_largest_partition, METH_VARARGS, "" },
  { "get_median_count", hashbits_get_median_count, METH_VARARGS, "Get the median, average, and stddev of the k-mer counts in the string" },

  {NULL, NULL, 0, NULL}           /* sentinel */
};

static PyObject *
khmer_hashbits_getattr(PyObject * obj, char * name)
{
  return Py_FindMethod(khmer_hashbits_methods, obj, name);
}

//
// new_hashbits
//

static PyObject* _new_hashbits(PyObject * self, PyObject * args)
{
  unsigned int k = 0;
  PyObject* sizes_list_o = NULL;

  if (!PyArg_ParseTuple(args, "IO", &k, &sizes_list_o)) {
    return NULL;
  }

  std::vector<khmer::HashIntoType> sizes;
  for (int i = 0; i < PyObject_Length(sizes_list_o); i++) {
    PyObject * size_o = PyList_GET_ITEM(sizes_list_o, i);
    sizes.push_back(PyLong_AsLongLong(size_o));
  }

  khmer_KHashbitsObject * khashbits_obj = (khmer_KHashbitsObject *) \
    PyObject_New(khmer_KHashbitsObject, &khmer_KHashbitsType);

  khashbits_obj->hashbits = new khmer::Hashbits(k, sizes);

  return (PyObject *) khashbits_obj;
}

static PyObject * hash_collect_high_abundance_kmers(PyObject * self, PyObject * args)
{
  khmer_KCountingHashObject * me = (khmer_KCountingHashObject *) self;
  khmer::CountingHash * counting = me->counting;

  char * filename = NULL;
  unsigned int lower_count, upper_count;

  if (!PyArg_ParseTuple(args, "sII", &filename, &lower_count, &upper_count)) {
    return NULL;
  }

  khmer::SeenSet found_kmers;
  counting->collect_high_abundance_kmers(filename, lower_count, upper_count,
					 found_kmers);

  // create a new hashbits object...
  std::vector<khmer::HashIntoType> sizes;
  sizes.push_back(1);

  khmer_KHashbitsObject * khashbits_obj = (khmer_KHashbitsObject *) \
    PyObject_New(khmer_KHashbitsObject, &khmer_KHashbitsType);

  // ...and set the collected kmers as the stoptags.
  khashbits_obj->hashbits = new khmer::Hashbits(counting->ksize(), sizes);
  khashbits_obj->hashbits->stop_tags.swap(found_kmers);

  return (PyObject *) khashbits_obj;
}

//
// khmer_counting_dealloc -- clean up a counting hash object.
//

static void khmer_counting_dealloc(PyObject* self)
{
  khmer_KCountingHashObject * obj = (khmer_KCountingHashObject *) self;
  delete obj->counting;
  obj->counting = NULL;
  
  PyObject_Del((PyObject *) obj);
}

//
// khmer_hashbits_dealloc -- clean up a hashbits object.
//

static void khmer_hashbits_dealloc(PyObject* self)
{
  khmer_KHashbitsObject * obj = (khmer_KHashbitsObject *) self;
  delete obj->hashbits;
  obj->hashbits = NULL;
  
  PyObject_Del((PyObject *) obj);
}

//
// MinMaxTable object
//

static PyObject * minmax_clear(PyObject * self, PyObject * args)
{
  khmer_MinMaxObject * me = (khmer_MinMaxObject *) self;
  khmer::MinMaxTable * mmt = me->mmt;

  unsigned int index;

  if (!PyArg_ParseTuple(args, "I", &index)) {
    return NULL;
  }

  mmt->clear(index);
  
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject * minmax_get_min(PyObject * self, PyObject * args)
{
  khmer_MinMaxObject * me = (khmer_MinMaxObject *) self;
  khmer::MinMaxTable * mmt = me->mmt;

  unsigned int index;

  if (!PyArg_ParseTuple(args, "I", &index)) {
    return NULL;
  }

  unsigned int val = mmt->get_min(index);

  return PyInt_FromLong(val);
}

static PyObject * minmax_get_max(PyObject * self, PyObject * args)
{
  khmer_MinMaxObject * me = (khmer_MinMaxObject *) self;
  khmer::MinMaxTable * mmt = me->mmt;

  unsigned int index;

  if (!PyArg_ParseTuple(args, "I", &index)) {
    return NULL;
  }

  unsigned int val = mmt->get_max(index);

  return PyInt_FromLong(val);
}

static PyObject * minmax_add_min(PyObject * self, PyObject * args)
{
  khmer_MinMaxObject * me = (khmer_MinMaxObject *) self;
  khmer::MinMaxTable * mmt = me->mmt;

  unsigned int index;
  unsigned int val;

  if (!PyArg_ParseTuple(args, "II", &index, &val)) {
    return NULL;
  }

  val = mmt->add_min(index, val);
  
  return PyInt_FromLong(val);
}

static PyObject * minmax_add_max(PyObject * self, PyObject * args)
{
  khmer_MinMaxObject * me = (khmer_MinMaxObject *) self;
  khmer::MinMaxTable * mmt = me->mmt;

  unsigned int index;
  unsigned int val;

  if (!PyArg_ParseTuple(args, "II", &index, &val)) {
    return NULL;
  }

  val = mmt->add_max(index, val);
  
  return PyInt_FromLong(val);
}

static PyObject * minmax_merge(PyObject * self, PyObject * args)
{
  khmer_MinMaxObject * me = (khmer_MinMaxObject *) self;
  PyObject * other_py_obj;

  khmer::MinMaxTable * mmt = me->mmt;

  if (!PyArg_ParseTuple(args, "O", &other_py_obj)) {
    return NULL;
  }

  khmer_MinMaxObject * other = (khmer_MinMaxObject *) other_py_obj;

  mmt->merge(*(other->mmt));

  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject * minmax_save(PyObject * self, PyObject * args)
{
  khmer_MinMaxObject * me = (khmer_MinMaxObject *) self;

  khmer::MinMaxTable * mmt = me->mmt;
  char * filename;

  if (!PyArg_ParseTuple(args, "s", &filename)) {
    return NULL;
  }

  mmt->save(filename);

  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject * minmax_load(PyObject * self, PyObject * args)
{
  khmer_MinMaxObject * me = (khmer_MinMaxObject *) self;

  khmer::MinMaxTable * mmt = me->mmt;
  char * filename;

  if (!PyArg_ParseTuple(args, "s", &filename)) {
    return NULL;
  }

  mmt->load(filename);

  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject * minmax_tablesize(PyObject * self, PyObject * args)
{
  khmer_MinMaxObject * me = (khmer_MinMaxObject *) self;
  khmer::MinMaxTable * mmt = me->mmt;

  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  return PyInt_FromLong(mmt->get_tablesize());
}

static PyMethodDef khmer_minmax_methods[] = {
  { "tablesize", minmax_tablesize, METH_VARARGS, "" },
  { "get_min", minmax_get_min, METH_VARARGS, "" },
  { "get_max", minmax_get_max, METH_VARARGS, "" },
  { "add_min", minmax_add_min, METH_VARARGS, "" },
  { "add_max", minmax_add_max, METH_VARARGS, "" },
  { "clear", minmax_clear, METH_VARARGS, "" },
  { "merge", minmax_merge, METH_VARARGS, "" },
  { "load", minmax_load, METH_VARARGS, "" },
  { "save", minmax_save, METH_VARARGS, "" },
  {NULL, NULL, 0, NULL}           /* sentinel */
};

static PyObject *
khmer_minmax_getattr(PyObject * obj, char * name)
{
  return Py_FindMethod(khmer_minmax_methods, obj, name);
}

//
// new_minmax
//

static PyObject* new_minmax(PyObject * self, PyObject * args)
{
  unsigned int size = 0;

  if (!PyArg_ParseTuple(args, "I", &size)) {
    return NULL;
  }

  khmer_MinMaxObject * minmax_obj = (khmer_MinMaxObject *) \
    PyObject_New(khmer_MinMaxObject, &khmer_MinMaxType);

  minmax_obj->mmt = new khmer::MinMaxTable(size);

  return (PyObject *) minmax_obj;
}

//
// khmer_minmax_dealloc -- clean up a minmax object.
//

static void khmer_minmax_dealloc(PyObject* self)
{
  khmer_MinMaxObject * obj = (khmer_MinMaxObject *) self;
  delete obj->mmt;
  obj->mmt = NULL;
  
  PyObject_Del((PyObject *) obj);
}


//////////////////////////////
// standalone functions

static PyObject * forward_hash(PyObject * self, PyObject * args)
{
  char * kmer;
  int ksize;

  if (!PyArg_ParseTuple(args, "si", &kmer, &ksize)) {
    return NULL;
  }

  if ((char)ksize != ksize) {
    PyErr_SetString(PyExc_ValueError, "k-mer size must be <= 255");
    return NULL;
  }

  return PyLong_FromUnsignedLongLong(khmer::_hash(kmer, ksize));
}

static PyObject * forward_hash_no_rc(PyObject * self, PyObject * args)
{
  char * kmer;
  unsigned int ksize;

  if (!PyArg_ParseTuple(args, "si", &kmer, &ksize)) {
    return NULL;
  }

  if ((unsigned char)ksize != ksize) {
    PyErr_SetString(PyExc_ValueError, "k-mer size must be <= 255");
    return NULL;
  }

  if (strlen(kmer) != ksize) {
    PyErr_SetString(PyExc_ValueError,
		    "k-mer length must be the same as the hashtable k-size");
    return NULL;
  }

  return PyLong_FromUnsignedLongLong(khmer::_hash_forward(kmer, ksize));
}

static PyObject * reverse_hash(PyObject * self, PyObject * args)
{
  khmer::HashIntoType val;
  int ksize;
  
  if (!PyArg_ParseTuple(args, "lI", &val, &ksize)) {
    return NULL;
  }

  if ((char)ksize != ksize) {
    PyErr_SetString(PyExc_ValueError, "k-mer size must be <= 255");
    return NULL;
  }

  return PyString_FromString(khmer::_revhash(val, ksize).c_str());
}

static PyObject * set_reporting_callback(PyObject * self, PyObject * args)
{
  PyObject * o;
  
  if (!PyArg_ParseTuple(args, "O", &o)) {
    return NULL;
  }

  Py_XDECREF(_callback_obj);
  Py_INCREF(o);
  _callback_obj = o;

  Py_INCREF(Py_None);
  return Py_None;
}

//
// Module machinery.
//

static PyMethodDef KhmerMethods[] = {
  /* { "new_config", new_config, METH_VARARGS, "Create a default internals config" }, */
  { "get_config", get_config, METH_VARARGS, "Get active khmer configuration object" },
  /* { "set_config", set_active_config, METH_VARARGS, "Set active khmer configuration object" }, */
  { "new_read_parser", new_read_parser, METH_VARARGS, "Create a new read parser" },
  { "new_ktable", new_ktable, METH_VARARGS, "Create an empty ktable" },
  { "new_hashtable", new_hashtable, METH_VARARGS, "Create an empty single-table counting hash" },
  { "_new_counting_hash", _new_counting_hash, METH_VARARGS, "Create an empty counting hash" },
  { "_new_hashbits", _new_hashbits, METH_VARARGS, "Create an empty hashbits table" },
  { "new_minmax", new_minmax, METH_VARARGS, "Create a new min/max value table" },
  { "forward_hash", forward_hash, METH_VARARGS, "", },
  { "forward_hash_no_rc", forward_hash_no_rc, METH_VARARGS, "", },
  { "reverse_hash", reverse_hash, METH_VARARGS, "", },
  { "set_reporting_callback", set_reporting_callback, METH_VARARGS, "" },
  { NULL, NULL, 0, NULL }
};

DL_EXPORT(void) init_khmer(void)
{
  khmer_ConfigType.ob_type	  = &PyType_Type;
  khmer_ReadType.ob_type	  = &PyType_Type;
  khmer_ReadParserType.ob_type	  = &PyType_Type;
  khmer_KTableType.ob_type	  = &PyType_Type;
  khmer_KCountingHashType.ob_type = &PyType_Type;

  PyObject * m;
  m = Py_InitModule("_khmer", KhmerMethods);

  KhmerError = PyErr_NewException((char *)"_khmer.error", NULL, NULL);
  Py_INCREF(KhmerError);

  PyModule_AddObject(m, "error", KhmerError);
}

// vim: set sts=2 sw=2:
