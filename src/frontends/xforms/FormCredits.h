/**
 * \file FormCredits.h
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Edwin Leuven, leuven@fee.uva.nl
 */

#ifndef FORMCREDITS_H
#define FORMCREDITS_H

#include "FormBase.h"

#ifdef __GNUG__
#pragma interface
#endif

struct FD_form_credits;

/** This class provides an XForms implementation of the FormCredits Dialog.
 */
class FormCredits : public FormBaseBI {
public:
	/// #FormCopyright x(LyXFunc ..., Dialogs ...);#
	FormCredits(LyXView *, Dialogs *);
	///
	~FormCredits();

private:
	/// Build the dialog
	virtual void build();
	/// Pointer to the actual instantiation of the xforms form
	virtual FL_FORM * form() const;
	/// Fdesign generated method
	FD_form_credits * build_credits();

	/// Real GUI implementation.
	FD_form_credits * dialog_;
};

#endif

