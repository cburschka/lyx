/**
 * \file ControlTexinfo.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Herbert Voss
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "ControlTexinfo.h"
#include "tex_helpers.h"
#include "funcrequest.h"


namespace {

string getFileList(ControlTexinfo::texFileSuffix type, bool withFullPath)
{
	switch (type) {
	    case ControlTexinfo::bst:
		return getTexFileList("bstFiles.lst", withFullPath);
		break;
	    case ControlTexinfo::cls:
		return getTexFileList("clsFiles.lst", withFullPath);
		break;
	    case ControlTexinfo::sty:
		return getTexFileList("styFiles.lst", withFullPath);
		break;
	}
	return string();
}

} // namespace anon


ControlTexinfo::ControlTexinfo(Dialog & parent)
	: Dialog::Controller(parent)
{}


// build filelists of all availabe bst/cls/sty-files. done through
// kpsewhich and an external script, saved in *Files.lst
void ControlTexinfo::rescanStyles() const
{
    rescanTexStyles();
}


void ControlTexinfo::runTexhash() const
{
    texhash();
}


string const
ControlTexinfo::getContents(texFileSuffix type, bool withFullPath) const
{
	string list(getFileList(type, withFullPath));

	// initial scan
	if (list.empty()) {
		rescanStyles();
		list = getFileList(type, withFullPath);
	}
	return list;
}


void ControlTexinfo::viewFile(string const filename) const
{
	string const arg = "file " + filename;
	kernel().dispatch(FuncRequest(LFUN_DIALOG_SHOW, arg));
}


string const ControlTexinfo::getClassOptions(string const & filename) const
{
	return getListOfOptions(filename, "cls");
}
