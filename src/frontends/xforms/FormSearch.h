// -*- C++ -*-
/**
 * \file FormSearch.h
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Edwin Leuven, leuven@fee.uva.nl
 */

#ifndef FORMSEARCH_H
#define FORMSEARCH_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBase.h"

class ControlSearch;
struct FD_search;

/** This class provides an XForms implementation of the FormSearch Dialog.
 */
class FormSearch : public FormCB<ControlSearch, FormDB<FD_search> > {
public:
	///
	FormSearch(ControlSearch &, Dialogs &);
private:
	/// not needed.
	virtual void apply() {}
	/// Build the dialog
	virtual void build();
	/// update the dialog
	virtual void update();

	/// Filter the inputs
	virtual ButtonPolicy::SMInput input(FL_OBJECT *, long);
};

#endif // FORMSEARCH_H
