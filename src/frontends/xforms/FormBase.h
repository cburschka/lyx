// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2000-2001 The LyX Team.
 *
 * ======================================================
 *
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#ifndef FORMBASE_H
#define FORMBASE_H

#include <boost/smart_ptr.hpp>
#include FORMS_H_LOCATION // Can't forward-declare FL_FORM

#ifdef __GNUG__
#pragma interface
#endif

#include "ViewBase.h"
#include "LString.h"
#include "ButtonPolicies.h"

class xformsBC;

/** This class is an XForms GUI base class.
 */
class FormBase : public ViewBC<xformsBC>
{
public:
	///
	FormBase(ControlButtons &, string const &, bool allowResize);
	///
	virtual ~FormBase() {}

	/// input callback function
	void InputCB(FL_OBJECT *, long);

	/// feedback callback function
	void FeedbackCB(FL_OBJECT *, int event);

protected:
	/// Build the dialog
	virtual void build() = 0;
	/// Hide the dialog.
	void hide();
	/// Create the dialog if necessary, update it and display it.
	void show();
	/** Set a prehandler for ob to:
	    1. display feedback as the mouse moves over it
	    2. activate the button controller after a paste with the middle
	    mouse button */
	void setPrehandler(FL_OBJECT * ob);

	/// post feedback for ob. Defaults to nothing
	virtual void feedback(FL_OBJECT * /* ob */) {}
	/// clear the feedback message
	virtual void clear_feedback() {}

	/** Flag that the message is a warning and should not be removed
	    when the mouse is no longer over the object */
	void setWarningPosted(bool);

private:
	/// Pointer to the actual instantiation of xform's form
	virtual FL_FORM * form() const = 0;
	/// Filter the inputs on callback from xforms 
	virtual ButtonPolicy::SMInput input(FL_OBJECT *, long);

	/** Redraw the form (on receipt of a Signal indicating, for example,
	    that the xform colors have been re-mapped). */
	virtual void redraw();

	/// The dialog's minimum allowable dimensions.
	int minw_;
	///
	int minh_;
	/// Can the dialog be resized after it has been created?
	bool allow_resize_;
	/// dialog title, displayed by WM.
  	string title_;
	/** Variable used to decide whether to remove the existing feedback
	    message or not (if it is infact a warning) */
	bool warning_posted_;
};


template <class Dialog>
class FormDB: public FormBase
{
protected:
	///
	FormDB(ControlButtons &, string const &, bool allowResize=true);
	/// Pointer to the actual instantiation of xform's form
	virtual FL_FORM * form() const;
	/// Real GUI implementation.
	boost::scoped_ptr<Dialog> dialog_;
};


template <class Dialog>
FormDB<Dialog>::FormDB(ControlButtons & c, string const & t, bool allowResize)
	: FormBase(c, t, allowResize)
{}


template <class Dialog>
FL_FORM * FormDB<Dialog>::form() const
{
	if (dialog_.get()) return dialog_->form;
	return 0;
}


template <class Controller, class Base>
class FormCB: public Base
{
protected:
	///
	FormCB(ControlButtons &, string const &, bool allowResize=true);
	/// The parent controller
	Controller & controller() const;
};


template <class Controller, class Base>
FormCB<Controller, Base>::FormCB(ControlButtons & c, string const & t,
				 bool allowResize)
	: Base(c, t, allowResize)
{}


template <class Controller, class Base>
Controller & FormCB<Controller, Base>::controller() const
{
	return static_cast<Controller &>(controller_);
	//return dynamic_cast<Controller &>(controller_);
}


#endif // FORMBASE_H
