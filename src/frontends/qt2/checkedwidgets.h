// -*- C++ -*-
/**
 * \file qt2/checkedwidgets.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CHECKEDWIDGETS_H
#define CHECKEDWIDGETS_H


#include "BCView.h"

class QLabel;
class QLineEdit;


void addCheckedLineEdit(BCView & bcview,
			QLineEdit * input, QLabel * label = 0);

class CheckedLineEdit : public CheckedWidget {
public:
	CheckedLineEdit(QLineEdit * input, QLabel * label = 0);

private:
	///
	virtual bool check() const;

	///
	QLineEdit * input_;
	QLabel * label_;
};

#endif // CHECKEDWIDGETS_H
