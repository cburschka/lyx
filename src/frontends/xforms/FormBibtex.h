// -*- C++ -*-
/**
 * \file FormBibtex.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Angus Leeming, a.leeming@ic.ac.uk
 * \author John Levon, moz@compsoc.man.ac.uk
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

private:
	/// Set the Params variable for the Controller.
	virtual void apply();
	/// Build the dialog.
	virtual void build();
	/// Update dialog before/whilst showing it.
	virtual void update();
	/// Filter the inputs on callback from xforms
	virtual ButtonPolicy::SMInput input(FL_OBJECT *, long);

	/// Fdesign generated method
	FD_form_bibtex * build_bibtex();
};

#endif // FORMBIBTEX_H
