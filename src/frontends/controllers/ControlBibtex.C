/**
 * \file ControlBibtex.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Angus Leeming
 * \author Herbert Voﬂ
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlBibtex.h"

#include "buffer.h"
#include "bufferparams.h"

#include "lyxrc.h"
#include "helper_funcs.h"
#include "tex_helpers.h"
#include "gettext.h"

#include "support/filetools.h"
#include "support/globbing.h"

using lyx::support::FileFilterList;
using lyx::support::OnlyFilename;

using std::pair;
using std::string;
using std::vector;


ControlBibtex::ControlBibtex(Dialog & d)
	: ControlCommand(d, "bibtex")
{}


string const ControlBibtex::browse(string const & in_name,
				   string const & title,
				   FileFilterList const & filters) const
{
	pair<string, string> dir1(_("Documents|#o#O"),
				  string(lyxrc.document_path));
	return browseRelFile(in_name, kernel().bufferFilepath(),
			     title, filters, false, dir1);
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


bool ControlBibtex::usingBibtopic() const
{
    return kernel().buffer().params().use_bibtopic;
}
