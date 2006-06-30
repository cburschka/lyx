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

#include "ui/QDelimiterUi.h"
#include <string>

#include <QLabel>
#include <QDialog>

class IconPalette;
class QLabel;

namespace lyx {
namespace frontend {

class QMathDelimiter;

class QDelimiterDialog : public QDialog, public Ui::QDelimiterUi {
	Q_OBJECT
public:
	QDelimiterDialog(QMathDelimiter * form);
public Q_SLOTS:
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

} // namespace frontend
} // namespace lyx

#endif // QDELIMITERDIALOG_H
