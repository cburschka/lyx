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

#ifndef FORMURL_H
#define FORMURL_H

#include "DialogBase.h"
#include "LString.h"

class Dialogs;
class LyXView;
class InsetCommandParams;
class InsetUrl;
struct FD_form_url;

/** This class provides an XForms implementation of the FormUrl Dialog.
 */
class FormUrl : public DialogBase {
public:
	/**@name Constructors and Destructors */
	//@{
	///
	FormUrl(LyXView *, Dialogs *);
	///
	~FormUrl();
	//@}

	/**@name Real per-instance Callback Methods */
	//@{
	static  int WMHideCB(FL_FORM *, void *);
	static void OKCB(FL_OBJECT *, long);
	static void CancelCB(FL_OBJECT *, long);
	//@}

private:
	FormUrl() {}
	FormUrl(FormUrl &) : DialogBase() {}
	
	/**@name Slot Methods */
	//@{
	/// Create the dialog if necessary, update it and display it.
	void createInset( string const & );
	/// 
	void showInset( InsetUrl * );
	/// 
	void show();
	/// Hide the dialog.
	void hide();
	///
	void update();
	//@}

	/**@name Dialog internal methods */
	//@{
	/// Apply from dialog
	void apply();
	/// Build the dialog
	void build();
	///
	FD_form_url * build_url();
	/// Explicitly free the dialog.
	void free();
	//@}

	/**@name Private Data */
	//@{
	/// Real GUI implementation.
	FD_form_url * dialog_;
	/** Which LyXFunc do we use?
	    We could modify Dialogs to have a visible LyXFunc* instead and
	    save a couple of bytes per dialog.
	*/
	LyXView * lv_;
	/** Which Dialogs do we belong to?
	    Used so we can get at the signals we have to connect to.
	*/
	Dialogs * d_;
	/// Update connection.
	Connection u_;
	/// Hide connection.
	Connection h_;
	/// inset::hide connection.
	Connection ih_;
	///
	InsetUrl * inset_;
	///
	bool dialogIsOpen;
	///
  	InsetCommandParams * params;
	//@}
};

#endif
