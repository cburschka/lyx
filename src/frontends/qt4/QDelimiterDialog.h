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

namespace lyx {
namespace frontend {

class QMathDelimiter;

class QDelimiterDialog : public QDialog, public Ui::QDelimiterUi {
	Q_OBJECT
public:
	QDelimiterDialog(QMathDelimiter * form);
public Q_SLOTS:
	void on_leftCO_activated(int);
	void on_rightCO_activated(int);
	void on_matchCB_stateChanged(int);
	void insertClicked();
private:
	/// owning form
	QMathDelimiter * form_;
};

} // namespace frontend
} // namespace lyx

#endif // QDELIMITERDIALOG_H
