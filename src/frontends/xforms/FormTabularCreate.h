// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-2000 The LyX Team.
 *
 *======================================================*/
/* FormTabularCreate.h
 * FormTabularCreate Interface Class
 */

#ifndef FORMTABULARCREATE_H
#define FORMTABULARCREATE_H

#include <boost/smart_ptr.hpp>

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBaseDeprecated.h"
#include "xformsBC.h"

class LyXView;
class Dialogs;
struct FD_form_tabular_create;

/** This class provides an XForms implementation of the FormTabularCreate
    Dialog.
 */
class FormTabularCreate : public FormBaseBD {
public:
	/// #FormTabularCreate x(LyXView ..., Dialogs ...);#
	FormTabularCreate(LyXView *, Dialogs *);

private:
	/// Pointer to the actual instantiation of the ButtonController.
	virtual xformsBC & bc();
	/// Connect signals etc.
	virtual void connect();

	/// Apply from dialog
	virtual void apply();
	/// Update dialog before showing it
	virtual void update();
	/// Pointer to the actual instantiation of the xforms form
	virtual FL_FORM * form() const;
	/// Build the dialog
	virtual void build();

	///
	FD_form_tabular_create * build_tabular_create();
	
	/// Real GUI implementation.
	boost::scoped_ptr<FD_form_tabular_create> dialog_;
	/// The ButtonController
	ButtonController<OkApplyCancelReadOnlyPolicy, xformsBC> bc_;
};


inline
xformsBC & FormTabularCreate::bc()
{
	return bc_;
}
#endif
