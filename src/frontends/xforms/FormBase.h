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
class Tooltips;

/** This class is an XForms GUI base class.
 */
class FormBase : public ViewBC<xformsBC>, public SigC::Object
{
public:
	///
	enum TooltipLevel {
		NO_TOOLTIP,
		MINIMAL_TOOLTIP,
		VERBOSE_TOOLTIP
	};

	///
	FormBase(ControlButtons &, string const &, bool allowResize);
	///
	virtual ~FormBase();

	/** input callback function.
	    Invoked only by C_FormBaseInputCB and by C_FormBasePrehandler */
	void InputCB(FL_OBJECT *, long);
	/// feedback callback function, invoked only by C_FormBasePrehandler
	void FeedbackCB(FL_OBJECT *, int event);

	/** Return the tooltip dependent on the value of tooltip_level_
	    currently non-const becuase it gets connected to a SigC::slot */
	string getTooltip(FL_OBJECT const *);

protected:
	/// Build the dialog
	virtual void build() = 0;
	/// Hide the dialog.
	void hide();
	/// Create the dialog if necessary, update it and display it.
	void show();

	/// Prepare the way to produce a tooltip when the mouse is over ob.
	void setTooltipHandler(FL_OBJECT * ob);

	/** Prepare the way to:
	    1. display feedback as the mouse moves over ob. This feedback will
	    typically be rather more verbose than just a tooltip.
	    2. activate the button controller after a paste with the middle
	    mouse button */
	void setPrehandler(FL_OBJECT * ob);

	/** Flag that the message is a warning and should not be removed
	    when the mouse is no longer over the object.
	    Used in conjunction with setPrehandler(ob) and with feedback(ob),
	    clear_feedback(). */
	void setWarningPosted(bool);

	/** Fill the tooltips chooser with the standard descriptions
	    and set it to the tooltips_level_ */
	void fillTooltipChoice(FL_OBJECT *);
	/// Set tooltips_level_ from the chooser.
	void setTooltipLevel(FL_OBJECT *);
	
private:
	/// Pointer to the actual instantiation of xform's form
	virtual FL_FORM * form() const = 0;
	/// Filter the inputs on callback from xforms 
	virtual ButtonPolicy::SMInput input(FL_OBJECT *, long);

	/** Redraw the form (on receipt of a Signal indicating, for example,
	    that the xform colors have been re-mapped). */
	virtual void redraw();

	/// These methods can be overridden in the daughter classes.
	virtual string const getMinimalTooltip(FL_OBJECT const *) const
		{ return string(); }
	virtual string const getVerboseTooltip(FL_OBJECT const *) const
		{ return string(); }

	/// Post feedback for ob. Defaults to nothing
	virtual void feedback(FL_OBJECT * /* ob */) {}
	/// clear the feedback message
	virtual void clear_feedback() {}

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
	///
	Tooltips * tooltip_;
	/// How verbose are the tooltips?
	TooltipLevel tooltip_level_;
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
	return static_cast<Controller &>(controller_);
	//return dynamic_cast<Controller &>(controller_);
}


#endif // FORMBASE_H
