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
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "inset.h"
#include "updatableinset.h"

#include "BufferView.h"
#include "debug.h"
#include "gettext.h"
#include "lyxtext.h"
#include "LColor.h"


using std::string;


InsetOld::InsetOld()
	: InsetBase(),
	  xo_(0), yo_(0), scx(0), owner_(0),
	  //background_color_(LColor::inherit)
	  background_color_(LColor::background)
{}


InsetOld::InsetOld(InsetOld const & in)
	: InsetBase(),
	  xo_(0), yo_(0), scx(0), owner_(0), name_(in.name_),
	  background_color_(in.background_color_)
{}


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


string const InsetOld::editMessage() const
{
	return _("Opened inset");
}


void InsetOld::setBackgroundColor(LColor_color color)
{
	background_color_ = color;
}


LColor_color InsetOld::backgroundColor() const
{
	return LColor::color(background_color_);
}


bool InsetOld::forceDefaultParagraphs(InsetOld const * inset) const
{
	if (owner())
		return owner()->forceDefaultParagraphs(inset);
	return false;
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


int InsetOld::scroll(bool recursive) const
{
	if (!recursive || !owner_)
		return scx;
	return 0;
}


int InsetOld::getCell(int x, int y) const
{
	for (int i = 0, n = numParagraphs(); i < n; ++i) {
		LyXText * text = getText(i);
		//lyxerr << "### text: " << text << " i: " << i
		//	<< " xo: " << text->xo_ << "..." << text->xo_ + text->width
		//	<< " yo: " << text->yo_ 
		//	<< " yo: " << text->yo_ - text->ascent() << "..."
		//		<<  text->yo_ + text->descent()
		//	<< std::endl;	
		if (x >= text->xo_
				&& x <= text->xo_ + text->width
				&& y >= text->yo_ 
				&& y <= text->yo_ + text->height)
		{
			lyxerr << "### found text # " << i << std::endl;	
			return i;
		}
	}
	return -1;
}


bool isEditableInset(InsetOld const * i)
{
	return i && i->editable();
}


bool isHighlyEditableInset(InsetOld const * i)
{
	return i && i->editable() == InsetOld::HIGHLY_EDITABLE;
}


