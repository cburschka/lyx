/**
 * \file QRefDialog.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Kalle Dalheimer <kalle@klaralvdalens-datakonsult.se>
 */

#ifndef QREFDIALOG_H
#define QREFDIALOG_H
 
#include <config.h>
 
#include "ui/QRefDialogBase.h"

class QRef;

class QRefDialog : public QRefDialogBase
{ Q_OBJECT

public:
	QRefDialog(QRef * form, QWidget * parent = 0, const char * name = 0, bool modal = FALSE, WFlags fl = 0);
	~QRefDialog();

public slots:
	void apply_adaptor();
	void goto_adaptor();
	void highlight_adaptor(const QString &);
	void close_adaptor();
	void select_adaptor(const QString &);
	void sort_adaptor(bool);
	void update_adaptor();

protected:
	void closeEvent(QCloseEvent * e);

private:
	QRef * form_;
};

#endif // QREFDIALOG_H
