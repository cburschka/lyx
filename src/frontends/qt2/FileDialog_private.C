/**
 * \file FileDialog_private.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#include <config.h>

#include "LString.h"

#include <qapplication.h>
#include <qfiledialog.h>

#include "QtLyXView.h"
#include "debug.h"
#include "funcrequest.h"

#include "FileDialog_private.h"

LyXFileDialog::LyXFileDialog(LyXView * lv, kb_action a,
		string const & p, string const & m, string const & t)
	: QFileDialog(p.c_str(), m.c_str(), qApp->mainWidget(), t.c_str(),
		            a == LFUN_SELECT_FILE_SYNC),
	  lv_(lv), action_(a)
{
	setCaption(t.c_str());
}


void LyXFileDialog::done(int what)
{
	lyxerr[Debug::GUI] << "Done FileDialog, value " << what << std::endl;

	if (action_ == LFUN_SELECT_FILE_SYNC) {
		QDialog::done(what);
		return;
	}

	if (what == QDialog::Accepted)
		lv_->getLyXFunc()->dispatch(FuncRequest(action_, selectedFile().data()));

	delete this;
}
