/**
 * \file ControlInclude.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Alejandro Aguilar Sierra
 * \author John Levon, moz@compsoc.man.ac.uk
 * \author Angus Leeming, a.leeming@.ac.uk
 */

#include <config.h>
#include <utility>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlInclude.h"
#include "ViewBase.h"
#include "ButtonControllerBase.h"
#include "buffer.h"
#include "Alert.h"
#include "BufferView.h"
#include "Dialogs.h"
#include "lyxfunc.h"
#include "gettext.h"
#include "helper_funcs.h"
#include "lyxrc.h"

#include "support/filetools.h"

#include "frontends/LyXView.h"

#include <boost/bind.hpp>

using std::pair;
using std::make_pair;

ControlInclude::ControlInclude(LyXView & lv, Dialogs & d)
	: ControlInset<InsetInclude, InsetInclude::Params>(lv, d)
{
	d_.showInclude = boost::bind(&ControlInclude::showInset, this, _1);
}


void ControlInclude::applyParamsToInset()
{
	inset()->set(params());
	lv_.view()->updateInset(inset(), true);
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
	lv_.getLyXFunc()->dispatch(LFUN_CHILDOPEN, file);
}


bool ControlInclude::fileExists(string const & file)
{
    string const fileWithAbsPath = MakeAbsPath(file, OnlyPath(params().masterFilename_));
    if (IsFileReadable(fileWithAbsPath))
	return true;
    else
	Alert::alert(_("Specified file doesn't exist !"));
    return false;
}
