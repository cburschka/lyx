/**
 * \file qt2/FileDialog.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>
#include <gettext.h>
#include <utility>

#include "commandtags.h"
#include "LString.h"
#include "frontends/FileDialog.h"
#include "FileDialog_private.h"
#include "debug.h"

#include <qapplication.h>
 
using std::make_pair;
using std::pair;
using std::endl;

struct FileDialog::Private { 
	Button b1;
	Button b2;
};
 
FileDialog::FileDialog(LyXView *lv, string const & t, kb_action s, Button b1, Button b2)
	: private_(new FileDialog::Private()), lv_(lv), title_(t), success_(s)
{
	private_->b1 = b1;
	private_->b2 = b2;
}


FileDialog::~FileDialog()
{
	delete private_;
}


FileDialog::Result const FileDialog::Select(string const & path, string const & mask, string const & suggested)
{
	string filter = mask;
	if (mask.empty())
		filter = _("*|All files");

	LyXFileDialog dlg(path, filter, title_, private_->b1, private_->b2);
	lyxerr[Debug::GUI] << "Select with path \"" << path << "\", mask \"" << filter << "\", suggested \"" << suggested << endl;

	if (!suggested.empty())
		dlg.setSelection(suggested.c_str());

	// This code relies on DestructiveClose which is broken
	// in Qt < 3.0.5. So we just don't allow it for now.
	//if (success_ == LFUN_SELECT_FILE_SYNC) {
 
	FileDialog::Result result;
	lyxerr[Debug::GUI] << "Synchronous FileDialog : " << endl;
	result.first = FileDialog::Chosen;
	int res = dlg.exec();
	lyxerr[Debug::GUI] << "result " << res << endl;
	if (res == QDialog::Accepted)
		result.second = string(dlg.selectedFile().data());
	dlg.hide();
	return result;
#if 0
	dlg->show();
	return make_pair(FileDialog::Later, string());
#endif
}
