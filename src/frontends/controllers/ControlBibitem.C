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
		bufferview()->ChangeCitationsIfUnique(inset()->params().getContents(),
						    params().getContents());

	inset()->setParams(params());
	bufferview()->updateInset(inset(), true);

	// We need to do a redraw because the maximum
	// InsetBibKey width could have changed
#warning please check you mean repaint() not update(),
#warning and whether the repaint() is needed at all
	bufferview()->repaint();
	bufferview()->fitCursor();
}
