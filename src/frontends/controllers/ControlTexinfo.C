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

#include "ViewBase.h"
#include "ButtonControllerBase.h"
#include "ControlTexinfo.h"
#include "Dialogs.h"
#include "LyXView.h"
#include "BufferView.h"
#include "gettext.h"
#include "support/filetools.h" // FileSearch
#include "support/syscall.h"
#include "support/path.h"
#include "helper_funcs.h"
#include "support/lstrings.h"

extern string user_lyxdir; // home of *Files.lst

ControlTexinfo::ControlTexinfo(LyXView & lv, Dialogs & d)
			: ControlDialog<ControlConnectBI>(lv, d)
{
	d_.showTexinfo.connect(SigC::slot(this, &ControlTexinfo::show));
}

// build filelists of all availabe bst/cls/sty-files. done through
// kpsewhich and an external script, saved in *Files.lst
void ControlTexinfo::rescanStyles() const
{
	// Run rescan in user lyx directory
	Path p(user_lyxdir);
	Systemcalls one(Systemcalls::System,
			LibFileSearch("scripts", "TeXFiles.sh"));
	p.pop();
}


void ControlTexinfo::runTexhash() const
{
	// Run texhash in user lyx directory
	Path p(user_lyxdir);

	//path to texhash through system
	Systemcalls one(Systemcalls::System,"texhash"); 
	p.pop();
//	Alert::alert(_("texhash run!"), 
//		   _("rebuilding of the TeX-tree could only be successfull"),
//		   _("if you have had user-write-permissions to the tex-dir."));
}


namespace {

string const sortEntries(string & str_in)
{
	std::vector<string> dbase = getVectorFromString(str_in,"\n");
	std::sort(dbase.begin(), dbase.end()); 		// sort entries
	std::vector<string>::iterator p = 
	    std::unique(dbase.begin(), dbase.end()); 	// compact
	dbase.erase(p, dbase.end()); 			// shrink
	return getStringFromVector(dbase,"\n");
}

} //namespace anon


string const
ControlTexinfo::getContents(texFileSuffix type, bool withFullPath) const
{
	static string const bstFilename("bstFiles.lst");
	static string const clsFilename("clsFiles.lst");
	static string const styFilename("styFiles.lst");

	string filename;
	switch (type) {
	case bst: 
		filename = bstFilename;
		break;
	case cls:
		filename = clsFilename;
		break;
	case sty:
		filename = styFilename;
		break;
	}

	string fileContents = GetFileContents(LibFileSearch(string(),filename));
	// everything ok?
	if (!fileContents.empty()) {
		if (withFullPath)
			return(sortEntries(fileContents));
		else {
			int Entries = 1;
			string dummy = OnlyFilename(token(fileContents,'\n',1));
			string contents = dummy;
			do {
				dummy = OnlyFilename(token(fileContents,'\n',++Entries));
				contents += ("\n"+dummy);
			} while (!dummy.empty());
			return(sortEntries(contents));
		}
	} else
		return _("Missing filelist. try Rescan");
}

void ControlTexinfo::viewFile(string const filename) const
{
	lv_.getDialogs()->showFile(filename);
}


void ControlTexinfo::help() const
{
	lv_.getDialogs()->showFile(i18nLibFileSearch("help","Texinfo.hlp"));
}
