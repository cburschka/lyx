/**
 * \file ControlTexinfo.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Herbert Voﬂ
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlTexinfo.h"
#include "FuncRequest.h"

#include "support/filetools.h"

#include <algorithm>

using std::string;
using std::vector;

namespace lyx {

using support::onlyFilename;

namespace frontend {

void getTexFileList(ControlTexinfo::texFileSuffix type,
		    std::vector<string> & list, bool withPath)
{
	string filename;
	switch (type) {
	case ControlTexinfo::bst:
		filename = "bstFiles.lst";
		break;
	case ControlTexinfo::cls:
		filename = "clsFiles.lst";
		break;
	case ControlTexinfo::sty:
		filename = "styFiles.lst";
		break;
	}
	getTexFileList(filename, list);
	if (list.empty()) {
		// build filelists of all availabe bst/cls/sty-files.
		// Done through kpsewhich and an external script,
		// saved in *Files.lst
		rescanTexStyles();
		getTexFileList(filename, list);
	}
	if (withPath)
		return;
	vector<string>::iterator it  = list.begin();
	vector<string>::iterator end = list.end();
	for (; it != end; ++it) {
		*it = onlyFilename(*it);
	}
	// sort on filename only (no path)
	std::sort(list.begin(), list.end());
}


ControlTexinfo::ControlTexinfo(Dialog & parent)
	: Controller(parent)
{}


void ControlTexinfo::viewFile(string const & filename) const
{
	string const arg = "file " + filename;
	kernel().dispatch(FuncRequest(LFUN_DIALOG_SHOW, arg));
}


string const ControlTexinfo::getClassOptions(string const & filename) const
{
	return getListOfOptions(filename, "cls");
}


string const ControlTexinfo::getFileType(ControlTexinfo::texFileSuffix type) const
{
	string ftype;
	switch (type) {
	case ControlTexinfo::bst:
		ftype = "bst";
		break;
	case ControlTexinfo::cls:
		ftype = "cls";
		break;
	case ControlTexinfo::sty:
		ftype = "sty";
		break;
	}
	return ftype;
}

} // namespace frontend
} // namespace lyx
