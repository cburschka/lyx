// -*- C++ -*-
/**
 * \file FormBase.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming 
 *
 * Full author contact details are available in file CREDITS
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

#include "forms_fwd.h"

class xformsBC;
class Tooltips;


/** This class is an XForms GUI base class.
 */
class FormBase : public ViewBase, public FeedbackController
{
public:
	///
	FormBase(string const &, bool allowResize);
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

	///
	xformsBC & bc();

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
	FormDB(string const &, bool allowResize=true);
	/// Pointer to the actual instantiation of xform's form
	virtual FL_FORM * form() const;
	/// Real GUI implementation.
	boost::scoped_ptr<Dialog> dialog_;
};


template <class Dialog>
FormDB<Dialog>::FormDB(string const & t, bool allowResize)
	: FormBase(t, allowResize)
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
	FormCB(string const &, bool allowResize = true);
	/// The parent controller
	Controller & controller();
	///
	Controller const & controller() const;
};


template <class Controller, class Base>
FormCB<Controller, Base>::FormCB(string const & t, bool allowResize)
	: Base(t, allowResize)
{}


template <class Controller, class Base>
Controller & FormCB<Controller, Base>::controller()
{
	return static_cast<Controller &>(getController());
}


template <class Controller, class Base>
Controller const & FormCB<Controller, Base>::controller() const
{
	return static_cast<Controller const &>(getController());
}


#endif // FORMBASE_H
