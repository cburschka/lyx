// -*- C++ -*-
/**
 * \file xforms/FormTabularCreate.h
 * Copyright 1995 Matthias Ettrich
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS
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
	FormTabularCreate();
private:
	/// Apply from dialog
	virtual void apply();
	/// Build the dialog
	virtual void build();
	/// not needed
	virtual void update() {};
};

#endif // FORMTABULARCREATE
