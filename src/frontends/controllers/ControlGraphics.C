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
#include "Dialogs.h"
#include "LyXView.h"
#include "gettext.h"
#include "lyxrc.h"

#include "insets/insetgraphics.h"
#include "insets/insetgraphicsParams.h" // need operator!=()

#include "support/FileInfo.h"  // for FileInfo
#include "helper_funcs.h"      // for browseFile
#include "support/lstrings.h"
#include "support/filetools.h" // for AddName
#include "BufferView.h"

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


// We need these in the file browser.
extern string system_lyxdir;
extern string user_lyxdir;

string const ControlGraphics::Browse(string const & in_name)
{
	string const title = N_("Graphics");
	// FIXME: currently we need the second '|' to prevent mis-interpretation 
	string const pattern = "*.(ps|eps|png|jpeg|jpg|gif|gz)|";

  	// Does user clipart directory exist?
  	string clipdir = AddName (user_lyxdir, "clipart");
 	FileInfo fileInfo(clipdir);
  	if (!(fileInfo.isOK() && fileInfo.isDir()))
  		// No - bail out to system clipart directory
  		clipdir = AddName (system_lyxdir, "clipart");
	pair<string, string> dir1(N_("Clipart|#C#c"), clipdir);
	pair<string, string> dir2(N_("Documents|#o#O"), string(lyxrc.document_path));
	// Show the file browser dialog
	return browseFile(&lv_, in_name, title, pattern, dir1, dir2);
}

string const ControlGraphics::readBB(string const & file)
{
// in a file it's an entry like %%BoundingBox:23 45 321 345
// the first number can following without a space, so we have
// to check a bit more. 
//	ControlGraphics::bbChanged = false;
	std::ifstream is(file.c_str());
	int count = 0;
	int const max_count = 50;	// don't search the whole file
	while (is && (++count < max_count)) {
		string s;
		is >> s;
		if (contains(s,"%%BoundingBox:")) {
			string a, b, c, d;
			is >> a >> b >> c >> d;
			if (is) {
				if (s != "%%BoundingBox:") 
				    return (s.substr(14)+" "+a+" "+b+" "+c+" ");
				else
				    return (a+" "+b+" "+c+" "+d+" ");
			}
		}
	}
	return string();
}

void ControlGraphics::help() const
{
	lv_.getDialogs()->showFile(i18nLibFileSearch("help","Graphics.hlp"));
}

