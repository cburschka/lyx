// -*- C++ -*-
/**
 * \file FileDialog_private.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef FILEDIALOG_PRIVATE_H
#define FILEDIALOG_PRIVATE_H

#ifdef __GNUG__
#pragma interface
#endif

#include "frontends/FileDialog.h"
#include <qfiledialog.h>
#include "LString.h"

class QToolButton;

class LyXFileDialog : public QFileDialog
{
	Q_OBJECT
public:
	LyXFileDialog(string const & p, string const & m, string const & t,
		      FileDialog::Button const & b1,
		      FileDialog::Button const & b2);
public slots:
	void buttonClicked();
private:
	QToolButton * b1_;
	string b1_dir_;

	QToolButton * b2_;
	string b2_dir_;
};

#endif // FILEDIALOG_PRIVATE_H
