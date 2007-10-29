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

#include <QLineEdit>
#include "KeySequence.h"

class QEvent;
class QKeyEvent;

namespace lyx {
namespace frontend {

/**
 * A lineedit for inputting shortcuts
 */
class ShortcutLineEdit : public QLineEdit {
	Q_OBJECT
public:
	ShortcutLineEdit(QWidget * parent);
	void reset();
	bool eventFilter(QObject*, QEvent* e );
	lyx::KeySequence const getKeySequence() const;
protected Q_SLOTS:
	bool event(QEvent* e);
	void keyPressEvent(QKeyEvent * e);
private:
	void appendToSequence(QKeyEvent * e);
	lyx::KeySequence keysequence_;
	bool has_cursor_;
};

} // namespace frontend
} // namespace lyx

#endif // CUSTOMIZEDWIDGETS_H
