// -*- C++ -*-
#ifndef SP_BASE_H
#define SP_BASE_H

// needed for pid_t typedef
#include <sys/types.h>
#include "LString.h" // can't forward declare...

class BufferParams;

class SpellBase
{
 public:

   /// status
   enum spellStatus  {
	ISP_OK = 1,
	ISP_ROOT,
	ISP_COMPOUNDWORD,
	ISP_UNKNOWN,
	ISP_MISSED,
	ISP_IGNORE
   };

   virtual ~SpellBase() {}

   virtual void initialize(BufferParams const & , string const &) = 0;

   virtual bool alive() = 0;

   virtual void cleanUp() = 0;

   virtual enum spellStatus check(string const &) = 0;

   virtual void close() = 0;

   virtual void insert(string const &) = 0;

   virtual void accept(string const &) = 0;

   virtual void store(string const & , string const &) = 0;

   virtual char const * error() = 0;

   virtual char const * nextMiss() = 0;

};

#endif
