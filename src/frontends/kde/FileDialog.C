/**
 * \file FileDialog.C
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

using std::make_pair;
using std::pair;
using std::endl;

FileDialog::FileDialog(LyXView *lv, string const &t, kb_action s, Button b1, Button b2)
	: private_(0), lv_(lv), title_(t), success_(s)
{
	// FIXME
}


FileDialog::~FileDialog()
{
}


FileDialog::Result const FileDialog::Select(string const & path, string const & mask, string const & suggested)
{
	string filter = mask;
 
	if (mask.empty())
		filter = _("*|All files");
 
	LyXKFileDialog * dlg = new LyXKFileDialog(lv_, success_, path, filter, title_);

	lyxerr[Debug::GUI] << "Select with path \"" << path << "\", mask \"" << filter << "\", suggested \"" << suggested << endl;

	if (!suggested.empty())
		dlg->setSelection(suggested.c_str());
 
	if (success_ == LFUN_SELECT_FILE_SYNC) {
		FileDialog::Result result;

		lyxerr[Debug::GUI] << "Synchronous FileDialog : " << endl;

		result.first = FileDialog::Chosen;

		int res = dlg->exec();

		lyxerr[Debug::GUI] << "result " << res << endl;
	
		if (res == QDialog::Accepted)
			result.second = string(dlg->selectedFile().data());

		delete dlg;
 
		return result;
	}

	dlg->show();

	return make_pair(FileDialog::Later, string());
}
