// -*- C++ -*-
/**
 * \file src/FontEnums.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author Angus Leeming
 * \author Dekel Tsur
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef FONT_ENUMS_H
#define FONT_ENUMS_H

namespace lyx {

/** The value INHERIT_* means that the font attribute is
inherited from the layout. In the case of layout fonts, the
attribute is inherited from the default font.
The value IGNORE_* is used with Font::update() when the
attribute should not be changed.
*/
enum FontFamily {
	///
	ROMAN_FAMILY = 0,
	///
	SANS_FAMILY,
	///
	TYPEWRITER_FAMILY,
	///
	SYMBOL_FAMILY,
	///
	CMR_FAMILY,
	///
	CMSY_FAMILY,
	///
	CMM_FAMILY,
	///
	CMEX_FAMILY,
	///
	MSA_FAMILY,
	///
	MSB_FAMILY,
	///
	DS_FAMILY,
	///
	EUFRAK_FAMILY,
	///
	RSFS_FAMILY,
	///
	STMARY_FAMILY,
	///
	WASY_FAMILY,
	///
	ESINT_FAMILY,
	///
	INHERIT_FAMILY,
	///
	IGNORE_FAMILY,
	///
	NUM_FAMILIES = INHERIT_FAMILY
};

///
enum FontSeries {
	///
	MEDIUM_SERIES = 0,
	///
	BOLD_SERIES,
	///
	INHERIT_SERIES,
	///
	IGNORE_SERIES,
	///
	NUM_SERIES = INHERIT_SERIES
};

///
enum FontShape {
	///
	UP_SHAPE = 0,
	///
	ITALIC_SHAPE,
	///
	SLANTED_SHAPE,
	///
	SMALLCAPS_SHAPE,
	///
	INHERIT_SHAPE,
	///
	IGNORE_SHAPE,
	///
	NUM_SHAPE = INHERIT_SHAPE
};

///
enum FontSize {
	///
	TINY_SIZE = 0,
	///
	SCRIPT_SIZE,
	///
	FOOTNOTE_SIZE,
	///
	SMALL_SIZE,
	///
	NORMAL_SIZE,
	///
	LARGE_SIZE,
	///
	LARGER_SIZE,
	///
	LARGEST_SIZE,
	///
	HUGE_SIZE,
	///
	HUGER_SIZE,
	///
	INCREASE_SIZE,
	///
	DECREASE_SIZE,
	///
	INHERIT_SIZE,
	///
	IGNORE_SIZE,
	///
	NUM_SIZE = INCREASE_SIZE
};

/// Used for emph, underbar, noun and latex toggles
enum FontState {
	///
	FONT_OFF,
	///
	FONT_ON,
	///
	FONT_TOGGLE,
	///
	FONT_INHERIT,
	///
	FONT_IGNORE
};


/// Math styles
enum MathStyle {
	///
	SCRIPTSCRIPT_STYLE = 0,
	///
	SCRIPT_STYLE,
	///
	TEXT_STYLE,
	///
	DISPLAY_STYLE,
	///
	INHERIT_STYLE,
	///
	IGNORE_STYLE,
	/// the text and display fonts are the same
	NUM_STYLE = DISPLAY_STYLE
};


} // namespace lyx
#endif
