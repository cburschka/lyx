/**
 * \file ControlInclude.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author John Levon
 * \author Angus Leeming 
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlInclude.h"

#include "helper_funcs.h"

#include "BufferView.h"
#include "funcrequest.h"
#include "gettext.h"
#include "lyxfunc.h"
#include "lyxrc.h"

#include "frontends/Alert.h"

#include "support/filetools.h"

#include <utility>

using std::pair;


ControlInclude::ControlInclude(LyXView & lv, Dialogs & d)
	: ControlInset<InsetInclude, InsetInclude::Params>(lv, d)
{}


void ControlInclude::applyParamsToInset()
{
	inset()->set(params());
	bufferview()->updateInset(inset(), true);
}


string const ControlInclude::Browse(string const & in_name, Type in_type)
{
	string const title = _("Select document to include");

	// input TeX, verbatim, or LyX file ?
	string pattern;
	switch (in_type) {
	case INPUT:
	    pattern = _("*.tex| LaTeX Documents (*.tex)");
	    break;

	case VERBATIM:
	    pattern = _("*| All files ");
	    break;

	case INCLUDE:
	    pattern = _("*.lyx| LyX Documents (*.lyx)");
	    break;
	}

	pair<string, string> dir1(N_("Documents|#o#O"),
				  string(lyxrc.document_path));

	string const docpath = OnlyPath(params().masterFilename_);

	return browseRelFile(&lv_, in_name, docpath, title, pattern, dir1);
}


void ControlInclude::load(string const & file)
{
	lyxfunc().dispatch(FuncRequest(LFUN_CHILDOPEN, file));
}


bool ControlInclude::fileExists(string const & file)
{
	string const fileWithAbsPath
		= MakeAbsPath(file, OnlyPath(params().masterFilename_));

	if (params().noload) {

		if (prefixIs(file, "../") || prefixIs(file, "/"))
			Alert::alert(_("Warning!"),
				_("On some systems, with this options only relative path names"),
				_("inside the master file dir are allowed. You might get a LaTeX error!"));
	}

	if (IsFileReadable(fileWithAbsPath))
		return true;
	
	else
		Alert::alert(_("Warning!"), _("Specified file doesn't exist"));
		return false;
}
