/**
 * \file QRefDialog.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Kalle Dalheimer <kalle@klaralvdalens-datakonsult.se>
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef QREFDIALOG_H
#define QREFDIALOG_H

#include <config.h>

#include "ui/QRefDialogBase.h"

class QRef;

class QRefDialog : public QRefDialogBase
{ Q_OBJECT

public:
	QRefDialog(QRef * form);

public slots:
	void changed_adaptor();
	void gotoClicked();
	void refHighlighted(const QString &);
	void refSelected(const QString &);
	void sortToggled(bool);
	void updateClicked();

protected:
	void closeEvent(QCloseEvent * e);

private:
	QRef * form_;
};

#endif // QREFDIALOG_H
