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

#ifdef __GNUG__
#pragma interface
#endif

#include "FormCommand.h"
struct FD_form_url;

/** This class provides an XForms implementation of the FormUrl Dialog.
 */
class FormUrl : public FormCommand {
public:
	/**@name Constructors and Destructors */
	//@{
	///
	FormUrl(LyXView *, Dialogs *);
	///
	~FormUrl();
	//@}

private:
	/// Build the dialog
	virtual void build();
	/// Not used but must be instantiated
	virtual void input( long ) {}
	/// Update dialog before showing it
	virtual void update();
	/// Apply from dialog (modify or create inset)
	virtual void apply();
	/// Pointer to the actual instantiation of the xform's form
	virtual FL_FORM * const form() const;
	///
	FD_form_url * build_url();

	/// 
	int minh, minw;
	/// Real GUI implementation.
	FD_form_url * dialog_;
};

#endif
