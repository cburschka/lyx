// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2000 The LyX Team.
 *
 * ======================================================
 *
 * Author: Angus Leeming <a.leeming@ic.ac.uk>
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
	FormBase(ControlBase &, string const &);
	///
	virtual ~FormBase() {}

	/// input callback function
	void InputCB(FL_OBJECT *, long);

protected:
	/// Build the dialog
	virtual void build() = 0;
	/// Hide the dialog.
	void hide();
	/// Create the dialog if necessary, update it and display it.
	void show();

private:
	/// Pointer to the actual instantiation of xform's form
	virtual FL_FORM * form() const = 0;
	/** Filter the inputs on callback from xforms
	    Return true if inputs are valid. */
	virtual ButtonPolicy::SMInput input(FL_OBJECT *, long) = 0;

	/** Redraw the form (on receipt of a Signal indicating, for example,
	    that the xform colors have been re-mapped). */
	virtual void redraw();

protected:
	/// Overcome a dumb xforms sizing bug
	mutable int minw_;
	///
	mutable int minh_;

private:
	/// dialog title, displayed by WM.
  	string title_;
};


template <class Controller, class Dialog>
class FormBase2: public FormBase
{
protected:
	///
	FormBase2(ControlBase &, string const &);
	/// The parent controller
	Controller & controller() const;
	/// Pointer to the actual instantiation of xform's form
	virtual FL_FORM * form() const;
	/// Real GUI implementation.
	boost::scoped_ptr<Dialog> dialog_;
};


template <class Controller, class Dialog>
FormBase2<Controller, Dialog>::FormBase2(ControlBase & c, string const & t)
	: FormBase(c, t)
{}


template <class Controller, class Dialog>
Controller & FormBase2<Controller, Dialog>::controller() const
{
	return static_cast<Controller &>(controller_);
	//return dynamic_cast<Controller &>(controller_);
}


template <class Controller, class Dialog>
FL_FORM * FormBase2<Controller, Dialog>::form() const
{
	if (dialog_.get()) return dialog_->form;
	return 0;
}


#endif // FORMBASE_H
