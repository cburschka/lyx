// -*- C++ -*-
/* FormCopyright.h
 * FormCopyright Interface Class
 * This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2000 The LyX Team.
 *
 * ======================================================
 */

#ifndef FORMCOPYRIGHT_H
#define FORMCOPYRIGHT_H

#include "DialogBase.h"
#include <gnome--/about.h>

class Dialogs;
// same arguement as in Dialogs.h s/LyX/UI/
class LyXView;

/** This class provides an GTK-- implementation of the FormCopyright Dialog.
 */
class FormCopyright : public DialogBase {
public:
	/**@name Constructors and Destructors */
	//@{
	/// #FormCopyright x(LyXFunc ..., Dialogs ...);#
	FormCopyright(LyXView *, Dialogs *);
	///
	~FormCopyright();
	//@}

private:
	FormCopyright() {}
	FormCopyright(FormCopyright &) : DialogBase() {}
	
	/**@name Slot Methods */
	//@{
	/// Create the dialog if necessary, update it and display it.
	void show();
	/// Hide the dialog.
	void hide();
	/// Not used but we've got to implement it.
	void update() {}
	//@}

	//@{
	/// Real GUI implementation.
	 Gnome::About * dialog_;
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
	/// Destroy connection.
	Connection destroy_;
	//@}
};

#endif


