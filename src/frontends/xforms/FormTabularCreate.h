// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-2001 The LyX Team.
 *
 *======================================================
 *
 * \file FormTabularCreate.h
 */

#ifndef FORMTABULARCREATE_H
#define FORMTABULARCREATE_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBase.h"

class ControlTabularCreate;
struct FD_form_tabular_create;

/** This class provides an XForms implementation of the TabularCreate
    Dialog.
 */
class FormTabularCreate :
	public FormCB<ControlTabularCreate, FormDB<FD_form_tabular_create> > {
public:
	/// 
	FormTabularCreate(ControlTabularCreate &);

private:
	/// Apply from dialog
	virtual void apply();
	/// Build the dialog
	virtual void build();
	/// not needed
	virtual void update() {};

	///
	FD_form_tabular_create * build_tabular_create();
};

#endif // FORMTABULARCREATE
