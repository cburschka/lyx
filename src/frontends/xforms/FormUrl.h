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

#include "FormInset.h"
struct FD_form_url;

/** This class provides an XForms implementation of the FormUrl Dialog.
 */
class FormUrl : public FormCommand {
public:
	///
	FormUrl(LyXView *, Dialogs *);
	///
	~FormUrl();
private:
	/// Connect signals etc. Set form's max size.
	virtual void connect();
	/// Build the dialog
	virtual void build();
	/// Update dialog before showing it
	virtual void update();
	/// Apply from dialog (modify or create inset)
	virtual void apply();
	/// Pointer to the actual instantiation of the xforms form
	virtual FL_FORM * form() const;
	///
	FD_form_url * build_url();
	/// Real GUI implementation.
	FD_form_url * dialog_;
};

#endif
