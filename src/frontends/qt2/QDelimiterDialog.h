/**
 * \file QDelimiterDialog.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef QDELIMITERDIALOG_H
#define QDELIMITERDIALOG_H

#include <config.h>

#include "LString.h"
 
#include "ui/QDelimiterDialogBase.h"

class QMath;
class IconPalette;
class QLabel;

class QDelimiterDialog : public QDelimiterDialogBase
{
	Q_OBJECT

public:
	QDelimiterDialog(QMath * form);

public slots:
	void ldelim_clicked(string str);
	void rdelim_clicked(string str);
	void insertClicked();

protected:
	//needed ? virtual void closeEvent(QCloseEvent * e);

private:
	void set_label(QLabel * label, string const & str);

	/// symbol of left delimiter 
	string left_;

	/// symbol of right delimiter
	string right_;
 
	/// owning form
	QMath * form_;
};

#endif // QDELIMITERDIALOG_H
