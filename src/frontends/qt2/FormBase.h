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
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#ifndef FORMBASE_H
#define FORMBASE_H

#include <boost/smart_ptr.hpp>

class QDialog;

#ifdef __GNUG__
#pragma interface
#endif

#include "ViewBase.h"
#include "LString.h"
#include "ButtonPolicies.h"

class qt2BC;

/** This class is an Qt2 GUI base class.
 */
class FormBase : public ViewBC<qt2BC>
{
public:
	///
	FormBase(ControlBase &, string const &);
	///
	virtual ~FormBase() {}

protected:
	/// Build the dialog
	virtual void build() = 0;
	/// Hide the dialog.
	void hide();
	/// Create the dialog if necessary, update it and display it.
	void show();

private:
	/// Pointer to the actual instantiation of xform's form
	virtual QDialog* form() const = 0;
// 	/** Filter the inputs on callback from xforms
// 	    Return true if inputs are valid. */
// 	virtual ButtonPolicy::SMInput input(FL_OBJECT *, long);

private:
	/// dialog title, displayed by WM.
  	string title_;
};


template <class Dialog>
class FormDB: public FormBase
{
protected:
	///
	FormDB(ControlBase &, string const &);
	/// Pointer to the actual instantiation of the Qt dialog
	virtual QDialog* form() const;
	/// Real GUI implementation.
	boost::scoped_ptr<Dialog> dialog_;
};


template <class Dialog>
FormDB<Dialog>::FormDB(ControlBase & c, string const & t)
	: FormBase(c, t)
{}


template <class Dialog>
QDialog* FormDB<Dialog>::form() const
{
    return dialog_.get();
}


template <class Controller, class Base>
class FormCB: public Base
{
protected:
	///
	FormCB(ControlBase &, string const &);
	/// The parent controller
	Controller & controller() const;
};


template <class Controller, class Base>
FormCB<Controller, Base>::FormCB(ControlBase & c, string const & t)
	: Base(c, t)
{}


template <class Controller, class Base>
Controller & FormCB<Controller, Base>::controller() const
{
	return static_cast<Controller &>(controller_);
	//return dynamic_cast<Controller &>(controller_);
}


#endif // FORMBASE_H
