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
 * \author Herbert Voss <voss@lyx.org>
 */

#include <algorithm>

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>
#include "ViewBase.h"
#include "ButtonControllerBase.h"
#include "ControlBibtex.h"
#include "Dialogs.h"
#include "LyXView.h"
#include "buffer.h"
#include "BufferView.h"
#include "lyxrc.h"
#include "helper_funcs.h"
#include "gettext.h"

using SigC::slot;
using std::pair;
using std::make_pair;

ControlBibtex::ControlBibtex(LyXView & lv, Dialogs & d)
	: ControlCommand(lv, d)
{
	d_.showBibtex.connect(slot(this, &ControlBibtex::showInset));
}

void ControlBibtex::applyParamsToInset()
{
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


void ControlBibtex::applyParamsNoInset()
{}


string const ControlBibtex::Browse(string const & in_name,
				   string const & title,
				   string const & pattern)
{
	pair<string, string> dir1(N_("Documents|#o#O"), string(lyxrc.document_path));
	return browseFile(&lv_, in_name, title, pattern, dir1);
}
