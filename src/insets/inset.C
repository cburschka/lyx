/**
 * \file inset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author Jürgen Vigna
 * \author Lars Gullik Bjønnes
 * \author Matthias Ettrich
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "inset.h"
#include "BufferView.h"
#include "funcrequest.h"
#include "gettext.h"
#include "lyxfont.h"
#include "lyxtext.h"
#include "dimension.h"

#include "frontends/Painter.h"
#include "frontends/mouse_state.h"

#include "support/lstrings.h"

using std::endl;


// Initialization of the counter for the inset id's,
unsigned int Inset::inset_id = 0;

Inset::Inset()
	: InsetBase(),
	top_x(0), top_baseline(0), scx(0),
	id_(inset_id++), owner_(0), par_owner_(0),
	background_color_(LColor::inherit)
{}


Inset::Inset(Inset const & in, bool same_id)
	: InsetBase(),
	top_x(0), top_baseline(0), scx(0), owner_(0),
	name_(in.name_), background_color_(in.background_color_)
{
	if (same_id)
		id_ = in.id();
	else
		id_ = inset_id++;
}


bool Inset::directWrite() const
{
	return false;
}


Inset::EDITABLE Inset::editable() const
{
	return NOT_EDITABLE;
}


void Inset::validate(LaTeXFeatures &) const
{}


bool Inset::autoDelete() const
{
	return false;
}


#if 0
LyXFont const Inset::convertFont(LyXFont const & font) const
{
#if 1
	return font;
#else
	return LyXFont(font);
#endif
}
#endif


string const Inset::editMessage() const
{
	return _("Opened inset");
}


LyXText * Inset::getLyXText(BufferView const * bv, bool const) const
{
	if (owner())
		return owner()->getLyXText(bv, false);
	else
		return bv->text;
}


void Inset::setBackgroundColor(LColor::color color)
{
	background_color_ = color;
}


LColor::color Inset::backgroundColor() const
{
	if (background_color_ == LColor::inherit) {
		if (owner())
			return owner()->backgroundColor();
		else
			return LColor::background;
	} else
		return background_color_;
}


int Inset::id() const
{
	return id_;
}

void Inset::id(int id_arg)
{
	id_ = id_arg;
}

void Inset::setFont(BufferView *, LyXFont const &, bool, bool)
{}


bool Inset::forceDefaultParagraphs(Inset const * inset) const
{
	if (owner())
		return owner()->forceDefaultParagraphs(inset);
	return false;
}

int Inset::latexTextWidth(BufferView * bv) const
{
	if (owner())
		return (owner()->latexTextWidth(bv));
	return bv->workWidth();
}


int Inset::ascent(BufferView * bv, LyXFont const & font) const
{
	Dimension dim;
	dimension(bv, font, dim);
	return dim.ascent();
}


int Inset::descent(BufferView * bv, LyXFont const & font) const
{
	Dimension dim;
	dimension(bv, font, dim);
	return dim.descent();
}


int Inset::width(BufferView * bv, LyXFont const & font) const
{
	Dimension dim;
	dimension(bv, font, dim);
	return dim.width();
}

