// -*- C++ -*-
/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 *           This file Copyright 2000
 *           Allan Rae
 * ======================================================
 *
 * Author: Allan Rae <rae@lyx.org>
 * Non-xforms-specific code stripped-out and placed in a base class by
 * Angus Leeming <a.leeming@ic.ac.uk>
 */

#ifndef QT2BC_H
#define QT2BC_H

#include <list>

#ifdef __GNUG__
#pragma interface
#endif

class QWidget;
class QButton;

#include <boost/utility.hpp>
#include "ButtonControllerBase.h"
#include "ButtonController.h"

/** General purpose button controller for up to four buttons.
    Controls the activation of the OK, Apply and Cancel buttons.
    Actually supports 4 buttons in all and it's up to the user to decide on
    the activation policy and which buttons correspond to which output of the
    state machine.
*/
class qt2BC : public ButtonControllerBase
{
public:
	///
	qt2BC(string const &, string const &);

	/* Initialise Button Functions */
	/// Call refresh() when finished setting the buttons.
	void setOK(QButton * obj) {
		okay_ = obj;
	}
	///
	void setApply(QButton * obj) {
		apply_ = obj;
	}
	///
	void setCancel(QButton * obj) {
		cancel_ = obj;
	}
	///
	void setUndoAll(QButton * obj) {
		undo_all_ = obj;
	}
	///
	void addReadOnly(QWidget * obj) {
		read_only_.push_front(obj);
	}
	///
	void eraseReadOnly() {
		read_only_.erase(read_only_.begin(), read_only_.end());
	}

	/* Action Functions */
	/// force a refresh of the buttons
	virtual void refresh();

private:
	///
	QButton * okay_;
	///
	QButton * apply_;
	///
	QButton * cancel_;
	///
	QButton * undo_all_;
	/// List of items to be deactivated when in one of the read-only states
	std::list<QWidget *> read_only_;
};

#endif // XFORMSBC_H
