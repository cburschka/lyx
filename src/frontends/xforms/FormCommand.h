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

#ifndef FORMCOMMAND_H
#define FORMCOMMAND_H

#include "DialogBase.h"
#include "LString.h"
#include "support/utility.hpp"
#include "insets/insetcommand.h"

class Dialogs;
class LyXView;

#ifdef __GNUG__
#pragma interface
#endif

/** This class is an XForms GUI base class to insets derived from
    InsetCommand
 */
class FormCommand : public DialogBase, public noncopyable {
public:
	/// Constructor
	FormCommand(LyXView *, Dialogs *, string const & );
	/// Destructor
	virtual ~FormCommand();

	/**@name Real per-instance Callback Methods */
	//@{
	static  int WMHideCB(FL_FORM *, void *);
	static void OKCB(FL_OBJECT *, long);
	static void ApplyCB(FL_OBJECT *, long);
	static void CancelCB(FL_OBJECT *, long);
	static void InputCB(FL_OBJECT *, long);
	//@}

protected:
	/**@name Slot Methods */
	//@{
	/// Slot launching dialog to (possibly) create a new inset
	void createInset( string const & );
	/// Slot launching dialog to an existing inset
	void showInset( InsetCommand * const );
	//@}

	/// Build the dialog
	virtual void build() = 0;
	/// Filter the inputs on callback from xforms
	virtual void input( long ) = 0;
	/// Update dialog before showing it
	virtual void update() = 0;
	/// Apply from dialog (modify or create inset)
	virtual void apply() = 0;
	/// delete derived class variables from hide()
	virtual void clearStore() {}
	/// Pointer to the actual instantiation of the xform's form
	virtual FL_FORM * const form() const = 0;

	/**@name Protected Data */
	//@{
	/** Which LyXFunc do we use?
	    We could modify Dialogs to have a visible LyXFunc* instead and
	    save a couple of bytes per dialog.
	*/
	LyXView * lv_;
	/** Which Dialogs do we belong to?
	    Used so we can get at the signals we have to connect to.
	*/
	Dialogs * d_;
	/// pointer to the inset passed through showInset (if any)
	InsetCommand * inset_;
	/// the nitty-griity. What is modified and passed back
  	InsetCommandParams params;
	//@}

private:
	/// Create the dialog if necessary, update it and display it.
	void show();
	/// Hide the dialog.
	void hide();
	/// Explicitly free the dialog.
	void free();


	/**@name Private Data */
	//@{
	/// Update connection.
	Connection u_;
	/// Hide connection.
	Connection h_;
	/// inset::hide connection.
	Connection ih_;
	/// block opening of form from more than one inset
	bool dialogIsOpen;
	/// dialog title, displayed by WM.
  	string title;
	//@}
};

#endif
