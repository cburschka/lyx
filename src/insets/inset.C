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
#include "metricsinfo.h"

using std::string;


InsetOld::InsetOld()
	: InsetBase(),
	  xo_(0), yo_(0), scx(0),
	  //background_color_(LColor::inherit)
	  background_color_(LColor::background)
{}


InsetOld::InsetOld(InsetOld const & in)
	: InsetBase(),
	  xo_(0), yo_(0), scx(0), name_(in.name_),
	  background_color_(in.background_color_)
{}


void InsetOld::setBackgroundColor(LColor_color color)
{
	background_color_ = color;
}


LColor_color InsetOld::backgroundColor() const
{
	return LColor::color(background_color_);
}


bool InsetOld::forceDefaultParagraphs(InsetBase const *) const
{
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


int InsetOld::scroll(bool) const
{
	return scx;
}


void InsetOld::setPosCache(PainterInfo const & pi, int x, int y) const
{
	//lyxerr << "InsetOld:: position cache to " << x << " " << y << std::endl;
	xo_ = x;
	yo_ = y + pi.base.bv->top_y();
}
