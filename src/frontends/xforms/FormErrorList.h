// -*- C++ -*-
/**
 * \file FormErrorList.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alfredo Braunstein
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef FORMERRORLIST_H
#define FORMERRORLIST_H


#include "FormDialogView.h"

class ControlErrorList;
struct FD_errorlist;

/** This class provides an XForms implementation of the FormErrorList Dialog.
 */
class FormErrorList : public FormController<ControlErrorList, FormView<FD_errorlist> > {
public:
	///
	FormErrorList(Dialog &);
private:
	/// not needed
	virtual void apply() {}
	/// Build the dialog
	virtual void build();
	/// Update dialog before showing it
	virtual void update();
	/// Filter the inputs on callback from xforms
	virtual ButtonPolicy::SMInput input(FL_OBJECT *, long);
	/// Goto to this error
	void goTo(int);
	///
	void updateContents();
};

#endif // FORMERRORLIST_H
