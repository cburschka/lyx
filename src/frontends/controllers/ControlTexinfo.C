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
#include "Lsstream.h"
#include "BufferView.h"
#include "gettext.h"
#include "support/filetools.h" // FileSearch
#include "version.h"
#include "support/syscall.h"
#include "support/path.h"
#include "lyx_gui_misc.h"

extern string system_lyxdir;
extern string user_lyxdir;
extern string help_lyxdir;

using SigC::slot;

ControlTexinfo::ControlTexinfo(LyXView & lv, Dialogs & d)
			: ControlDialog<ControlConnectBI>(lv, d)
{
	d_.showTexinfo.connect(SigC::slot(this, &ControlTexinfo::show));
}

// build filelists of all availabe bst/cls/sty-files. done through
// kpsewhich and an external script, saved in *Files.lst
void ControlTexinfo::rescanStyles()
{
	// Run rescan in user lyx directory
	Path p(user_lyxdir);
	Systemcalls one(Systemcalls::System,
			AddName(system_lyxdir,"TeXFiles.sh"));
	p.pop();
}

void ControlTexinfo::runTexhash()
{
	// Run texhash in user lyx directory
	Path p(user_lyxdir);

	//path to texhash through system
	Systemcalls one(Systemcalls::System,"texhash"); 
	p.pop();
//	WriteAlert(_("texhash run!"), 
//		   _("rebuilding of the TeX-tree could only be successfull"),
//		   _("if you have had user-write-permissions to the tex-dir."));
}


void ControlTexinfo::viewFile(string const filename)
{
	lv_.getDialogs()->showFile(filename);
}

void ControlTexinfo::help()
{
	lv_.getDialogs()->showFile(help_lyxdir+"Texinfo.hlp");
}
