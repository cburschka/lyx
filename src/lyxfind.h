// -*- C++ -*-
#ifndef LYXFIND_H
#define LYXFIND_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"

class BufferView;
class LyXText;

namespace lyxfind {

enum SearchResult {
	//
	SR_NOT_FOUND = 0,
	//
	SR_FOUND,
	//
	SR_FOUND_NOUPDATE
};


int LyXReplace(BufferView * bv,
	       string const &, string const &,
	       bool, bool = true, bool = false,
	       bool = false, bool = false);

/**
 * This function is called as a general interface to find some
 * text from the actual cursor position in whatever direction
 * we want to go. This does also update the screen.
 */
bool LyXFind(BufferView *,
	     string const & searchstr, bool forward,
	     bool casesens = true, bool matchwrd = false);

/**
 * This function does search from the cursor position inside the
 * passed LyXText parameter and regards this LyXText as the root
 * LyXText. It will NOT update any screen stuff. It will however
 * set the cursor to the new position inside LyXText, before
 * returning to the calling function.
 */

SearchResult LyXFind(BufferView *, LyXText * text,
		     string const & searchstr, bool forward,
		     bool casesens = true, bool matchwrd = false);

} // end namespace LyXFind
#endif
