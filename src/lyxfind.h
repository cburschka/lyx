// -*- C++ -*-
/**
 * \file lyxfind.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author John Levon
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYXFIND_H
#define LYXFIND_H

#include "LString.h"
#include "support/types.h"

class BufferView;
class LyXText;

namespace lyx {
namespace find {

enum SearchResult {
	//
	SR_NOT_FOUND = 0,
	//
	SR_FOUND,
	//
	SR_FOUND_NOUPDATE
};


int replace(BufferView * bv,
	       string const &, string const &,
	       bool, bool = true, bool = false,
	       bool = false, bool = false);

/**
 * This function is called as a general interface to find some
 * text from the actual cursor position in whatever direction
 * we want to go. This does also update the screen.
 */
bool find(BufferView *,
	     string const & searchstr, bool forward,
	     bool casesens = true, bool matchwrd = false);

/**
 * This function does search from the cursor position inside the
 * passed LyXText parameter and regards this LyXText as the root
 * LyXText. It will NOT update any screen stuff. It will however
 * set the cursor to the new position inside LyXText, before
 * returning to the calling function.
 */

SearchResult find(BufferView *, LyXText * text,
		     string const & searchstr, bool forward,
		     bool casesens = true, bool matchwrd = false);

/// find the next change in the buffer
bool findNextChange(BufferView * bv);

SearchResult findNextChange(BufferView * bv, LyXText * text, lyx::pos_type & length);

SearchResult nextChange(BufferView * bv, LyXText * text, lyx::pos_type & length);

} // namespace find
} // namespace lyx

#endif // LYXFIND_H
