// -*- C++ -*-
/**
 * \file FormIndex.h
 * Copyright 2000-2001 the LyX Team
 * Read the file COPYING
 *
 * \author Angus Leeming, a.leeming@ic.ac.uk
 */

#ifndef FORMINDEX_H
#define FORMINDEX_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBase.h"

class ControlIndex;
struct FD_form_index;

/** This class provides an XForms implementation of the Index Dialog.
 */
class FormIndex : public FormCB<ControlIndex, FormDB<FD_form_index> > {
public:
	///
	FormIndex(ControlIndex &);

private:
	/// Set the Params variable for the Controller.
	virtual void apply();
	/// Build the dialog.
	virtual void build();
	/// Update dialog before/whilst showing it.
	virtual void update();

	/// Fdesign generated method
	FD_form_index * build_index();
};

#endif // FORMINDEX_H
