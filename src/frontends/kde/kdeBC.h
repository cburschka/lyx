// -*- C++ -*-
/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 * ======================================================
 *
 * Author: Angus Leeming <a.leeming@ic.ac.uk>
 */

#ifndef KDEBC_H
#define KDEBC_H

#include "ButtonController.h"
#include <list>

#ifdef __GNUG__
#pragma interface
#endif

class QWidget;
class QPushButton;

class kdeBC : public ButtonControllerBase
{
public:
	///
	kdeBC(string const & cancel, string const & close);

	/* Initialise Button Functions */
	/// Call refresh() when finished setting the buttons.
	void setOK(QPushButton * obj) {
		okay_ = obj;
	}
	///
	void setApply(QPushButton * obj) {
		apply_ = obj;
	}
	///
	void setCancel(QPushButton * obj) {
		cancel_ = obj;
	}
	///
	void setUndoAll(QPushButton * obj) {
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
	QPushButton * okay_;
	///
	QPushButton * apply_;
	///
	QPushButton * cancel_;
	///
	QPushButton * undo_all_;
	/// List of items to be deactivated when in one of the read-only states
	std::list<QWidget *> read_only_;
};

#endif // KDEBC_H
