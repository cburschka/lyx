/**
 * \file ControlMinipage.C
 * Read the file COPYING
 *
 * \author Jürgen Vigna
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlMinipage.h"
#include "BufferView.h"


ControlMinipage::ControlMinipage(LyXView & lv, Dialogs & d)
	: ControlInset<InsetMinipage, MinipageParams>(lv, d)
{}


void ControlMinipage::applyParamsToInset()
{
	inset()->pageWidth(params().pageWidth);
	inset()->pos(params().pos);

	bufferview()->updateInset(inset(), true);
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
	: pageWidth(inset.pageWidth()), pos(inset.pos())
{}


bool operator==(MinipageParams const & p1, MinipageParams const & p2)
{
	return (p1.pageWidth == p2.pageWidth && p1.pos == p2.pos);
}


bool operator!=(MinipageParams const & p1, MinipageParams const & p2)
{
	return !(p1 == p2);
}
