// -*- C++ -*-
/**
 * \file FormSearch.h
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Edwin Leuven
 */

#ifndef FORMSEARCH_H
#define FORMSEARCH_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBase.h"

class ControlSearch;
struct FD_form_search;

/** This class provides an XForms implementation of the FormSearch Dialog.
 */
class FormSearch : public FormCB<ControlSearch, FormDB<FD_form_search> > {
public:
	///
	FormSearch(ControlSearch &);
   
private:
	/// not needed.
	virtual void apply() {}
	/// Build the dialog
	virtual void build();
	/// not needed.
	virtual void update() {}

	/// Filter the inputs
	virtual ButtonPolicy::SMInput input(FL_OBJECT *, long);
   
	/// Fdesign generated method
	FD_form_search  * build_search();
};

#endif // FORMSEARCH_H
