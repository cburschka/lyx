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

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlBibtex.h"
#include "BufferView.h"
#include "lyxrc.h"
#include "helper_funcs.h"
#include "gettext.h"

#include "frontends/LyXView.h"

using std::pair;


ControlBibtex::ControlBibtex(LyXView & lv, Dialogs & d)
	: ControlCommand(lv, d)
{}


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
	pair<string, string> dir1(_("Documents|#o#O"), string(lyxrc.document_path));
	return browseRelFile(&lv_, in_name, lv_.buffer()->filePath(),
			     title, pattern, dir1);
}
