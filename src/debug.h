// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#ifndef LYXDEBUG_H
#define LYXDEBUG_H

#ifdef __GNUG__
#pragma interface
#endif

#include <iosfwd>

#include "LString.h"

/** Ideally this should have been a namespace, but since we try to be
    compilable on older C++ compilators too, we use a struct instead.
    This is all the different debug levels that we have.
*/
struct Debug {
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
		GRAPHICS   = (1 << 21)
	};
	///
	static type const ANY;

	/** A function to convert symbolic string names on debug levels
	    to their numerical value.
	*/
	static Debug::type value(string const & val); 

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


#include "support/DebugStream.h"



std::ostream & operator<<(std::ostream & o, Debug::type t);

extern DebugStream lyxerr;

#endif
