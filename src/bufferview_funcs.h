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
 * Full author contact details are available in file CREDITS
 */

#ifndef BUFFERVIEW_FUNCS_H
#define BUFFERVIEW_FUNCS_H

#include "LString.h"

class BufferView;
class LyXFont;
class LyXText;

namespace bv_funcs {

/** Set \param data using \param font and \param toggle.
 *  If successful, returns true.
 */
bool font2string(LyXFont const & font, bool toggle, string & data);
/** Set \param font and \param toggle using \param data.
 *  If successful, returns true.
 */
bool string2font(string const & data, LyXFont & font, bool & toggle);
/** Returns the current freefont, encoded as a string to be passed to the
 *  frontends.
 */
string const freefont2string();
/** Set the freefont using the contents of \param data dispatched from
 *  the frontends and apply it at the current cursor location.
 */
void update_and_apply_freefont(BufferView * bv, string const & data);
/** Apply the contents of freefont at the current cursor location.
 */
void apply_freefont(BufferView * bv);

/// what type of depth change to make
enum DEPTH_CHANGE {
	INC_DEPTH,
	DEC_DEPTH
};

/**
 * Increase or decrease the nesting depth of the selected paragraph(s)
 * if test_only, don't change any depths. Returns whether something
 * (would have) changed
 */
extern bool changeDepth(BufferView *, LyXText *, DEPTH_CHANGE, bool test_only);

///
extern void emph(BufferView *);
///
extern void bold(BufferView *);
///
extern void noun(BufferView *);
///
extern void lang(BufferView *, string const &);
///
extern void number(BufferView *);
///
extern void tex(BufferView *);
///
extern void code(BufferView *);
///
extern void sans(BufferView *);
///
extern void roman(BufferView *);
///
extern void styleReset(BufferView *);
///
extern void underline(BufferView *);
///
extern void fontSize(BufferView *, string const &);
/// Returns the current font and depth as a message.
extern string const currentState(BufferView *);
///
extern void toggleAndShow(BufferView *, LyXFont const &,
			  bool toggleall = true);

}; // namespace bv_funcs

#endif
