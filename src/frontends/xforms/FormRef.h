/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2000-2001 The LyX Team.
 *
 * ======================================================
 *
 * \file FormRef.h
 * \author Angus Leeming, a.leeming@.ac.uk
 */

#ifndef FORMREF_H
#define FORMREF_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBase.h"

class ControlRef;
struct FD_form_ref;

/** This class provides an XForms implementation of the FormRef Dialog.
 */
class FormRef : public FormCB<ControlRef, FormDB<FD_form_ref> > {
public:
	///
	FormRef(ControlRef &);

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

	/// Fdesign generated method
	FD_form_ref * build_ref();

	///
	bool at_ref_;
	/// 
	std::vector<string> refs_;
};

#endif // FORMREF_H
