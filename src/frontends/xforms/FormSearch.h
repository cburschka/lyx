// -*- C++ -*-
/**
 * \file FormSearch.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS
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
	FormSearch();
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
