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
#include "metricsinfo.h"

#include "insets/updatableinset.h"

#include "frontends/Painter.h"
#include "frontends/mouse_state.h"

#include "support/lstrings.h"


#include "debug.h"

// Initialization of the counter for the inset id's,
unsigned int InsetOld::inset_id = 0;

InsetOld::InsetOld()
	: InsetBase(),
	top_x(0), top_baseline(0), scx(0),
	id_(inset_id++), owner_(0), par_owner_(0),
	background_color_(LColor::inherit)
{}


InsetOld::InsetOld(InsetOld const & in)
	: InsetBase(),
	top_x(0), top_baseline(0), scx(0), id_(in.id_), owner_(0),
	name_(in.name_), background_color_(in.background_color_)
{
}


bool InsetOld::directWrite() const
{
	return false;
}


InsetOld::EDITABLE InsetOld::editable() const
{
	return NOT_EDITABLE;
}


bool InsetOld::autoDelete() const
{
	return false;
}


#if 0
LyXFont const InsetOld::convertFont(LyXFont const & font) const
{
#if 1
	return font;
#else
	return LyXFont(font);
#endif
}
#endif


string const InsetOld::editMessage() const
{
	return _("Opened inset");
}


LyXText * InsetOld::getLyXText(BufferView const * bv, bool /*recursive*/) const
{
	if (owner())
		return owner()->getLyXText(bv, false);
	else
		return bv->text;
}


void InsetOld::setBackgroundColor(LColor::color color)
{
	background_color_ = color;
}


LColor::color InsetOld::backgroundColor() const
{
	if (background_color_ == LColor::inherit) {
		if (owner())
			return owner()->backgroundColor();
		else
			return LColor::background;
	} else
		return background_color_;
}


int InsetOld::id() const
{
	return id_;
}

void InsetOld::id(int id_arg)
{
	id_ = id_arg;
}

void InsetOld::setFont(BufferView *, LyXFont const &, bool, bool)
{}


bool InsetOld::forceDefaultParagraphs(InsetOld const * inset) const
{
	if (owner())
		return owner()->forceDefaultParagraphs(inset);
	return false;
}

int InsetOld::latexTextWidth(BufferView * bv) const
{
	if (owner())
		return (owner()->latexTextWidth(bv));
	return bv->workWidth();
}


int InsetOld::ascent() const
{
	return dim_.asc;
}


int InsetOld::descent() const
{
	return dim_.des;
}


int InsetOld::width() const
{
	return dim_.wid;
}


bool InsetOld::insetAllowed(InsetOld * in) const
{
	return insetAllowed(in->lyxCode());
}


bool InsetOld::checkInsertChar(LyXFont &)
{
	return false;
}


bool isEditableInset(InsetOld const * i)
{
	return i && i->editable();
}


bool isHighlyEditableInset(InsetOld const * i)
{
	return i && i->editable() == InsetOld::HIGHLY_EDITABLE;
}

