// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2000 The LyX Team.
 *
 * ======================================================
 */

#ifndef FORMBASE_H
#define FORMBASE_H

#include "DialogBase.h"
#include "LString.h"
#include "support/utility.hpp"
#include FORMS_H_LOCATION

class Dialogs;
class LyXView;

#ifdef __GNUG__
#pragma interface
#endif

/** This class is an XForms GUI base class
 */
class FormBase : public DialogBase, public noncopyable {
public:
	/// Constructor
	FormBase(LyXView *, Dialogs *, string const &);

	/// Callback functions
	static  int WMHideCB(FL_FORM *, void *);
	///
	static void ApplyCB(FL_OBJECT *, long);
	///
	static void CancelCB(FL_OBJECT *, long);
	///
	static void InputCB(FL_OBJECT *, long);
	///
	static void OKCB(FL_OBJECT *, long);

protected:
	/// Create the dialog if necessary, update it and display it.
	void show();
	/// Hide the dialog.
	void hide();

	/// Build the dialog
	virtual void build() = 0;
	/// Filter the inputs on callback from xforms
	virtual void input( long ) {}
	/// Update dialog before showing it
	virtual void update() {}
	/// Apply from dialog (modify or create inset)
	virtual void apply() {}
	/// delete derived class variables when hide()ing
	virtual void clearStore() {}
	/// Pointer to the actual instantiation of xform's form
	virtual FL_FORM * const form() const = 0;

	/** Which LyXFunc do we use?
	    We could modify Dialogs to have a visible LyXFunc* instead and
	    save a couple of bytes per dialog.
	*/
	LyXView * lv_;
	/** Which Dialogs do we belong to?
	    Used so we can get at the signals we have to connect to.
	*/
	Dialogs * d_;
private:
	/// Update connection.
	Connection u_;
	/// Hide connection.
	Connection h_;
	/// dialog title, displayed by WM.
  	string title;

protected:
	/// block opening of form twice at the same time
	bool dialogIsOpen;
};

#endif
