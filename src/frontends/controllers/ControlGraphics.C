/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2001 The LyX Team.
 *
 * ======================================================
 *
 * \file ControlGraphics.C
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#include <config.h> 

#ifdef __GNUG__
#pragma implementation
#endif 

#include "ControlGraphics.h"
#include "buffer.h"
#include "Dialogs.h"
#include "lyxfunc.h"
#include "LyXView.h"

#include "insets/insetgraphics.h"

ControlGraphics::ControlGraphics(LyXView & lv, Dialogs & d)
	: ControlInset<InsetGraphics, InsetGraphicsParams>(lv, d)
{
	d_.showGraphics.connect(SigC::slot(this, &ControlGraphics::showInset));
}


LyXView * ControlGraphics::lv() const
{
        return &lv_;
}


InsetGraphicsParams const ControlGraphics::getParams(string const &)
{
	return InsetGraphicsParams();
}


InsetGraphicsParams const
ControlGraphics::getParams(InsetGraphics const & inset)
{
	return inset.getParams();
}


void ControlGraphics::applyParamsToInset()
{
	// Set the parameters in the inset, it also returns true if the new
	// parameters are different from what was in the inset already.
	bool changed = inset()->setParams(params());
	// Tell LyX we've got a change, and mark the document dirty,
	// if it changed.
	lv_.view()->updateInset(inset(), changed);
}


void ControlGraphics::applyParamsNoInset()
{}
