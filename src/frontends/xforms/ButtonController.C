#include <config.h>

#include <algorithm>

#include FORMS_H_LOCATION

#ifdef __GNUG__
#pragma implementation
#endif

#include "ButtonController.h"
#include "support/LAssert.h"
#include "gettext.h" // _()
//#include "debug.h"

using std::find;
using std::vector;

ButtonController::ButtonController(ButtonPolicy * bp,
				   char const * cancel, char const * close)
	: bp_(bp), okay_(0), apply_(0), cancel_(0), undo_all_(0),
	  read_only_(), trigger_change_(),
	  cancel_label(cancel), close_label(close)
{
	Assert(bp);
}


void ButtonController::refresh()
{
	if (okay_) {
		if (bp_->buttonStatus(ButtonPolicy::OKAY)) {
			fl_activate_object(okay_);
			fl_set_object_lcol(okay_, FL_BLACK);
		} else {
			fl_deactivate_object(okay_);
			fl_set_object_lcol(okay_, FL_INACTIVE);
		}
	}
	if (apply_) {
		if (bp_->buttonStatus(ButtonPolicy::APPLY)) {
			fl_activate_object(apply_);
			fl_set_object_lcol(apply_, FL_BLACK);
		} else {
			fl_deactivate_object(apply_);
			fl_set_object_lcol(apply_, FL_INACTIVE);
		}
	}
	if (undo_all_) {
		if (bp_->buttonStatus(ButtonPolicy::UNDO_ALL)) {
			fl_activate_object(undo_all_);
			fl_set_object_lcol(undo_all_, FL_BLACK);
		} else {
			fl_deactivate_object(undo_all_);
			fl_set_object_lcol(undo_all_,
					   FL_INACTIVE);
		}
	}
	if (cancel_) {
		if (bp_->buttonStatus(ButtonPolicy::CANCEL)) {
			fl_set_object_label(cancel_,
					    _(cancel_label));
		} else {
			fl_set_object_label(cancel_,
					    _(close_label));
		}
	}
	if (!read_only_.empty()) {
		if (bp_->isReadOnly()) {
			std::list<FL_OBJECT *>::iterator
				end = read_only_.end();
			for (std::list<FL_OBJECT *>::iterator
				     iter = read_only_.begin();
			     iter != end;
			     ++iter) {
				fl_deactivate_object(*iter);
				fl_set_object_lcol(*iter,
						   FL_INACTIVE);
			}
		} else {
			std::list<FL_OBJECT *>::iterator
				end = read_only_.end();
			for (std::list<FL_OBJECT *>::iterator
				     iter = read_only_.begin();
			     iter != end;
			     ++iter) {
				fl_activate_object(*iter);
				fl_set_object_lcol(*iter,
						   FL_BLACK);
			}
		}
	}
}


void ButtonController::input(ButtonPolicy::SMInput in)
{
	//lyxerr << "ButtonController::input: bp_[" << bp_ << "]" << endl;
	bp_->input(in);
	refresh();
}


void ButtonController::ok()
{
	input(ButtonPolicy::SMI_OKAY);
}


void ButtonController::apply()
{
	input(ButtonPolicy::SMI_APPLY);
}


void ButtonController::cancel()
{
	input(ButtonPolicy::SMI_CANCEL);
}


void ButtonController::undoAll()
{
	input(ButtonPolicy::SMI_UNDO_ALL);
}


void ButtonController::hide()
{
	input(ButtonPolicy::SMI_HIDE);
}


bool ButtonController::readOnly(bool ro)
{
	if (ro) {
		input(ButtonPolicy::SMI_READ_ONLY);
	} else {
		input(ButtonPolicy::SMI_READ_WRITE);
	}
	return ro;
}


void ButtonController::readWrite()
{
	readOnly(false);
}


bool ButtonController::valid(bool v, FL_OBJECT * obj)
{ 
	if (obj && !trigger_change_.empty()) {
		vector<FL_OBJECT *>::const_iterator cit =
			find(trigger_change_.begin(), trigger_change_.end(),
			     obj);

		// Only trigger a change if the obj is in the list
		if (cit != trigger_change_.end()) {
			if (v) {
				input(ButtonPolicy::SMI_VALID);
			} else {
				input(ButtonPolicy::SMI_INVALID);
			}
		}
	} else {
		if (v) {
			input(ButtonPolicy::SMI_VALID);
		} else {
			input(ButtonPolicy::SMI_INVALID);
		}
	}
	
	return v;
}


void ButtonController::invalid()
{
	valid(false);
}
