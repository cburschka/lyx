/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2001 The LyX Team.
 *
 * ======================================================
 *
 * \author Juergen Vigna, jug@sad.it
 * \author Angus Leeming, a.leeming@ic.ac.uk
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlMinipage.h"
#include "Dialogs.h"
#include "LyXView.h"
#include "buffer.h"

using std::vector;
using SigC::slot;

ControlMinipage::ControlMinipage(LyXView & lv, Dialogs & d)
	: ControlInset<InsetMinipage, MinipageParams>(lv, d)
{
	d_.showMinipage.connect(slot(this, &ControlMinipage::showInset));

	// We could have an extra method updateInset that calls
	// view().update() rather than view().show(), but I don't see why
	// it is really needed.
	d_.updateMinipage.connect(slot(this, &ControlMinipage::showInset));
}


void ControlMinipage::applyParamsToInset()
{
	inset()->width(params().width);
	inset()->widthp(params().widthp);
	inset()->pos(params().pos);

	lv_.view()->updateInset(inset(), true);
}


void ControlMinipage::applyParamsNoInset()
{
}


MinipageParams const ControlMinipage::getParams(InsetMinipage const & inset)
{
	return MinipageParams(inset);
}


MinipageParams::MinipageParams()
	: widthp(0), pos(InsetMinipage::top)
{}

MinipageParams::MinipageParams(InsetMinipage const & inset)
	: width(inset.width()), widthp(inset.widthp()), pos(inset.pos())
{}

bool MinipageParams::operator==(MinipageParams const & o) const
{
	return (width == o.width && widthp == o.widthp && pos == o.pos);
}

bool MinipageParams::operator!=(MinipageParams const & o) const
{
	return !(*this == o);
}

namespace minipage {

vector<string> const getUnits()
{
	vector<string> units;
	units.push_back("mm");
	units.push_back("in");
	units.push_back("em");
	units.push_back("\%");
	units.push_back("\%c");
	units.push_back("\%l");
	units.push_back("\%p");

	return units;
}
 
} // namespace minipage

