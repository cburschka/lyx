// -*- C++ -*-
/**
 * \file FormBibtex.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Angus Leeming
 * \author John Levon
 */

#ifndef FORMBIBTEX_H
#define FORMBIBTEX_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBase.h"

#include "form_bibtex.h"

class ControlBibtex;

/**
 * For bibtex database setting
 */
class FormBibtex : public FormBase2<ControlBibtex, FD_form_bibtex> {
public:
	///
	FormBibtex(ControlBibtex &);

	// Functions accessible to the Controller.

	/// Set the Params variable for the Controller.
	virtual void apply();
	/// Build the dialog.
	virtual void build();
	/// Update dialog before/whilst showing it.
	virtual void update();
	
private:
	/// Filter the inputs on callback from xforms
	virtual ButtonPolicy::SMInput input(FL_OBJECT *, long);
	///
	FD_form_bibtex * build_bibtex();
};

#endif // FORMBIBTEX_H
