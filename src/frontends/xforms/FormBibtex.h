// -*- C++ -*-
/**
 * \file FormBibtex.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming 
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef FORMBIBTEX_H
#define FORMBIBTEX_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBase.h"

class ControlBibtex;
struct FD_bibtex;

/**
 * For bibtex database setting
 */
class FormBibtex : public FormCB<ControlBibtex, FormDB<FD_bibtex> > {
public:
	///
	FormBibtex();
private:
	/// Set the Params variable for the Controller.
	virtual void apply();
	/// Build the dialog.
	virtual void build();
	/// Update dialog before/whilst showing it.
	virtual void update();
	/// Filter the inputs on callback from xforms
	virtual ButtonPolicy::SMInput input(FL_OBJECT *, long);
};

#endif // FORMBIBTEX_H
