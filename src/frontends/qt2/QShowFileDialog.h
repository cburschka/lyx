// -*- C++ -*-
/**
 * \file QShowFileDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QSHOWFILEDIALOG_H
#define QSHOWFILEDIALOG_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ui/QShowFileDialogBase.h"

class QShowFile;

class QShowFileDialog : public QShowFileDialogBase {
	Q_OBJECT
public:
	QShowFileDialog(QShowFile * form);
protected:
	virtual void closeEvent(QCloseEvent * e);
private:
	QShowFile * form_;
};

#endif // QSHOWFILEDIALOG_H
