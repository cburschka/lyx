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

#include "ControlInclude.h"
#include "helper_funcs.h"

#include "funcrequest.h"
#include "gettext.h"
#include "lyxrc.h"

#include "support/filetools.h"

#include <utility>

using std::pair;


ControlInclude::ControlInclude(Dialog & parent)
	: Dialog::Controller(parent)
{}


bool ControlInclude::initialiseParams(string const & data)
{
	InsetInclude::Params params;
	InsetIncludeMailer::string2params(data, params);
	inset_.reset(new InsetInclude(params));
	return true;
}


void ControlInclude::clearParams()
{
	inset_.reset();
}


void ControlInclude::dispatchParams()
{
	InsetInclude::Params p = params();
	string const lfun = InsetIncludeMailer::params2string(p);
	kernel().dispatch(FuncRequest(LFUN_INSET_APPLY, lfun));
}


void ControlInclude::setParams(InsetInclude::Params const & params)
{
	inset_->set(params);
}

string const ControlInclude::Browse(string const & in_name, Type in_type)
{
	string const title = _("Select document to include");

	// input TeX, verbatim, or LyX file ?
	string pattern;
	switch (in_type) {
	case INPUT:
	    pattern = _("*.(tex|lyx)| LaTeX/LyX Documents (*.tex *.lyx)");
	    break;

	case VERBATIM:
	    pattern = _("*| All files (*)");
	    break;

	case INCLUDE:
	    pattern = _("*.(tex|lyx)| LaTeX/LyX Documents (*.tex *.lyx)");
	    break;
	}

	pair<string, string> dir1(N_("Documents|#o#O"),
				  string(lyxrc.document_path));

	string const docpath = OnlyPath(params().masterFilename_);

	return browseRelFile(in_name, docpath, title, pattern, false, dir1);
}


void ControlInclude::load(string const & file)
{
	kernel().dispatch(FuncRequest(LFUN_CHILDOPEN, file));
}


bool ControlInclude::fileExists(string const & file)
{
	string const fileWithAbsPath
		= MakeAbsPath(file, OnlyPath(params().masterFilename_));

	if (IsFileReadable(fileWithAbsPath))
		return true;

	return false;
}
