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
	/// Create the dialog if necessary, update it and display it.
	void createInset( string const & );
	/// 
	void showInset( InsetCommand * const );
	/// 
	void show();
	/// Hide the dialog.
	void hide();
	/// Explicitly free the dialog.
	void free();
	///
	virtual void input( long ) = 0;
	///
	virtual void update() = 0;
	/// Apply from dialog
	virtual void apply() = 0;
	/// Build the dialog
	virtual void build() = 0;
	///
	virtual FL_FORM * const form() const = 0;
	//@}

	/**@name Private Data */
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
	/// Update connection.
	Connection u_;
	/// Hide connection.
	Connection h_;
	/// inset::hide connection.
	Connection ih_;
	///
	InsetCommand * inset_;
	///
	bool dialogIsOpen;
	///
  	InsetCommandParams params;
	///
  	string title;
	//@}
};

#endif
