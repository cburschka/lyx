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
#include "ButtonPolicies.h"
#include "forms_fwd.h"

#include "LString.h"
#include <boost/scoped_ptr.hpp>
#include <X11/Xlib.h> // for Pixmap

class xformsBC;
class Tooltips;


/** This class is an XForms GUI base class.
 */
class FormBase : public ViewBase
{
public:
	///
	FormBase(string const &, bool allowResize);
	///
	virtual ~FormBase();

	/** Input callback function.
	 *  Invoked only by the xforms callback interface
	 */
	void InputCB(FL_OBJECT *, long);

	/** Message callback function.
	 *  Invoked only by the xforms callback interface
	 */
	void MessageCB(FL_OBJECT *, int event);

	/** Prehandler callback function.
	 *  Invoked only by the xforms callback interface
	 */
	void PrehandlerCB(FL_OBJECT * ob, int event, int key);

	///
	Tooltips & tooltips();

protected:
	/// Build the dialog
	virtual void build() = 0;
	/// Hide the dialog.
	virtual void hide();
	/// Create the dialog if necessary, update it and display it.
	virtual void show();
	///
	virtual bool isVisible() const;

	/** Prepare the way to:
	 *  1. display feedback as the mouse moves over ob. This feedback will
	 *  typically be rather more verbose than just a tooltip.
	 *  2. activate the button controller after a paste with the middle
	 *  mouse button.
	 */
	static void setPrehandler(FL_OBJECT * ob);

	/** Pass the class a pointer to the message_widget so that it can
	    post the message */
	void setMessageWidget(FL_OBJECT * message_widget);

	/** Send the warning message from the daughter class to the
	    message_widget direct. The message will persist till the mouse
	    movesto a new object. */
	void postWarning(string const & warning);
	/// Reset the message_widget_
	void clearMessage();

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

	/** Called on the first show() request, initialising various bits and
	 *  pieces.
	 */
	void prepare_to_show();

	/** Get the feedback message for ob.
	    Called if warning_posted_ == false. */
	virtual string const getFeedback(FL_OBJECT * /* ob */)
		{ return string(); }

	/// Post the feedback message for ob to message_widget_
	void postMessage(string const & message);

	/** Variable used to decide whether to remove the existing feedback
	    message or not (if it is in fact a warning) */
	bool warning_posted_;
	/// The widget to display the feedback
	FL_OBJECT * message_widget_;

	/// The dialog's minimum allowable dimensions.
	int minw_;
	///
	int minh_;
	/// Can the dialog be resized after it has been created?
	bool allow_resize_;
	/// dialog title, displayed by the window manager.
	string title_;
	/// Passed to the window manager to give a pretty little symbol ;-)
	Pixmap icon_pixmap_;
	///
	Pixmap icon_mask_;
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
	return dialog_.get() ? dialog_->form : 0;
}


template <class Controller, class Base>
class FormCB: public Base
{
public:
	/// The parent controller
	Controller & controller();
	///
	Controller const & controller() const;

protected:
	///
	FormCB(string const &, bool allowResize = true);
};


template <class Controller, class Base>
FormCB<Controller, Base>::FormCB(string const & t, bool allowResize)
	: Base(t, allowResize)
{}


template <class Controller, class Base>
Controller & FormCB<Controller, Base>::controller()
{
	return static_cast<Controller &>(this->getController());
}


template <class Controller, class Base>
Controller const & FormCB<Controller, Base>::controller() const
{
	return static_cast<Controller const &>(this->getController());
}


#endif // FORMBASE_H
