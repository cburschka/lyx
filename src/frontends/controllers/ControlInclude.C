/**
 * \file ControlInclude.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlInclude.h"
#include "helper_funcs.h"
#include "Kernel.h"

#include "buffer.h"
#include "format.h"
#include "funcrequest.h"
#include "gettext.h"
#include "lyxrc.h"

#include "insets/insetinclude.h"

#include "support/filefilterlist.h"
#include "support/filetools.h"

#include <utility>

using std::pair;
using std::string;

namespace lyx {

using support::FileFilterList;
using support::IsFileReadable;
using support::MakeAbsPath;
using support::OnlyPath;

namespace frontend {

ControlInclude::ControlInclude(Dialog & parent)
	: Dialog::Controller(parent)
{}


bool ControlInclude::initialiseParams(string const & data)
{
	InsetIncludeMailer::string2params(data, params_);
	return true;
}


void ControlInclude::clearParams()
{
	params_ = InsetCommandParams();
}


void ControlInclude::dispatchParams()
{
	string const lfun = InsetIncludeMailer::params2string(params_);
	kernel().dispatch(FuncRequest(LFUN_INSET_APPLY, lfun));
}


void ControlInclude::setParams(InsetCommandParams const & params)
{
	params_ = params;
}


string const ControlInclude::browse(string const & in_name, Type in_type) const
{
	string const title = _("Select document to include");

	// input TeX, verbatim, or LyX file ?
	FileFilterList filters;
	switch (in_type) {
	case INCLUDE:
	case INPUT:
	    filters = FileFilterList(_("LaTeX/LyX Documents (*.tex *.lyx)"));
	    break;
	case VERBATIM:
	    break;
	}

	pair<string, string> dir1(N_("Documents|#o#O"),
				  string(lyxrc.document_path));

	string const docpath = OnlyPath(kernel().buffer().fileName());

	return browseRelFile(in_name, docpath, title,
			     filters, false, dir1);
}


void ControlInclude::load(string const & file)
{
	string const ext = support::GetExtension(file);
	if (ext == "lyx")
		kernel().dispatch(FuncRequest(LFUN_CHILDOPEN, file));
	else
		// tex file or other text file in verbatim mode
		formats.edit(kernel().buffer(), file, "text");
}


bool ControlInclude::fileExists(string const & file)
{
	string const fileWithAbsPath
		= MakeAbsPath(file,
			      OnlyPath(kernel().buffer().fileName()));

	if (IsFileReadable(fileWithAbsPath))
		return true;

	return false;
}

} // namespace frontend
} // namespace lyx
