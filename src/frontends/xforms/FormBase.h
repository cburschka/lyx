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

class Buffer;
class Dialogs;
class LyXView;

#ifdef __GNUG__
#pragma interface
#endif

/** This class is an XForms GUI base class
    @author Angus Leeming
 */
class FormBase : public DialogBase, public noncopyable {
public:
	///
	enum BufferDependency {
		///
		BUFFER_DEPENDENT,
		///
		BUFFER_INDEPENDENT
	};
	///
	enum ChangedBufferAction {
		///
		UPDATE,
		///
		HIDE
	};

	/** Constructor.
	    #FormBase(lv, d, _("DialogName"), BUFFER_DEPENDENT, new ButtonPolicy)#
	 */
	FormBase(LyXView *, Dialogs *, string const &,
		 BufferDependency, ChangedBufferAction,
		 ButtonPolicy * bp = new OkApplyCancelReadOnlyPolicy,
		 char const * close = N_("Close"),
		 char const * cancel = N_("Cancel"));
	///
	virtual ~FormBase();

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
	/// Create the dialog if necessary, update it and display it.
	void show();
	/// Hide the dialog.
	virtual void hide();
	/// Connect signals
	virtual void connect();
	/// Disconnect signals
	virtual void disconnect();
	/// Build the dialog
	virtual void build() = 0;
	/** Filter the inputs on callback from xforms
	    Return true if inputs are valid.
	 */
	virtual bool input( FL_OBJECT *, long ) {
		return true;
	}
	/// Update dialog before showing it
	virtual void update() {}
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
	/// delete derived class variables when hide()ing
	virtual void clearStore() {}
	/// Pointer to the actual instantiation of xform's form
	virtual FL_FORM * form() const = 0;

private: // methods
	/// method connected to updateBufferDependent signal.
	void updateOrHide();

protected: // data
	/// block opening of form twice at the same time.
	bool dialogIsOpen;
	/** Which LyXFunc do we use?
	    We could modify Dialogs to have a visible LyXFunc* instead and
	    save a couple of bytes per dialog.
	*/
	LyXView * lv_;
	/// Useable even in derived-class's const functions.
	mutable ButtonController bc_;

private: // data
	/// Used so we can get at the signals we have to connect to.
	Dialogs * d_;
	/// flag whether dialog is buffer dependent or not.
	BufferDependency const bd_;
	/// flag whether to hide or update on updateBufferDependent signal.
	ChangedBufferAction const cba_;
	/// stores parent buffer when popup was launched.
	Buffer * parent_;
	/// Update connection.
	Connection u_;
	/// Hide connection.
	Connection h_;
	/// dialog title, displayed by WM.
  	string title;
	///
	ButtonPolicy * bp_;
};

#endif
