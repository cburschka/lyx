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

#include <boost/smart_ptr.hpp>
#include "FormInset.h"
#include "xformsBC.h"

#ifdef __GNUG__
#pragma interface
#endif

class InsetError;
struct FD_form_error;

/** This class provides an XForms implementation of the FormError Dialog.
 */
class FormError : public FormInset {
public:
	/// Constructor
	FormError(LyXView *, Dialogs *);
private:
	/// Pointer to the actual instantiation of the ButtonController.
	virtual xformsBC & bc();
	/// Disconnect signals. Also perform any necessary housekeeping.
	virtual void disconnect();

	/// Slot launching dialog to an existing inset
	void showInset(InsetError *);
	/// Update dialog before showing it
	virtual void update();
	/// Build the dialog
	virtual void build();
	/// Pointer to the actual instantiation of the xforms form
	virtual FL_FORM * form() const;
	/// Fdesign generated method
	FD_form_error * build_error();

	/// Real GUI implementation.
	boost::scoped_ptr<FD_form_error> dialog_;
	/// pointer to the inset passed through showInset
	InsetError * inset_;
	/// the error message
  	string message_;
	/// The ButtonController
	ButtonController<OkCancelPolicy, xformsBC> bc_;
};


inline
xformsBC & FormError::bc()
{
	return bc_;
}
#endif
