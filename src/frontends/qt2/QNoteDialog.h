// -*- C++ -*-
/**
 * \file QNoteDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QNOTEDIALOG_H
#define QNOTEDIALOG_H

#include "ui/QNoteDialogBase.h"


class QNote;

class QNoteDialog : public QNoteDialogBase {
	Q_OBJECT
public:
	QNoteDialog(QNote * form);
protected slots:
	virtual void change_adaptor();
protected:
	virtual void closeEvent(QCloseEvent * e);
private:
	QNote * form_;
};

#endif // QNOTEDIALOG_H
