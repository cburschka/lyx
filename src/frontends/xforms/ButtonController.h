// -*- C++ -*-
/* ButtonController.h
 * Controls the activation of the OK, Apply and Cancel buttons.
 * Actually supports 4 buttons in all and it's up to the user to decide on
 * the activation policy and which buttons correspond to which output of the
 * state machine.
 * Author: Allan Rae <rae@lyx.org>
 * This file is part of
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
 */

#ifndef BUTTONCONTROLLER_H
#define BUTTONCONTROLLER_H

#include "ButtonPolicies.h"

/** General purpose button controller for up to four buttons.
    Controls the activation of the OK, Apply and Cancel buttons.
    Actually supports 4 buttons in all and it's up to the user to decide on
    the activation policy and which buttons correspond to which output of the
    state machine.
*/
template <class Policy>
class ButtonController : public noncopyable
{
public:
	/**@name Constructors and Deconstructors */
	//@{
	/** Constructor.
	    The cancel/close label entries are _not_ managed within the class
	    thereby allowing you to reassign at will and to use static labels.
	    It also means if you really don't want to have the Cancel button
	    label be different when there is nothing changed in the dialog then
	    you can just assign "Cancel" to both labels.  Or even reuse this
	    class for something completely different.
	 */
	ButtonController(char const * cancel, char const * close)
		: bp_(), okay_(0), apply_(0), cancel_(0), undo_all_(0),
		  cancel_label(cancel), close_label(close) {}
	/// Somebody else owns the FL_OBJECTs we just manipulate them.
	~ButtonController() {}
	//@}

	/**@name Initialise Button Functions */
	//@{
	/// Call refresh() when finished setting the buttons.
	void setOkay(FL_OBJECT * obj)
		{ okay_ = obj; }
	///
	void setApply(FL_OBJECT * obj)
		{ apply_ = obj; }
	///
	void setCancel(FL_OBJECT * obj)
		{ cancel_ = obj; }
	///
	void setUndoAll(FL_OBJECT * obj)
		{ undo_all_ = obj; }
	///
	void setCancelTrueLabel(char const * c)
		{ cancel_label = c; }
	///
	void setCancelFalseLabel(char const * c)
		{ close_label = c; }
	//@}

	/**@name Action Functions */
	//@{
	///
	void input(ButtonPolicy::SMInput in)
		{
			bp_.input(in);
			refresh();
		}
	///
	void okay()
		{ input(ButtonPolicy::SMI_OKAY); }
	///
	void apply()
		{ input(ButtonPolicy::SMI_APPLY); }
	///
	void cancel()
		{ input(ButtonPolicy::SMI_CANCEL); }
	///
	void undoAll()
		{ input(ButtonPolicy::SMI_UNDO_ALL); }
	///
	void hide()
		{ input(ButtonPolicy::SMI_HIDE); }
	/// Passthrough function -- returns its input value
	bool read_only(bool ro = true)
		{
			if (ro) {
				input(ButtonPolicy::SMI_READ_ONLY);
			} else {
				input(ButtonPolicy::SMI_READ_WRITE);
			}
			return ro;
		}
	///
	void read_write()
		{ read_only(false); }
	/// Passthrough function -- returns its input value
	bool valid(bool v = true)
		{ 
			if (v) {
				input(ButtonPolicy::SMI_VALID);
			} else {
				input(ButtonPolicy::SMI_INVALID);
			}
			return v;
		}
	///
	void invalid()
		{ valid(false); }
	/// force a refresh of the buttons
	void refresh()
		{
			if (okay_) {
				if (bp_.buttonStatus(ButtonPolicy::OKAY)) {
					fl_activate_object(okay_);
					fl_set_object_lcol(okay_, FL_BLACK);
				} else {
					fl_deactivate_object(okay_);
					fl_set_object_lcol(okay_, FL_INACTIVE);
				}
			}
			if (apply_) {
				if (bp_.buttonStatus(ButtonPolicy::APPLY)) {
					fl_activate_object(apply_);
					fl_set_object_lcol(apply_, FL_BLACK);
				} else {
					fl_deactivate_object(apply_);
					fl_set_object_lcol(apply_, FL_INACTIVE);
				}
			}
			if (undo_all_) {
				if (bp_.buttonStatus(ButtonPolicy::UNDO_ALL)) {
					fl_activate_object(undo_all_);
					fl_set_object_lcol(undo_all_, FL_BLACK);
				} else {
					fl_deactivate_object(undo_all_);
					fl_set_object_lcol(undo_all_,
							   FL_INACTIVE);
				}
			}
			if (cancel_) {
				if (bp_.buttonStatus(ButtonPolicy::CANCEL)) {
					fl_set_object_label(cancel_,
							    cancel_label);
				} else {
					fl_set_object_label(cancel_,
							    close_label);
				}
			}
		}
	//@}
private:
	///
	Policy bp_;
	/**@name Button Widgets */
	//@{
	///
	FL_OBJECT * okay_;
	///
	FL_OBJECT * apply_;
	///
	FL_OBJECT * cancel_;
	///
	FL_OBJECT * undo_all_;
	//@}
	/**@name Cancel/Close Button Labels */
	//@{
	///
	char const * cancel_label;
	///
	char const * close_label;	
	//@}
};

#endif
