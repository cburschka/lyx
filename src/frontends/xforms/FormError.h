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

#ifndef FORMERROR_H
#define FORMERROR_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBase.h"

class InsetError;
struct FD_form_error;

/** This class provides an XForms implementation of the FormError Dialog.
 */
class FormError : public FormBase {
public:
	/// Constructor
	FormError(LyXView *, Dialogs *);
	///
	~FormError();
private:
	/// Slot launching dialog to an existing inset
	void showInset( InsetError * const );
	/// Update dialog before showing it
	virtual void update();
	/// Build the dialog
	virtual void build();
	/// Reset data when hide() is called
	virtual void clearStore();
	/// Pointer to the actual instantiation of the xform's form
	virtual FL_FORM * form() const;
	/// Fdesign generated method
	FD_form_error * build_error();

	/// Real GUI implementation.
	FD_form_error * dialog_;
	/// inset::hide connection.
	Connection ih_;
	/// the error message
  	string message_;
};

#endif
