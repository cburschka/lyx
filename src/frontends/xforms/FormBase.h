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
#include <boost/utility.hpp>
#include FORMS_H_LOCATION
#include "ButtonController.h"
#include "gettext.h"
#include <sigc++/signal_system.h> // temporary

class Buffer;
class Dialogs;
class LyXView;

#ifdef __GNUG__
#pragma interface
#endif

/** This class is an XForms GUI base class.
    It is meant to be used solely as the parent class to FormBaseBI
    and FormBaseBD.
    @author Angus Leeming
 */

class FormBase : public DialogBase, public noncopyable {
public:
	/// Callback functions
	static  int WMHideCB(FL_FORM *, void *);
	///
	static void ApplyCB(FL_OBJECT *, long);
	///
	static void OKCB(FL_OBJECT *, long);
	///
	static void CancelCB(FL_OBJECT *, long);
	///
	static void InputCB(FL_OBJECT *, long);
	///
	static void RestoreCB(FL_OBJECT *, long);

protected: // methods
	///
	FormBase(LyXView *, Dialogs *, string const &,
		 ButtonPolicy *, char const *, char const *);
	///
	virtual ~FormBase();

	/** Redraw the form (on receipt of a Signal indicating, for example,
	    that the xform colours have been re-mapped).
	    Must be virtual because dialogs with tabbed folders will need to
	    redraw the form for each tab.
	*/
	virtual void redraw();

	/// Create the dialog if necessary, update it and display it.
	void show();
	/// Hide the dialog.
	virtual void hide();
	/// Update the dialog.
	virtual void update() {}
	/// Connect signals. Also perform any necessary initialisation.
	virtual void connect();
	/// Disconnect signals. Also perform any necessary housekeeping.
	virtual void disconnect() = 0;
	/// Build the dialog
	virtual void build() = 0;
	/** Filter the inputs on callback from xforms
	    Return true if inputs are valid.
	 */
	virtual bool input( FL_OBJECT *, long) {
		return true;
	}
	/// Apply from dialog (modify or create inset)
	virtual void apply() {}
	/// OK from dialog
	virtual void ok() {
		apply();
		hide();
	}
	/// Cancel from dialog
	virtual void cancel() {
		hide();
	}
	/// Restore from dialog
	virtual void restore() {
		update();
	}
	/// Pointer to the actual instantiation of xform's form
	virtual FL_FORM * form() const = 0;

	/** Which LyXFunc do we use?
	    We could modify Dialogs to have a visible LyXFunc* instead and
	    save a couple of bytes per dialog.
	*/
	LyXView * lv_;
	/// Useable even in derived-class's const functions.
	mutable ButtonController bc_;
	/// Used so we can get at the signals we have to connect to.
	Dialogs * d_;
	/// Hide connection.
	Connection h_;
	/// dialog title, displayed by WM.
  	string title;
private:
	///
	ButtonPolicy * bp_;
public:
	/// Overcome a dumb xforms sizing bug
	mutable int minw_;
	///
	mutable int minh_;
};


/** This class is an XForms GUI base class for Buffer Independent dialogs.
    Such dialogs do not require an update Connection although they may use
    an update() function which is also supported by restore().
 */
class FormBaseBI : public FormBase {
protected:
	/// Constructor
	FormBaseBI(LyXView *, Dialogs *, string const &,
		   ButtonPolicy * bp,
		   char const * close = N_("Close"),
		   char const * cancel = N_("Cancel"));

	/// Connect signals
	virtual void connect();
	/// Disconnect signals
	virtual void disconnect();
};


/** This class is an XForms GUI base class for Buffer Dependent dialogs
 */
class FormBaseBD : public FormBase {
protected:
	/// Constructor
	FormBaseBD(LyXView *, Dialogs *, string const &,
		   ButtonPolicy * bp,
		   char const * close = N_("Close"),
		   char const * cancel = N_("Cancel"));

	/// Connect signals
	virtual void connect();
	/// Disconnect signals
	virtual void disconnect();
	/// bool indicates if a buffer switch took place
	virtual void updateSlot(bool) { update(); }

	/// Update connection.
	Connection u_;
};


#endif
