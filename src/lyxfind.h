#ifndef LYXFIND_H
#define LYXFIND_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"
#include "lyxparagraph.h"

class BufferView;

int LyXReplace(BufferView * bv, string const &, string const &,
	       bool const &, bool const &, bool const &, bool const & = false);

bool LyXFind(BufferView * bv, string const &, bool const &, bool const &,
	     bool const &);

/// returns true if the specified string is at the specified  position
bool IsStringInText(LyXParagraph * par, LyXParagraph::size_type pos,
		    string const & str, bool const & = true,
		    bool const & = false);

/// if the string is found: return true and set the cursor to the new position
bool SearchForward(BufferView *, string const & str, bool const & = true, 
		   bool const & = false);
///
bool SearchBackward(BufferView *, string const & str, bool const & = true, 
		    bool const & = false);


#endif
