// -*- C++ -*-
/**
 * \file lyxfind.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author John Levon
 * \author Jürgen Vigna
 * \author Alfredo Braunstein
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYXFIND_H
#define LYXFIND_H

#include "support/types.h"

#include <string>

class BufferView;
class LyXText;

namespace lyx {
namespace find {

/**
 * This function replaces an ocurrence of \param search with the
 * string \param replace
 *
 *  \param bv the BufferView in which the search is to be performed,
 *         starting at the current cursor position.
 *  \param search the string we're looking for.
 *  \param replace if \c search is found, replace it with this.
 *  \param cs perform a case-sensitive search for \c search.
 *  \param mw match whole words only.
 *  \param fw search forward from the current cursor position.
 */

int replace(BufferView * bv,
	    std::string const & search, std::string const & replace,
	    bool cs, bool mw, bool fw);
/**
 * This function replaces all ocurrences of \param search with
 * the string \param replace
 *
 *  \param bv the BufferView in which the search is to be performed,
 *         starting at the current cursor position.
 *  \param search the string we're looking for.
 *  \param replace if \c search is found, replace it with this.
 *  \param cs perform a case-sensitive search for \c search.
 *  \param mw match whole words only.
 */

int replaceAll(BufferView * bv,
	       std::string const & search, std::string const & replace,
	       bool cs, bool mw);

/**
 * This function is called as a general interface to find some text
 * from the actual cursor position in whatever direction we want to
 * go. This does also update the screen.
 */
bool find(BufferView *, std::string const & search, 
	  bool cs, bool mw, bool fw);


/// find the next change in the buffer
bool findNextChange(BufferView * bv);

} // namespace find
} // namespace lyx

#endif // LYXFIND_H
