/**
 * \file ControlERT.C
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

#include "ControlERT.h"

// sorry this is just a temporary hack we should include vspace.h! (Jug)
extern const char * stringFromUnit(int);


ControlERT::ControlERT(LyXView & lv, Dialogs & d)
	: ControlInset<InsetERT, ERTParams>(lv, d)
{}


void ControlERT::applyParamsToInset()
{
	inset()->status(bufferview(), params().status);
}


void ControlERT::applyParamsNoInset()
{
}


ERTParams const ControlERT::getParams(InsetERT const & inset)
{
	return ERTParams(inset);
}


ERTParams::ERTParams()
	: status(InsetERT::Collapsed)
{}


ERTParams::ERTParams(InsetERT const & inset)
	: status(inset.status())
{}


bool operator==(ERTParams const & p1, ERTParams const & p2)
{
	return (p1.status == p2.status);
}


bool operator!=(ERTParams const & p1, ERTParams const & p2)
{
	return !(p1 == p2);
}
