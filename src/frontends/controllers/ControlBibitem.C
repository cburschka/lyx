/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2001 The LyX Team.
 *
 * ======================================================
 *
 * \file ControlBibitem.C
 * \author John Levon, moz@compsoc.man.ac.uk
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlBibitem.h"
#include "BufferView.h"


ControlBibitem::ControlBibitem(LyXView & lv, Dialogs & d)
	: ControlCommand(lv, d)
{}


void ControlBibitem::applyParamsToInset()
{
	// FIXME:
	// confirm, is this only necessary for FormBibTeX ???
	if (params().getContents() != inset()->params().getContents())
		lv_.view()->ChangeCitationsIfUnique(inset()->params().getContents(),
						    params().getContents());

	inset()->setParams(params());
	lv_.view()->updateInset(inset(), true);

	// We need to do a redraw because the maximum
	// InsetBibKey width could have changed
	lv_.view()->redraw();
	lv_.view()->fitCursor();
}
