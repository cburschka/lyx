// -*- C++ -*-
/**
 * \file xforms/FormRef.h
 * Copyright 2000-2001 the LyX Team
 * Read the file COPYING
 *
 * \author Angus Leeming, a.leeming@ic.ac.uk
 */

#ifndef FORMREF_H
#define FORMREF_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBase.h"

class ControlRef;
struct FD_ref;

/** This class provides an XForms implementation of the FormRef Dialog.
 */
class FormRef : public FormCB<ControlRef, FormDB<FD_ref> > {
public:
	///
	FormRef(ControlRef &, Dialogs &);
private:
	/// Set the Params variable for the Controller.
	virtual void apply();
	/// Build the dialog
	virtual void build();
	/// Filter the inputs on callback from xforms
	virtual ButtonPolicy::SMInput input(FL_OBJECT *, long);
	/// Update dialog before showing it
	virtual void update();

	///
	void updateBrowser(std::vector<string> const &) const;

	///
	bool at_ref_;
	///
	std::vector<string> refs_;
};

#endif // FORMREF_H
