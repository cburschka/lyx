// -*- C++ -*-
/**
 * \file debug.h
 *
 * FIXME: It would be nice if, in lyx::use_gui mode, instead of
 * outputting to the console, we would pipe all messages onto a file
 * and visualise the contents dynamically in a Qt window if needed.
 *
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
#include "support/strfwd.h"


namespace lyx {

/** Ideally this should have been a namespace, but since we try to be
 *  compilable on older C++ compilators too, we use a struct instead.
 *  This is all the different debug levels that we have.
 */
class Debug {
public:
	///
	enum type {
		///
		NONE = 0,
		///
		INFO       = (1 << 0),   // 1
		///
		INIT       = (1 << 1),   // 2
		///
		KEY        = (1 << 2),   // 4
		///
		GUI        = (1 << 3),   // 8
		///
		PARSER     = (1 << 4),   // 16
		///
		LYXRC      = (1 << 5),   // 32
		///
		KBMAP      = (1 << 6),   // 64
		///
		LATEX      = (1 << 7),   // 128
		///
		MATHED     = (1 << 8),   // 256 // Alejandro, please use this.
		///
		FONT       = (1 << 9),   // 512
		///
		TCLASS     = (1 << 10),  // 1024
		///
		LYXVC      = (1 << 11),  // 2048
		///
		LYXSERVER  = (1 << 12),  // 4096
		///
		ROFF       = (1 << 13),  // 8192
		///
		ACTION     = (1 << 14),   // 16384
		///
		LYXLEX     = (1 << 15),
		///
		DEPEND     = (1 << 16),
		///
		INSETS     = (1 << 17),
		///
		FILES      = (1 << 18),
		///
		WORKAREA   = (1 << 19),
		///
		INSETTEXT  = (1 << 20),
		///
		GRAPHICS   = (1 << 21),
		/// change tracking
		CHANGES    = (1 << 22),
		///
		EXTERNAL   = (1 << 23),
		///
		PAINTING   = (1 << 24),
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
void operator|=(Debug::type & d1, Debug::type d2)
{
	d1 = static_cast<Debug::type>(d1 | d2);
}


typedef basic_debugstream<Debug> LyXErr;

extern LyXErr lyxerr;

#define LYXERR(type, msg) \
	do { \
		if (!lyx::lyxerr.debugging(type)) {} else lyx::lyxerr << msg << std::endl; \
	} while (0)


} // namespace lyx

#endif
