// -*- C++ -*-
/**
 * \file debug.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYXDEBUG_H
#define LYXDEBUG_H

#include "support/debugstream.h"
#include "support/docstring.h"


namespace lyx {

/** Ideally this should have been a namespace, but since we try to be
 *  compilable on older C++ compilators too, we use a struct instead.
 *  This is all the different debug levels that we have.
 */
class lyx_debug_trait {
public:
	///
	enum type {
		///
		NONE = 0,
		///
		INFO       = (1 << 0),
		///
		DEBUG      = (1 << 31),
		///
		ANY = 0xffffffff
	};

	static bool match(type a, type b) { return (a & b); }

	/** A function to convert symbolic string names on debug levels
	    to their numerical value.
	*/
	static type value(std::string const & val);

	/** Display the tags and descriptions of the current debug level
	    of ds
	*/
	static void showLevel(std::ostream & o, type level);

	/** show all the possible tags that can be used for debugging */
	static void showTags(std::ostream & o);

};



inline
void operator|=(lyx_debug_trait::type & d1, lyx_debug_trait::type d2)
{
	d1 = static_cast<lyx_debug_trait::type>(d1 | d2);
}


typedef basic_debugstream<lyx_debug_trait> LyXErr;
typedef LyXErr::debug Debug;

extern LyXErr lyxerr;

} // namespace lyx

#define LYXERR(type, msg) \
	do { if (!lyx::lyxerr.debugging(type)) {} \
       else lyx::lyxerr << msg  << std::endl; } while (0)


#endif
