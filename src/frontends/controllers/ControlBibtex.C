// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2001 The LyX Team.
 *
 * ======================================================
 *
 * \file ControlBibtex.C
 * \author John Levon, moz@compsoc.man.ac.uk
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#include <algorithm>

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>
#include "ControlBibtex.h"
#include "Dialogs.h"
#include "LyXView.h"
#include "BufferView.h"

using SigC::slot;

ControlBibtex::ControlBibtex(LyXView & lv, Dialogs & d)
	: ControlCommand(lv, d)
{
	d_.showBibtex.connect(slot(this, &ControlBibtex::showInset));
}

void ControlBibtex::apply()
{
	view().apply();

	if (inset_ && params() != inset_->params()) {
		if (params().getContents() != inset_->params().getContents())
			lv_.view()->ChangeCitationsIfUnique(
				inset_->params().getContents(),
				params().getContents());

		inset_->setParams(params());
		lv_.view()->updateInset(inset_, true);

		// We need to do a redraw because the maximum
		// InsetBibKey width could have changed
		lv_.view()->redraw();
		lv_.view()->fitCursor(lv_.view()->getLyXText());
	}
}
