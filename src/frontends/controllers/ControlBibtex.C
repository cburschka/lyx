/**
 * \file ControlBibtex.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Angus Leeming
 * \author Herbert Voss
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlBibtex.h"
#include "Kernel.h"
#include "helper_funcs.h"
#include "tex_helpers.h"

#include "funcrequest.h"
#include "gettext.h"
#include "lyxrc.h"

#include "support/filetools.h"

using namespace lyx::support;

using std::pair;
using std::vector;


ControlBibtex::ControlBibtex(Dialog & d)
	: Dialog::Controller(d)
{}


bool ControlBibtex::initialiseParams(string const & data)
{
	InsetBibtexMailer::string2params(data, kernel().buffer(), params_);
	return true;
}


void ControlBibtex::clearParams()
{
	params_.erase();
}


void ControlBibtex::dispatchParams()
{
	string const lfun =
		InsetBibtexMailer::params2string(params_, kernel().buffer());
	kernel().dispatch(FuncRequest(LFUN_INSET_APPLY, lfun));
}


string const ControlBibtex::Browse(string const & in_name,
				   string const & title,
				   string const & pattern)
{
	pair<string, string> dir1(_("Documents|#o#O"),
				  string(lyxrc.document_path));
	return browseRelFile(in_name, kernel().bufferFilepath(),
			     title, pattern, false, dir1);
}


void ControlBibtex::getBibStyles(vector<string> & data) const
{
	data.clear();

	getTexFileList("bstFiles.lst", data);
	// test, if we have a valid list, otherwise run rescan
	if (data.empty()) {
		rescanBibStyles();
		getTexFileList("bstFiles.lst", data);
	}
	vector<string>::iterator it  = data.begin();
	vector<string>::iterator end = data.end();
	for (; it != end; ++it) {
		*it = OnlyFilename(*it);
	}
	std::sort(data.begin(), end);

	// Add an empy string to the list.
	data.insert(data.begin(), " ");
}


void ControlBibtex::getBibFiles(vector<string> & data) const
{
	data.clear();

	getTexFileList("bibFiles.lst", data);
	// test, if we have a valid list, otherwise run rescan
	if (data.empty()) {
		rescanBibStyles();
		getTexFileList("bibFiles.lst", data);
	}
	vector<string>::iterator it  = data.begin();
	vector<string>::iterator end = data.end();
	for (; it != end; ++it) {
		*it = OnlyFilename(*it);
	}
}


void ControlBibtex::rescanBibStyles() const
{
	rescanTexStyles();
}
