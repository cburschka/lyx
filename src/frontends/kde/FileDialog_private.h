/**
 * \file FileDialog_private.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#ifndef FILEDIALOG_PRIVATE_H
#define FILEDIALOG_PRIVATE_H

#include <config.h>

#include <kfiledialog.h>

#include "LString.h"
#include "lyxfunc.h" 

#include "frontends/FileDialog.h"

class LyXView;

class LyXKFileDialog : public KFileDialog
{
public:
	LyXKFileDialog(LyXView * lv, kb_action a, string const & p, string const & m, string const & t);

	friend class FileDialog;

protected:
	virtual void done(int);

private:
	LyXView * lv_;
	int action_;
};

#endif // FILEDIALOG_PRIVATE_H
