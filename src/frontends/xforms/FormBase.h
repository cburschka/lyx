// -*- C++ -*-
/**
 * \file FormBase.h
 * Copyright 2000-2002 the LyX Team
 * Read the file COPYING
 *
 * \author Angus Leeming, a.leeming@ic.ac.uk
 */

/* A base class for the MCV-ed xforms dialogs.
 */

#ifndef FORMBASE_H
#define FORMBASE_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ViewBase.h"
#include "LString.h"
#include "ButtonPolicies.h"
#include "FeedbackController.h"

#include <boost/scoped_ptr.hpp>

#include FORMS_H_LOCATION // Can't forward-declare FL_FORM

class xformsBC;
class Tooltips;

/** This class is an XForms GUI base class.
 */
class FormBase : public ViewBC<xformsBC>, public FeedbackController
{
public:
	///
	FormBase(ControlButtons &, string const &, bool allowResize);
	///
	virtual ~FormBase();

	/** input callback function. invoked only by the xforms callback
	 *  interface
	 */
	void InputCB(FL_OBJECT *, long);

	Tooltips & tooltips();

protected:
	/// Build the dialog
	virtual void build() = 0;
	/// Hide the dialog.
	void hide();
	/// Create the dialog if necessary, update it and display it.
	void show();

	/** Prepare the way to:
	 *  1. display feedback as the mouse moves over ob. This feedback will
	 *  typically be rather more verbose than just a tooltip.
	 *  2. activate the button controller after a paste with the middle
	 *  mouse button.
	 */
	static void setPrehandler(FL_OBJECT * ob);

private:
	/// Pointer to the actual instantiation of xform's form
	virtual FL_FORM * form() const = 0;
	/// Filter the inputs on callback from xforms
	virtual ButtonPolicy::SMInput input(FL_OBJECT *, long);

	/** Redraw the form (on receipt of a Signal indicating, for example,
	 *  that the xform colors have been re-mapped).
	 */
	virtual void redraw();

	/// The dialog's minimum allowable dimensions.
	int minw_;
	///
	int minh_;
	/// Can the dialog be resized after it has been created?
	bool allow_resize_;
	/// dialog title, displayed by WM.
	string title_;
	///
	Tooltips * tooltips_;
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
	FormCB(Controller &, string const &, bool allowResize=true);
	/// The parent controller
	Controller & controller() const;
};


template <class Controller, class Base>
FormCB<Controller, Base>::FormCB(Controller & c, string const & t,
				 bool allowResize)
	: Base(c, t, allowResize)
{}


template <class Controller, class Base>
Controller & FormCB<Controller, Base>::controller() const
{
	return static_cast<Controller &>(ViewBase::controller_);
	//return dynamic_cast<Controller &>(ViewBase::controller_);
}


#endif // FORMBASE_H
