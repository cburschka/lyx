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
	EUFRAK_FAMILY,
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
	IGNORE_SERIES
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
	IGNORE_SHAPE
};

///
enum FontSize {
	///
	FONT_SIZE_TINY = 0,
	///
	FONT_SIZE_SCRIPT,
	///
	FONT_SIZE_FOOTNOTE,
	///
	FONT_SIZE_SMALL,
	///
	FONT_SIZE_NORMAL,
	///
	FONT_SIZE_LARGE,
	///
	FONT_SIZE_LARGER,
	///
	FONT_SIZE_LARGEST,
	///
	FONT_SIZE_HUGE,
	///
	FONT_SIZE_HUGER,
	///
	FONT_SIZE_INCREASE,
	///
	FONT_SIZE_DECREASE,
	///
	FONT_SIZE_INHERIT,
	///
	FONT_SIZE_IGNORE
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

} // namespace lyx
#endif
