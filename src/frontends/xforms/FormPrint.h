// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-2000 The LyX Team.
 *
 *	    This file copyright 1999-2000
 *	    Allan Rae
 *======================================================*/
/* FormPrint.h
 * FormPrint Interface Class
 */

#ifndef FORMPRINT_H
#define FORMPRINT_H

#include "DialogBase.h"
#ifdef SIGC_CXX_NAMESPACES
using SigC::Connection;
#endif

class LyXView;
class Dialogs;
struct FD_form_print;

/** This class provides an XForms implementation of the FormPrint Dialog.
    The print dialog allows users to print their documents.
 */
class FormPrint : public DialogBase {
public:
	/**@name Constructors and Destructors */
	//@{
	/// #FormPrint x(LyXFunc ..., Dialogs ...);#
	FormPrint(LyXView *, Dialogs *);
	///
	~FormPrint();
	//@}

	/**@name Real per-instance Callback Methods */
	//@{
	static  int WMHideCB(FL_FORM *, void *);
	static void OKCB(FL_OBJECT *, long);
	static void ApplyCB(FL_OBJECT *, long);
	static void CancelCB(FL_OBJECT *, long);
	static void InputCB(FL_OBJECT *, long);
	//@}

private:
	FormPrint() {}
	FormPrint(FormPrint &) : DialogBase() {}

	/**@name Slot Methods */
	//@{
	/// Create the dialog if necessary, update it and display it.
	void show();
	/// Hide the dialog.
	void hide();
	/// Update the dialog.
	void update();
	//@}

	/**@name Dialog internal methods */
	//@{
	/// Apply from dialog
	void apply();
	/// Filter the inputs
	void input();
	/// Build the dialog
	void build();
	///
	FD_form_print * build_print();
	/// Explicitly free the dialog.
	void free();
	//@}

	/**@name Private Data */
	//@{
	/// Real GUI implementation.
	FD_form_print * dialog_;
	/// Which LyXView do we belong to?
	LyXView * lv_;
	Dialogs * d_;
	/// Update connection.
	Connection u_;
	/// Hide connection.
	Connection h_;
	//@}
};

#endif
