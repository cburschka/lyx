// -*- C++ -*-
/**
 * \file CustomizedWidgets.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Bo Peng
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CUSTOMIZEDWIDGETS_H
#define CUSTOMIZEDWIDGETS_H

#include <QEvent>
#include <QCloseEvent>
#include <QLineEdit>

/**
 * A lineedit for inputting shortcuts
 */
class ShortcutLineEdit : public QLineEdit {
	Q_OBJECT
public:
	ShortcutLineEdit(QWidget * parent) : QLineEdit(parent) {}
protected Q_SLOTS:
	void keyPressEvent(QKeyEvent * e);
	bool event(QEvent * e);
};


/**
  * A lineedit that displays a hint message when there is no
  * text and not under focus.
  */
class SearchLineEdit : public QLineEdit {
	Q_OBJECT
public:
	QString const hintMessage() const;
	SearchLineEdit(QWidget * parent) : QLineEdit(parent) {}
protected Q_SLOTS:
	void focusInEvent(QFocusEvent * e);
	void focusOutEvent(QFocusEvent * e);
};


#endif // CUSTOMIZEDWIDGETS_H
