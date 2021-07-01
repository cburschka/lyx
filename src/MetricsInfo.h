// -*- C++ -*-
/**
 * \file MetricsInfo.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 * \author Stefan Schimanski
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef METRICSINFO_H
#define METRICSINFO_H

#include "Changes.h"
#include "ColorCode.h"
#include "FontInfo.h"

#include "support/strfwd.h"
#include "support/Changer.h"

#include "insets/Inset.h"


#include <string>


namespace lyx {

namespace frontend { class Painter; }
class BufferView;
class Length;
class MacroContext;


//
// This is the part common to MetricsInfo and PainterInfo
//
class MetricsBase {
public:
	///
	MetricsBase(BufferView * bv = 0, FontInfo font = FontInfo(),
	            int textwidth = 0);

	/// the current view
	BufferView * bv;
	/// current font
	FontInfo font;
	/// name of current font - mathed specific
	std::string fontname;
	/// This is the width available in pixels
	int textwidth;
	/// count wether the current mathdata is nested in macro(s)
	int macro_nesting;

	/// Temporarily change a full font.
	Changer changeFontSet(std::string const & name);
	/// Temporarily change the font to math if needed.
	Changer changeEnsureMath(Inset::mode_type mode = Inset::MATH_MODE);
	// Temporarily change to the style suitable for use in fractions
	Changer changeFrac();
	// Temporarily change to the style suitable for use in arrays
	// or to style suitable for smallmatrix when \c small is true.
	Changer changeArray(bool small = false);
	// Temporarily change the style to (script)script style
	Changer changeScript();
	///
	int solidLineThickness() const { return solid_line_thickness_; }
	///
	int solidLineOffset() const { return solid_line_offset_; }
	///
	int dottedLineThickness() const { return dotted_line_thickness_; }
	/** return the on-screen size of this length
	 *
	 *  This version of the function uses the current inset width as
	 *  width and the EM value of the current font.
	 */
	int inPixels(Length const & len) const;

private:
	int solid_line_thickness_;
	int solid_line_offset_;
	int dotted_line_thickness_;
};


//
// This contains a MetricsBase and information that's only relevant during
// the first phase of the two-phase draw
//
class MetricsInfo {
public:
	///
	MetricsInfo();
	///
	MetricsInfo(BufferView * bv, FontInfo font, int textwidth,
	            MacroContext const & mc, bool vm, bool tight_insets);

	///
	MetricsBase base;
	/// The context to resolve macros
	MacroContext const & macrocontext;
	/// Are we at the start of a paragraph (vertical mode)?
	bool vmode;
	/// if true, do not expand insets to max width artificially
	bool tight_insets;
};


//
// This contains a MetricsBase and information that's only relevant during
// the second phase of the two-phase draw
//
class PainterInfo {
public:
	///
	PainterInfo(BufferView * bv, frontend::Painter & pain);
	///
	void draw(int x, int y, char_type c);
	///
	void draw(int x, int y, docstring const & str);
	/// Determines the background color based on the
	/// selection state, the background color inherited from the parent inset
	/// and the inset's own background color (if one is specified).
	/// \param sel whether to take the selection state into account
	ColorCode backgroundColor(Inset const * inset = nullptr, bool sel = true) const;

	/// Determines the text color based on the intended color, the
	/// change tracking state and the selection state.
	/// \param color what the color should be by default
	Color textColor(Color const & color) const;

	///
	MetricsBase base;
	///
	frontend::Painter & pain;
	/// Whether the text at this point is right-to-left (for insets)
	bool ltr_pos;
	/// The change the parent is part of (change tracking)
	Change change;
	/// Whether the parent is selected as a whole
	bool selected;
	/// Whether the left/right margins are selected
	bool selected_left, selected_right;
	/// Whether the spell checker is enabled for the parent
	bool do_spellcheck;
	/// True when it can be assumed that the screen has been cleared
	bool full_repaint;
	/// Current background color
	ColorCode background_color;
	/// The left and right position of current line (inside margins).
	/// Useful for drawing display math numbering
	int leftx, rightx;
};

class TextMetricsInfo {};

} // namespace lyx

#endif
