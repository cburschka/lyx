// -*- C++ -*-
/**
 * \file QTocDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QTOCDIALOG_H
#define QTOCDIALOG_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ui/QTocDialogBase.h"

class QToc;

class QTocDialog : public QTocDialogBase {
	Q_OBJECT
public:
	QTocDialog(QToc * form);
	~QTocDialog();
public slots:
	void activate_adaptor(int);
	void depth_adaptor(int);
	void select_adaptor(QListViewItem *);
	void update_adaptor();
protected:
	void closeEvent(QCloseEvent * e);
private:
	QToc * form_;
};

#endif // QTOCDIALOG_H
