/**
 * \file FormCredits.h
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Edwin Leuven, leuven@fee.uva.nl
 */

#ifndef FORMCREDITS_H
#define FORMCREDITS_H

#include <boost/smart_ptr.hpp>

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBaseDeprecated.h"

struct FD_form_credits;

/** This class provides an XForms implementation of the FormCredits Dialog.
 */
class FormCredits : public FormBaseBI {
public:
	/// #FormCopyright x(LyXFunc ..., Dialogs ...);#
	FormCredits(LyXView *, Dialogs *);

private:
	/// Pointer to the actual instantiation of the ButtonController.
	virtual xformsBC & bc();
	/// Build the dialog
	virtual void build();
	/// Pointer to the actual instantiation of the xforms form
	virtual FL_FORM * form() const;
	/// Fdesign generated method
	FD_form_credits * build_credits();

	/// Real GUI implementation.
	boost::scoped_ptr<FD_form_credits> dialog_;
	/// The ButtonController
	ButtonController<OkCancelPolicy, xformsBC> bc_;
};


inline
xformsBC & FormCredits::bc()
{
	return bc_;
}
#endif

