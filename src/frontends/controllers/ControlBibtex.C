/**
 * \file ControlBibtex.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Angus Leeming
 * \author Herbert Voss
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlBibtex.h"
#include "buffer.h"
#include "BufferView.h"
#include "lyxrc.h"
#include "helper_funcs.h"
#include "tex_helpers.h"
#include "gettext.h"
#include "support/lstrings.h"


using std::pair;


ControlBibtex::ControlBibtex(LyXView & lv, Dialogs & d)
	: ControlCommand(lv, d)
{}



void ControlBibtex::applyParamsToInset()
{
	if (params().getContents() != inset()->params().getContents())
		bufferview()->ChangeCitationsIfUnique(inset()->params().getContents(),
						    params().getContents());

	inset()->setParams(params());
	bufferview()->updateInset(inset(), true);

	// We need to do a redraw because the maximum
	// InsetBibKey width could have changed
#warning are you sure you need this repaint() ?
	bufferview()->repaint();
	bufferview()->fitCursor();
}


void ControlBibtex::applyParamsNoInset()
{}


string const ControlBibtex::Browse(string const & in_name,
				   string const & title,
				   string const & pattern)
{
	pair<string, string> dir1(_("Documents|#o#O"),
				  string(lyxrc.document_path));
	return browseRelFile(&lv_, in_name, buffer()->filePath(),
			     title, pattern, dir1);
}


string const ControlBibtex::getBibStyles() const
{
	string list = getTexFileList("bstFiles.lst", false);
	// test, if we have a valid list, otherwise run rescan
	if (list.empty()) {
		rescanBibStyles();
		list = getTexFileList("bstFiles.lst", false);
	}
	return list;
}


void ControlBibtex::rescanBibStyles() const
{
	rescanTexStyles();
}
