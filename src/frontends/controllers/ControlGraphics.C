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
 * \author Herbert Voss <voss@perce.de>
 */

#include <config.h>
#include <fstream>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ViewBase.h"
#include "ButtonControllerBase.h"
#include "ControlGraphics.h"
#include "ControlInset.tmpl"
#include "buffer.h"
#include "BufferView.h"
#include "Dialogs.h"
#include "LyXView.h"
#include "gettext.h"
#include "lyxrc.h"

#include "insets/insetgraphics.h"
#include "insets/insetgraphicsParams.h" // need operator!=()

#include "support/FileInfo.h"  // for FileInfo
#include "helper_funcs.h"
#include "support/lstrings.h"
#include "support/filetools.h" // for AddName, zippedFile

using std::pair;
using std::make_pair;
using std::ifstream;

ControlGraphics::ControlGraphics(LyXView & lv, Dialogs & d)
	: ControlInset<InsetGraphics, InsetGraphicsParams>(lv, d)
{
	d_.showGraphics.connect(SigC::slot(this, &ControlGraphics::showInset));
}


InsetGraphicsParams const ControlGraphics::getParams(string const &)
{
	return InsetGraphicsParams();
}


InsetGraphicsParams const
ControlGraphics::getParams(InsetGraphics const & inset)
{
	return inset.params();
}


void ControlGraphics::applyParamsToInset()
{
	// Set the parameters in the inset, it also returns true if the new
	// parameters are different from what was in the inset already.
	bool changed = inset()->setParams(params(), lv_.buffer()->filePath());
	// Tell LyX we've got a change, and mark the document dirty,
	// if it changed.
	lv_.view()->updateInset(inset(), changed);
}


void ControlGraphics::applyParamsNoInset()
{}


// We need these in the file browser.
extern string system_lyxdir;
extern string user_lyxdir;


string const ControlGraphics::Browse(string const & in_name)
{
	string const title = _("Select graphics file");
	// FIXME: currently we need the second '|' to prevent mis-interpretation
	string const pattern = "*.(ps|eps|png|jpeg|jpg|gif|gz)|";

	// Does user clipart directory exist?
	string clipdir = AddName (user_lyxdir, "clipart");
	FileInfo fileInfo(clipdir);
	if (!(fileInfo.isOK() && fileInfo.isDir()))
		// No - bail out to system clipart directory
		clipdir = AddName (system_lyxdir, "clipart");
	pair<string, string> dir1(_("Clipart|#C#c"), clipdir);
	pair<string, string> dir2(_("Documents|#o#O"), string(lyxrc.document_path));
	// Show the file browser dialog
	return browseRelFile(&lv_, in_name, lv_.buffer()->filePath(),
			     title, pattern, dir1, dir2);
}


string const ControlGraphics::readBB(string const & file)
{
	return readBB_from_PSFile(MakeAbsPath(file, lv_.buffer()->filePath()));
}


bool ControlGraphics::isFilenameValid(string const & fname) const
{
	// It may be that the filename is relative.
	string const name = MakeAbsPath(fname, lv_.buffer()->filePath());
	return IsFileReadable(name);
}
