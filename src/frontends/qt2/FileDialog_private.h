// -*- C++ -*-
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

#include "LString.h"
#include "lyxfunc.h"

#include "frontends/FileDialog.h"

#include <qfiledialog.h>

class LyXView;

class LyXFileDialog : public QFileDialog
{
  Q_OBJECT

public:
	LyXFileDialog(LyXView * lv, kb_action a, string const & p, string const & m, string const & t);

	friend class FileDialog;

public slots:
	void done(int);

private:
	LyXView * lv_;
	kb_action action_;
};

#endif // FILEDIALOG_PRIVATE_H
