// -*- C++ -*-
/**
 * \file xforms/FormTabularCreate.h
 * Copyright 1995 Matthias Ettrich
 * Copyright 1995-2001 the LyX Team
 * Read the file COPYING
 *
 * \author Jürgen Vigna, jug@sad.it
 */

#ifndef FORMTABULARCREATE_H
#define FORMTABULARCREATE_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBase.h"

class ControlTabularCreate;
struct FD_tabular_create;

/** This class provides an XForms implementation of the TabularCreate
    Dialog.
 */
class FormTabularCreate :
	public FormCB<ControlTabularCreate, FormDB<FD_tabular_create> > {
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
};

#endif // FORMTABULARCREATE
