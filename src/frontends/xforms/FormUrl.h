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
	///
	FormUrl(LyXView *, Dialogs *);
	///
	~FormUrl();
private:
	/// Build the dialog
	virtual void build();
	/// Update dialog before showing it
	virtual void update();
	/// Apply from dialog (modify or create inset)
	virtual void apply();
	/// Pointer to the actual instantiation of the xform's form
	virtual FL_FORM * form() const;
	///
	FD_form_url * build_url();
	/// 
	int minh;
	///
	int minw;
	/// Real GUI implementation.
	FD_form_url * dialog_;
};

#endif
