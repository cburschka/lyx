// -*- C++ -*-
/* FormCopyright.h
 * FormCopyright Interface Class
 * This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 *           This file Copyright 2000
 *           Allan Rae
 * ======================================================
 */

#ifndef FORMCOPYRIGHT_H
#define FORMCOPYRIGHT_H

#include "DialogBase.h"
#include "form_copyright.h"
#include "support/utility.hpp"

#ifdef __GNUG__
#pragma interface
#endif

class Dialogs;
class LyXView;
struct FD_form_copyright;

/** This class provides an XForms implementation of the FormCopyright Dialog.
 */
class FormCopyright : public DialogBase, public noncopyable {
public:
	/// #FormCopyright x(LyXFunc ..., Dialogs ...);#
	FormCopyright(LyXView *, Dialogs *);
	///
	~FormCopyright();

	///
	static  int WMHideCB(FL_FORM *, void *);
	///
	static void OKCB(FL_OBJECT *, long);
private:
	/// Create the dialog if necessary, update it and display it.
	void show();
	/// Hide the dialog.
	void hide();
	/// Not used but we've got to implement it.
	void update() {}

	/// Build the dialog
	void build();
	///
	FD_form_copyright * build_copyright();

	/// Real GUI implementation.
	FD_form_copyright * dialog_;
	/** Which LyXFunc do we use?
	    We could modify Dialogs to have a visible LyXFunc* instead and
	    save a couple of bytes per dialog.
	*/
	LyXView * lv_;
	/** Which Dialogs do we belong to?
	    Used so we can get at the signals we have to connect to.
	*/
	Dialogs * d_;
	/// Hide connection.
	Connection h_;
};

#endif

