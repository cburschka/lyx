// -*- C++ -*-
/**
 * \file FormRef.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef FORMREF_H
#define FORMREF_H


#include "FormDialogView.h"

class ControlRef;
struct FD_ref;

/** This class provides an XForms implementation of the FormRef Dialog.
 */
class FormRef : public FormController<ControlRef, FormView<FD_ref> > {
public:
	///
	FormRef(Dialog &);
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
	void switch_go_button();

	///
	bool at_ref_;
	///
	std::vector<string> refs_;
};

#endif // FORMREF_H
