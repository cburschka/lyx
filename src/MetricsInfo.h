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


#include <string>


namespace lyx {

namespace frontend { class Painter; }
class BufferView;
class Inset;
class MacroContext;


/// Standard Sizes (mode styles)
/// note: These values are hard-coded in changeStyle
enum Styles {
	///
	LM_ST_DISPLAY = 0,
	///
	LM_ST_TEXT,
	///
	LM_ST_SCRIPT,
	///
	LM_ST_SCRIPTSCRIPT
};


//
// This is the part common to MetricsInfo and PainterInfo
//
class MetricsBase {
public:
	///
	MetricsBase();
	///
	MetricsBase(BufferView * bv, FontInfo const & font, int textwidth);

	/// the current view
	BufferView * bv;
	/// current font
	FontInfo font;
	/// current math style (display/text/script/..)
	Styles style;
	/// name of current font - mathed specific
	std::string fontname;
	/// This is the width available in pixels
	int textwidth;

	/// Temporarily change a full font.
	Changer changeFontSet(docstring const & font, bool cond = true);
	Changer changeFontSet(char const * font, bool cond = true);
	/// Temporarily change the font size and the math style.
	Changer changeStyle(Styles style, bool cond = true);
	// Temporarily change to the style suitable for use in fractions
	Changer changeFrac(bool cond = true);
	// Temporarily change the style to (script)script style
	Changer changeScript(bool cond = true);
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
	MetricsInfo(BufferView * bv, FontInfo const & font, int textwidth, MacroContext const & mc);

	///
	MetricsBase base;
	/// The context to resolve macros
	MacroContext const & macrocontext;
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
	/// Determines the background color for the specified inset based on the
	/// selection state, the background color inherited from the parent inset 
	/// and the inset's own background color.
	/// \param sel whether to take the selection state into account
	ColorCode backgroundColor(Inset const * inset, bool sel = true) const;

	/// Determines the text color based on the intended color, the
	/// change tracking state and the selection state. 
	/// \param color what the color should be by default
	Color textColor(Color const & color) const;

	///
	MetricsBase base;
	///
	frontend::Painter & pain;
	/// Whether the text at this point is right-to-left (for InsetNewline)
	bool ltr_pos;
	/// The change the parent is part of (change tracking)
	Change change_;
	/// Whether the parent is selected as a whole
	bool selected;
	/// Whether the spell checker is enabled for the parent
	bool do_spellcheck;
	///
	bool full_repaint;
	/// Current background color
	ColorCode background_color;
};

class TextMetricsInfo {};

} // namespace lyx

#endif
