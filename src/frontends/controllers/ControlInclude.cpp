/**
 * \file ControlInclude.cpp
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
#include "frontend_helpers.h"

#include "Buffer.h"
#include "Format.h"
#include "FuncRequest.h"
#include "gettext.h"
#include "LyXRC.h"

#include "insets/InsetInclude.h"

#include "support/FileFilterList.h"
#include "support/filetools.h"

#include <utility>

using std::pair;
using std::string;

namespace lyx {

using support::FileFilterList;
using support::FileName;
using support::isFileReadable;
using support::makeAbsPath;
using support::onlyPath;

namespace frontend {

ControlInclude::ControlInclude(Dialog & parent)
	: Controller(parent), params_("include")
{}


bool ControlInclude::initialiseParams(string const & data)
{
	InsetIncludeMailer::string2params(data, params_);
	return true;
}


void ControlInclude::clearParams()
{
	params_.clear();
}


void ControlInclude::dispatchParams()
{
	string const lfun = InsetIncludeMailer::params2string(params_);
	dispatch(FuncRequest(getLfun(), lfun));
}


void ControlInclude::setParams(InsetCommandParams const & params)
{
	params_ = params;
}


docstring const ControlInclude::browse(docstring const & in_name, Type in_type) const
{
	docstring const title = _("Select document to include");

	// input TeX, verbatim, or LyX file ?
	FileFilterList filters;
	switch (in_type) {
	case INCLUDE:
	case INPUT:
		filters = FileFilterList(_("LaTeX/LyX Documents (*.tex *.lyx)"));
		break;
	case VERBATIM:
		break;
	case LISTINGS:
		break;
	}

	pair<docstring, docstring> dir1(_("Documents|#o#O"),
		from_utf8(lyxrc.document_path));

	docstring const docpath = from_utf8(onlyPath(buffer().fileName()));

	return browseRelFile(in_name, docpath, title,
			     filters, false, dir1);
}


void ControlInclude::edit(string const & file)
{
	string const ext = support::getExtension(file);
	if (ext == "lyx")
		dispatch(FuncRequest(LFUN_BUFFER_CHILD_OPEN, file));
	else
		// tex file or other text file in verbatim mode
		formats.edit(buffer(), 
			FileName(makeAbsPath(file, onlyPath(buffer().fileName()))),
			"text");
}


bool ControlInclude::fileExists(string const & file)
{
	FileName const fileWithAbsPath(
		makeAbsPath(file, onlyPath(buffer().fileName())));

	if (isFileReadable(fileWithAbsPath))
		return true;

	return false;
}

} // namespace frontend
} // namespace lyx
