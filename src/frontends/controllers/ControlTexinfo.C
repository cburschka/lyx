/**
 * \file ControlTexinfo.C
 * See the file COPYING.
 *
 * \author Herbert Voss
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlTexinfo.h"

#include "helper_funcs.h"
#include "tex_helpers.h"

#include "frontends/Dialogs.h"
#include "frontends/LyXView.h"

#include "support/filetools.h" // FileSearch
#include "support/path.h"
#include "support/lstrings.h"

extern string user_lyxdir; // home of *Files.lst

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
 
}

ControlTexinfo::ControlTexinfo(LyXView & lv, Dialogs & d)
	: ControlDialogBI(lv, d)
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
	lv_.getDialogs().showFile(filename);
}


string const ControlTexinfo::getClassOptions(string const & filename) const
{
	return getListOfOptions(filename, "cls");
}
