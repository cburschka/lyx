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
class FuncRequest;
class LyXText;

namespace lyx {
namespace find {

/** Encode the parameters needed to find \c search as a string
 *  that can be dispatched to the LyX core in a FuncRequest wrapper.
 */
std::string const find2string(std::string const & search,
			      bool casesensitive,
			      bool matchword,
			      bool forward);

/** Encode the parameters needed to replace \c search with \c replace
 *  as a string that can be dispatched to the LyX core in a FuncRequest
 *  wrapper.
 */
std::string const replace2string(std::string const & search,
				 std::string const & replace,
				 bool casesensitive,
				 bool matchword,
				 bool all,
				 bool forward);

/** Parse the string encoding of the find request that is found in
 *  \c ev.argument and act on it.
 * The string is encoded by \c find2string.
 */
void find(FuncRequest const & ev);

/** Parse the string encoding of the replace request that is found in
 *  \c ev.argument and act on it.
 * The string is encoded by \c replace2string.
 */
void replace(FuncRequest const &);

/// find the next change in the buffer
bool findNextChange(BufferView * bv);

} // namespace find
} // namespace lyx

#endif // LYXFIND_H
