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

class QToolButton;

class LyXFileDialog : public QFileDialog
{
	Q_OBJECT
public:
	LyXFileDialog(string const & p, string const & m, string const & t,
		FileDialog::Button const & b1, FileDialog::Button const & b2);

public slots:
	void buttonClicked();
 
private:
	QToolButton * b1_;
	string b1_dir_;
 
	QToolButton * b2_;
	string b2_dir_;
};

#endif // FILEDIALOG_PRIVATE_H
