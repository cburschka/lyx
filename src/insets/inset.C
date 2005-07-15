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

#include "BufferView.h"
#include "debug.h"
#include "gettext.h"
#include "lyxtext.h"
#include "LColor.h"
#include "metricsinfo.h"
#include "coordcache.h"

using std::string;


InsetOld::InsetOld()
	: //background_color_(LColor::inherit)
	  background_color_(LColor::background)
{}


InsetOld::InsetOld(InsetOld const & in)
	: InsetBase(in), name_(in.name_),
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


void InsetOld::setPosCache(PainterInfo const &, int x, int y) const
{
	//lyxerr << "InsetOld:: position cache to " << x << " " << y << std::endl;
	theCoords.insets().add(this, x, y);
}
