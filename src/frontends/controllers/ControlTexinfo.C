/**
 * \file ControlTexinfo.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Herbert Voﬂ
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlTexinfo.h"
#include "funcrequest.h"


using std::string;

namespace lyx {
namespace frontend {

void getTexFileList(ControlTexinfo::texFileSuffix type,
		    std::vector<string> & list)
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
}


ControlTexinfo::ControlTexinfo(Dialog & parent)
	: Dialog::Controller(parent)
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

} // namespace frontend
} // namespace lyx
