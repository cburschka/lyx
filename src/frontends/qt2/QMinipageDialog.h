// -*- C++ -*-
/**
 * \file QMinipageDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QMINIPAGEDIALOG_H
#define QMINIPAGEDIALOG_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ui/QMinipageDialogBase.h"

class QMinipage;

class QMinipageDialog : public QMinipageDialogBase {
	Q_OBJECT
public:
	QMinipageDialog(QMinipage * form);
protected slots:
	virtual void change_adaptor();
protected:
	virtual void closeEvent(QCloseEvent * e);
private:
	QMinipage * form_;
};

#endif // QMINIPAGEDIALOG_H
