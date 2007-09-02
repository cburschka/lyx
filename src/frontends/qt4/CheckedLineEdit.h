// -*- C++ -*-
/**
 * \file qt4/CheckedLineEdit.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CHECKEDLINEEDIT_H
#define CHECKEDLINEEDIT_H

class QWidget;
class QLineEdit;

namespace lyx {
namespace frontend {

class BCView;

void addCheckedLineEdit(BCView & bcview,
	QLineEdit * input, QWidget * label = 0);

class CheckedLineEdit
{
public:
	CheckedLineEdit(QLineEdit * input, QWidget * label = 0);
	bool check() const;

private:
	QLineEdit * input_;
	QWidget * label_;
};

} // namespace frontend
} // namespace lyx

#endif // CHECKEDLINEEDIT_H
