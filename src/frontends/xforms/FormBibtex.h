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

class ControlBibtex;
struct FD_form_bibtex;

/**
 * For bibtex database setting
 */
class FormBibtex : public FormCB<ControlBibtex, FormDB<FD_form_bibtex> > {
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
