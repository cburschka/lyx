/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2001 The LyX Team.
 *
 * ======================================================
 *
 * \file ControlTexinfo.C
 * \author Herbert Voss <voss@lyx.org>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlTexinfo.h"
#include "Dialogs.h"
#include "BufferView.h"
#include "gettext.h"
#include "helper_funcs.h"
#include "tex_helpers.h"

#include "frontends/LyXView.h"

#include "support/filetools.h" // FileSearch
#include "support/path.h"
#include "support/lstrings.h"

extern string user_lyxdir; // home of *Files.lst


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
	switch (type) {
	    case bst: 
		return getTexFileList("bstFiles.lst", withFullPath);
		break;
	    case cls:
		return getTexFileList("clsFiles.lst", withFullPath);
		break;
	    case sty:
		return getTexFileList("styFiles.lst", withFullPath);
		break;
	}
	return string();
}


void ControlTexinfo::viewFile(string const filename) const
{
	lv_.getDialogs()->showFile(filename);
}


string const ControlTexinfo::getClassOptions(string const & filename) const
{
	return getListOfOptions(filename, "cls");
}
