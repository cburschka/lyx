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

#include "ViewBase.h"
#include "ButtonControllerBase.h"
#include "ControlFloat.h"
#include "Dialogs.h"
#include "LyXView.h"
#include "buffer.h"
#include "BufferView.h"
#include "insets/insetfloat.h"

using std::vector;
using SigC::slot;


ControlFloat::ControlFloat(LyXView & lv, Dialogs & d)
	: ControlInset<InsetFloat, FloatParams>(lv, d)
{
	d_.showFloat.connect(slot(this, &ControlFloat::showInset));

	// We could have an extra method updateInset that calls
	// view().update() rather than view().show(), but I don't see why
	// it is really needed.
	//d_.updateFloat.connect(slot(this, &ControlFloat::showInset));
}


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
