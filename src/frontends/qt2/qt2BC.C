/**
 * \file qt2BC.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Allan Rae <rae@lyx.org>
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "qt2BC.h"

#include <qbutton.h>

qt2BC::qt2BC(string const & cancel, string const & close)
	: ButtonControllerBase(cancel, close),
	  okay_(0), apply_(0), cancel_(0), restore_(0), read_only_()
{}


void qt2BC::refresh()
{
	if (okay_) {
		if (bp().buttonStatus(ButtonPolicy::OKAY)) {
		    okay_->setEnabled( true );
		} else {
		    okay_->setEnabled( false );
		}
	}
	if (apply_) {
		if (bp().buttonStatus(ButtonPolicy::APPLY)) {
		    apply_->setEnabled( true );
		} else {
		    apply_->setEnabled( false );
		}
	}
	if (restore_) {
		if (bp().buttonStatus(ButtonPolicy::RESTORE)) {
		    restore_->setEnabled( true );
		} else {
		    restore_->setEnabled( false );
		}
	}
	if (cancel_) {
		if (bp().buttonStatus(ButtonPolicy::CANCEL)) {
		    cancel_->setText( cancel_label_.c_str() );
		} else {
		    cancel_->setText( close_label_.c_str() );
		}
	}
	if (!read_only_.empty()) {
		bool enable = true;
		if (bp().isReadOnly()) enable = false;

		for (std::list<QWidget *>::iterator iter = read_only_.begin();
		     iter != read_only_.end(); ++iter) {
		    (*iter)->setEnabled( enable );
		}
	}
}
