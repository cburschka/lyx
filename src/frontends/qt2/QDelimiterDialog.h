// -*- C++ -*-
/**
 * \file QDelimiterDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QDELIMITERDIALOG_H
#define QDELIMITERDIALOG_H

#include "ui/QDelimiterDialogBase.h"
#include <string>


class QMathDelimiter;
class IconPalette;
class QLabel;

class QDelimiterDialog : public QDelimiterDialogBase {
	Q_OBJECT
public:
	QDelimiterDialog(QMathDelimiter * form);
public slots:
	void ldelim_clicked(const std::string & str);
	void rdelim_clicked(const std::string & str);
	void insertClicked();
protected:
	//needed ? virtual void closeEvent(QCloseEvent * e);
private:
	void set_label(QLabel * label, const std::string & str);

	/// symbol of left delimiter
	std::string left_;

	/// symbol of right delimiter
	std::string right_;

	/// owning form
	QMathDelimiter * form_;
};

#endif // QDELIMITERDIALOG_H
