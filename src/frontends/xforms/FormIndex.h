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

#ifndef FORMINDEX_H
#define FORMINDEX_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormInset.h"
struct FD_form_index;

/** This class provides an XForms implementation of the FormIndex Dialog.
 */
class FormIndex : public FormCommand {
public:
	///
	FormIndex(LyXView *, Dialogs *);
	///
	~FormIndex();
private:
	/// Build the dialog
	virtual void build();
	/// Update dialog before showing it
	virtual void update(bool switched = false);
	/// Apply from dialog (modify or create inset)
	virtual void apply();
	/// Pointer to the actual instantiation of the xform's form
	virtual FL_FORM * form() const;
	///
	FD_form_index * build_index();
	/// 
	int minh;
	///
	int minw;
	/// Real GUI implementation.
	FD_form_index * dialog_;
};

#endif
