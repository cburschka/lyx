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

using SigC::slot;

ControlMinipage::ControlMinipage(LyXView & lv, Dialogs & d)
	: ControlInset<InsetMinipage, MinipageParams>(lv, d)
{
	d_.showMinipage.connect(slot(this, &ControlMinipage::showInset));
	d_.updateMinipage.connect(slot(this, &ControlMinipage::showInset));
}


void ControlMinipage::applyParamsToInset()
{
	inset()->width(params().width);
	inset()->widthp(params().widthp);
	inset()->pos(params().pos);
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
