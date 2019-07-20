// -*- C++ -*-
/**
 * \file CustomizedWidgets.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Bo Peng
 * \author Edwin Leuven
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CUSTOMIZEDWIDGETS_H
#define CUSTOMIZEDWIDGETS_H

#include <QLabel>
#include "KeySequence.h"

class QEvent;
class QKeyEvent;

namespace lyx {
namespace frontend {

/**
 * A widget for inputting shortcuts
 */
class ShortcutWidget : public QLabel {
	Q_OBJECT
public:
	ShortcutWidget(QWidget * parent);
	void reset();
	KeySequence const getKeySequence() const;
	void setKeySequence(lyx::KeySequence const & s);
	void removeFromSequence();
protected Q_SLOTS:
	bool event(QEvent* e);
	void keyPressEvent(QKeyEvent * e);
private:
	void appendToSequence(QKeyEvent * e);
	KeySequence keysequence_;
	bool has_cursor_;
};

} // namespace frontend
} // namespace lyx

#endif // CUSTOMIZEDWIDGETS_H
