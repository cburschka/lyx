#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "kdeBC.h"
#include <qpushbutton.h>

kdeBC::kdeBC(string const & cancel, string const & close)
	: ButtonControllerBase(cancel, close),
	  okay_(0), apply_(0), cancel_(0), undo_all_(0), read_only_()
{}


void kdeBC::refresh()
{
	if (okay_) {
		if (bp().buttonStatus(ButtonPolicy::OKAY)) {
			okay_->setEnabled(true);
		} else {
			okay_->setEnabled(false);
		}
	}
	if (apply_) {
		if (bp().buttonStatus(ButtonPolicy::APPLY)) {
			apply_->setEnabled(true);
		} else {
			apply_->setEnabled(false);
		}
	}
	if (undo_all_) {
		if (bp().buttonStatus(ButtonPolicy::UNDO_ALL)) {
			undo_all_->setEnabled(true);
		} else {
			undo_all_->setEnabled(false);
		}
	}
	if (cancel_) {
		if (bp().buttonStatus(ButtonPolicy::CANCEL)) {
			cancel_->setText(cancel_label.c_str());
		} else {
			cancel_->setText(close_label.c_str());
		}
	}
	if (!read_only_.empty()) {
		bool enable = true;
		if (bp().isReadOnly()) enable = false;
			
		for (std::list<QWidget *>::iterator iter = read_only_.begin();
		     iter != read_only_.end(); ++iter) {
			(*iter)->setEnabled(enable);
		}
	}
}
