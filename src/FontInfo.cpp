/**
 * \file src/FontInfo.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author Angus Leeming
 * \author André Pönitz
 * \author Dekel Tsur
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "FontInfo.h"

#include "support/debug.h"

using namespace std;

namespace lyx {

FontInfo const sane_font(
	ROMAN_FAMILY,
	MEDIUM_SERIES,
	UP_SHAPE,
	FONT_SIZE_NORMAL,
	Color_none,
	Color_background,
	FONT_OFF,
	FONT_OFF,
	FONT_OFF,
	FONT_OFF);

FontInfo const inherit_font(
	INHERIT_FAMILY,
	INHERIT_SERIES,
	INHERIT_SHAPE,
	FONT_SIZE_INHERIT,
	Color_inherit,
	Color_inherit,
	FONT_INHERIT,
	FONT_INHERIT,
	FONT_INHERIT,
	FONT_OFF);

FontInfo const ignore_font(
	IGNORE_FAMILY,
	IGNORE_SERIES,
	IGNORE_SHAPE,
	FONT_SIZE_IGNORE,
	Color_ignore,
	Color_ignore,
	FONT_IGNORE,
	FONT_IGNORE,
	FONT_IGNORE,
	FONT_IGNORE);


FontInfo::FontInfo()
{
	*this = sane_font;
}

/// Decreases font size_ by one
FontInfo & FontInfo::decSize()
{
	switch (size_) {
	case FONT_SIZE_HUGER:        size_ = FONT_SIZE_HUGE;     break;
	case FONT_SIZE_HUGE:         size_ = FONT_SIZE_LARGEST;  break;
	case FONT_SIZE_LARGEST:      size_ = FONT_SIZE_LARGER;   break;
	case FONT_SIZE_LARGER:       size_ = FONT_SIZE_LARGE;    break;
	case FONT_SIZE_LARGE:        size_ = FONT_SIZE_NORMAL;   break;
	case FONT_SIZE_NORMAL:       size_ = FONT_SIZE_SMALL;    break;
	case FONT_SIZE_SMALL:        size_ = FONT_SIZE_FOOTNOTE; break;
	case FONT_SIZE_FOOTNOTE:     size_ = FONT_SIZE_SCRIPT;   break;
	case FONT_SIZE_SCRIPT:       size_ = FONT_SIZE_TINY;     break;
	case FONT_SIZE_TINY:         break;
	case FONT_SIZE_INCREASE:
		LYXERR0("Can't FontInfo::decSize on FONT_SIZE_INCREASE");
		break;
	case FONT_SIZE_DECREASE:
		LYXERR0("Can't FontInfo::decSize on FONT_SIZE_DECREASE");
		break;
	case FONT_SIZE_INHERIT:
		LYXERR0("Can't FontInfo::decSize on FONT_SIZE_INHERIT");
		break;
	case FONT_SIZE_IGNORE:
		LYXERR0("Can't FontInfo::decSize on FONT_SIZE_IGNORE");
		break;
	}
	return *this;
}


/// Increases font size_ by one
FontInfo & FontInfo::incSize()
{
	switch (size_) {
	case FONT_SIZE_HUGER:	break;
	case FONT_SIZE_HUGE:         size_ = FONT_SIZE_HUGER;    break;
	case FONT_SIZE_LARGEST:      size_ = FONT_SIZE_HUGE;     break;
	case FONT_SIZE_LARGER:       size_ = FONT_SIZE_LARGEST;  break;
	case FONT_SIZE_LARGE:        size_ = FONT_SIZE_LARGER;   break;
	case FONT_SIZE_NORMAL:       size_ = FONT_SIZE_LARGE;    break;
	case FONT_SIZE_SMALL:        size_ = FONT_SIZE_NORMAL;   break;
	case FONT_SIZE_FOOTNOTE:     size_ = FONT_SIZE_SMALL;    break;
	case FONT_SIZE_SCRIPT:       size_ = FONT_SIZE_FOOTNOTE; break;
	case FONT_SIZE_TINY:         size_ = FONT_SIZE_SCRIPT;   break;
	case FONT_SIZE_INCREASE:
		LYXERR0("Can't FontInfo::incSize on FONT_SIZE_INCREASE");
		break;
	case FONT_SIZE_DECREASE:
		LYXERR0("Can't FontInfo::incSize on FONT_SIZE_DECREASE");
		break;
	case FONT_SIZE_INHERIT:
		LYXERR0("Can't FontInfo::incSize on FONT_SIZE_INHERIT");
		break;
	case FONT_SIZE_IGNORE:
		LYXERR0("Can't FontInfo::incSize on FONT_SIZE_IGNORE");
		break;
	}
	return *this;
}


/// Reduce font to fall back to template where possible
void FontInfo::reduce(FontInfo const & tmplt)
{
	if (family_ == tmplt.family_)
		family_ = INHERIT_FAMILY;
	if (series_ == tmplt.series_)
		series_ = INHERIT_SERIES;
	if (shape_ == tmplt.shape_)
		shape_ = INHERIT_SHAPE;
	if (size_ == tmplt.size_)
		size_ = FONT_SIZE_INHERIT;
	if (emph_ == tmplt.emph_)
		emph_ = FONT_INHERIT;
	if (underbar_ == tmplt.underbar_)
		underbar_ = FONT_INHERIT;
	if (noun_ == tmplt.noun_)
		noun_ = FONT_INHERIT;
	if (color_ == tmplt.color_)
		color_ = Color_inherit;
	if (background_ == tmplt.background_)
		background_ = Color_inherit;
}


/// Realize font from a template
FontInfo & FontInfo::realize(FontInfo const & tmplt)
{
	if ((*this) == inherit_font) {
		operator=(tmplt);
		return *this;
	}

	if (family_ == INHERIT_FAMILY)
		family_ = tmplt.family_;

	if (series_ == INHERIT_SERIES)
		series_ = tmplt.series_;

	if (shape_ == INHERIT_SHAPE)
		shape_ = tmplt.shape_;

	if (size_ == FONT_SIZE_INHERIT)
		size_ = tmplt.size_;

	if (emph_ == FONT_INHERIT)
		emph_ = tmplt.emph_;

	if (underbar_ == FONT_INHERIT)
		underbar_ = tmplt.underbar_;

	if (noun_ == FONT_INHERIT)
		noun_ = tmplt.noun_;

	if (color_ == Color_inherit)
		color_ = tmplt.color_;

	if (background_ == Color_inherit)
		background_ = tmplt.background_;

	return *this;
}


/// Updates a misc setting according to request
static FontState setMisc(FontState newfont,
	FontState org)
{
	if (newfont == FONT_TOGGLE) {
		if (org == FONT_ON)
			return FONT_OFF;
		else if (org == FONT_OFF)
			return FONT_ON;
		else {
			LYXERR0("Font::setMisc: Need state"
				" FONT_ON or FONT_OFF to toggle. Setting to FONT_ON");
			return FONT_ON;
		}
	} else if (newfont == FONT_IGNORE)
		return org;
	else
		return newfont;
}

/// Updates font settings according to request
void FontInfo::update(FontInfo const & newfont, bool toggleall)
{
	if (newfont.family_ == family_ && toggleall)
		setFamily(INHERIT_FAMILY); // toggle 'back'
	else if (newfont.family_ != IGNORE_FAMILY)
		setFamily(newfont.family_);
	// else it's IGNORE_SHAPE

	// "Old" behaviour: "Setting" bold will toggle bold on/off.
	switch (newfont.series_) {
	case BOLD_SERIES:
		// We toggle...
		if (series_ == BOLD_SERIES && toggleall)
			setSeries(MEDIUM_SERIES);
		else
			setSeries(BOLD_SERIES);
		break;
	case MEDIUM_SERIES:
	case INHERIT_SERIES:
		setSeries(newfont.series_);
		break;
	case IGNORE_SERIES:
		break;
	}

	if (newfont.shape_ == shape_ && toggleall)
		shape_ = INHERIT_SHAPE; // toggle 'back'
	else if (newfont.shape_ != IGNORE_SHAPE)
		shape_ = newfont.shape_;
	// else it's IGNORE_SHAPE

	if (newfont.size_ != FONT_SIZE_IGNORE) {
		if (newfont.size_ == FONT_SIZE_INCREASE)
			incSize();
		else if (newfont.size_ == FONT_SIZE_DECREASE)
			decSize();
		else
			size_ = newfont.size_;
	}

	setEmph(setMisc(newfont.emph_, emph_));
	setUnderbar(setMisc(newfont.underbar_, underbar_));
	setNoun(setMisc(newfont.noun_, noun_));
	setNumber(setMisc(newfont.number_, number_));

	if (newfont.color_ == color_ && toggleall)
		setColor(Color_inherit); // toggle 'back'
	else if (newfont.color_ != Color_ignore)
		setColor(newfont.color_);

	if (newfont.background_ == background_ && toggleall)
		setBackground(Color_inherit); // toggle 'back'
	else if (newfont.background_ != Color_ignore)
		setBackground(newfont.background_);
}

/// Is font resolved?
bool FontInfo::resolved() const
{
	return (family_ != INHERIT_FAMILY && series_ != INHERIT_SERIES
		&& shape_ != INHERIT_SHAPE && size_ != FONT_SIZE_INHERIT
		&& emph_ != FONT_INHERIT && underbar_ != FONT_INHERIT
		&& noun_ != FONT_INHERIT
		&& color_ != Color_inherit
		&& background_ != Color_inherit);
}


Color FontInfo::realColor() const
{
	if (draw_color_ != Color_none)
		return draw_color_;
	if (color_ == Color_none)
		return Color_foreground;
	return color_;
}

} // namespace lyx
