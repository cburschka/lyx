/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2001 The LyX Team.
 *
 * ======================================================
 *
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlFloat.h"


ControlFloat::ControlFloat(LyXView & lv, Dialogs & d)
	: ControlInset<InsetFloat, FloatParams>(lv, d)
{}


void ControlFloat::applyParamsToInset()
{
	inset()->placement(params().placement);
}


void ControlFloat::applyParamsNoInset()
{}


FloatParams const ControlFloat::getParams(InsetFloat const & inset)
{
	return FloatParams(inset);
}


FloatParams::FloatParams()
	: placement("htbp"),
	  allow_here_definitely(true)
{}


FloatParams::FloatParams(InsetFloat const & inset)
	: placement(inset.placement()),
	  allow_here_definitely(!inset.wide())
{}
