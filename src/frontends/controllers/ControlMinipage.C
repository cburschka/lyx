/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2001 The LyX Team.
 *
 * ======================================================
 *
 * \author Jürgen Vigna, jug@sad.it
 * \author Angus Leeming, a.leeming@ic.ac.uk
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ViewBase.h"
#include "ButtonControllerBase.h"
#include "ControlMinipage.h"
#include "Dialogs.h"
#include "LyXView.h"
#include "buffer.h"

using std::vector;
using SigC::slot;

// sorry this is just a temporary hack we should include vspace.h! (Jug)
extern const char * stringFromUnit(int);

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
	: pos(InsetMinipage::top)
{}

MinipageParams::MinipageParams(InsetMinipage const & inset)
	: width(inset.width()), pos(inset.pos())
{}

bool MinipageParams::operator==(MinipageParams const & o) const
{
	return (width == o.width && pos == o.pos);
}

bool MinipageParams::operator!=(MinipageParams const & o) const
{
	return !(*this == o);
}

namespace minipage {

vector<string> const getUnits()
{
	vector<string> units;
	const char * str;
	for(int i=0;(str=stringFromUnit(i));++i)
	    units.push_back(str);

	return units;
}
 
} // namespace minipage
