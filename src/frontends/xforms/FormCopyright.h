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

#include <boost/smart_ptr.hpp>

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBaseDeprecated.h"

struct FD_form_copyright;

/** This class provides an XForms implementation of the FormCopyright Dialog.
 */
class FormCopyright : public FormBaseBI {
public:
	/// #FormCopyright x(LyXFunc ..., Dialogs ...);#
	FormCopyright(LyXView *, Dialogs *);
private:
	/// Pointer to the actual instantiation of the ButtonController.
	virtual xformsBC & bc();
	/// Build the dialog
	virtual void build();
	/// Pointer to the actual instantiation of the xforms form
	virtual FL_FORM * form() const;
	/// Fdesign generated method
	FD_form_copyright * build_copyright();

	/// Real GUI implementation.
	boost::scoped_ptr<FD_form_copyright> dialog_;
	/// The ButtonController
	ButtonController<OkCancelPolicy, xformsBC> bc_;
};


inline
xformsBC & FormCopyright::bc()
{
	return bc_;
}
#endif

