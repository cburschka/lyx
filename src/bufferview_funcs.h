// -*- C++ -*-
/**
 * \file bufferview_funcs.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef BUFFERVIEW_FUNCS_H
#define BUFFERVIEW_FUNCS_H

#include <string>


class BufferView;
class LyXFont;
class LyXText;
class PosIterator;


namespace bv_funcs {

/** Set \param data using \param font and \param toggle.
 *  If successful, returns true.
 */
bool font2string(LyXFont const & font, bool toggle, std::string & data);
/** Set \param font and \param toggle using \param data.
 *  If successful, returns true.
 */
bool string2font(std::string const & data, LyXFont & font, bool & toggle);
/** Returns the current freefont, encoded as a std::string to be passed to the
 *  frontends.
 */
std::string const freefont2string();
///
void put_selection_at(BufferView * bv, PosIterator const & cur,
		      int length, bool backwards);


/// what type of depth change to make
enum DEPTH_CHANGE {
	INC_DEPTH,
	DEC_DEPTH
};

/// Increase or decrease the nesting depth of the selected paragraph(s)
void changeDepth(BufferView *, LyXText *, DEPTH_CHANGE);

/// Returns whether something would be changed by changeDepth
bool changeDepthAllowed(BufferView *, LyXText *, DEPTH_CHANGE);

/// Returns the current font and depth as a message.
std::string const currentState(BufferView *);
/// replace selection with insertion
void replaceSelection(LyXText * lt);


}; // namespace bv_funcs

#endif
