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

#include "FormBase.h"

#ifdef __GNUG__
#pragma interface
#endif

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
	///
	~FormTabularCreate();

private:
	/// Connect signals etc.
	virtual void connect();

	/// Apply from dialog
	virtual void apply();
	/// Pointer to the actual instantiation of the xform's form
	virtual FL_FORM * form() const;
	/// Build the dialog
	virtual void build();

	///
	FD_form_tabular_create * build_tabular_create();
	
	/// Real GUI implementation.
	FD_form_tabular_create * dialog_;
};

#endif
