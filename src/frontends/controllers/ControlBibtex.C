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
#include "biblio.h"

#include "buffer.h"
#include "bufferparams.h"

#include "lyxrc.h"
#include "helper_funcs.h"
#include "tex_helpers.h"
#include "gettext.h"

#include "support/filefilterlist.h"
#include "support/filetools.h"
#include "support/lstrings.h"

using std::pair;
using std::string;
using std::vector;


namespace lyx {

using support::contains;
using support::FileFilterList;
using support::onlyFilename;
using support::prefixIs;
using support::split;

namespace frontend {


ControlBibtex::ControlBibtex(Dialog & d)
	: ControlCommand(d, "bibtex")
{}


string const ControlBibtex::browseBib(string const & in_name) const
{
	pair<string, string> dir1(_("Documents|#o#O"),
				  string(lyxrc.document_path));
	FileFilterList const filter(_("BibTeX Databases (*.bib)"));
	return browseRelFile(in_name, kernel().bufferFilepath(),
			     _("Select a BibTeX database to add"),
			     filter, false, dir1);
}


string const ControlBibtex::browseBst(string const & in_name) const
{
	pair<string, string> dir1(_("Documents|#o#O"),
				  string(lyxrc.document_path));
	FileFilterList const filter(_("BibTeX Styles (*.bst)"));
	return browseRelFile(in_name, kernel().bufferFilepath(),
			     _("Select a BibTeX style"), filter, false, dir1);
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
		*it = onlyFilename(*it);
	}
	// sort on filename only (no path)
	std::sort(data.begin(), data.end());
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
		*it = onlyFilename(*it);
	}
	// sort on filename only (no path)
	std::sort(data.begin(), data.end());
}


void ControlBibtex::rescanBibStyles() const
{
	rescanTexStyles();
}


bool ControlBibtex::usingBibtopic() const
{
	return kernel().buffer().params().use_bibtopic;
}


bool ControlBibtex::bibtotoc() const
{
	return prefixIs(params().getOptions(), "bibtotoc");
}


string const ControlBibtex::getStylefile() const
{
	// the different bibtex packages have (and need) their
	// own "plain" stylefiles
	biblio::CiteEngine_enum const & engine =
		biblio::getEngine(kernel().buffer());
	string defaultstyle;
	switch (engine) {
	case biblio::ENGINE_BASIC:
		defaultstyle = "plain";
		break;
	case biblio::ENGINE_NATBIB_AUTHORYEAR:
		defaultstyle = "plainnat";
		break;
	case biblio::ENGINE_NATBIB_NUMERICAL:
		defaultstyle = "plainnat";
		break;
	case biblio::ENGINE_JURABIB:
		defaultstyle = "jurabib";
		break;
	}

	string bst = params().getOptions();
	if (bibtotoc()){
		// bibstyle exists?
		if (contains(bst,',')) {
			string bibtotoc = "bibtotoc";
			bst = split(bst, bibtotoc, ',');
		} else
			bst.erase();
	}

	// propose default style file for new insets
	// existing insets might have (legally) no bst files
	// (if the class already provides a style)
	if (bst.empty() && params().getContents().empty())
		bst = defaultstyle;

	return bst;
}

} // namespace frontend
} // namespace lyx
