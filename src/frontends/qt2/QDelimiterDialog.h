// -*- C++ -*-
/**
 * \file QDelimiterDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QDELIMITERDIALOG_H
#define QDELIMITERDIALOG_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ui/QDelimiterDialogBase.h"
#include "LString.h"

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
