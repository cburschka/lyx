// -*- C++ -*-
/**
 * \file FormBranch.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Martin Vermeer
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef FORMBRANCH_H
#define FORMBRANCH_H


#include "FormDialogView.h"


class ControlBranch;
struct FD_branch;

/** This class provides an XForms implementation of the Branch Dialog.
 */
class FormBranch : public FormController<ControlBranch, FormView<FD_branch> > {
public:
	/// Constructor
	FormBranch(Dialog &);
private:
	/// 
	virtual void apply();
	/// Build the dialog
	virtual void build();
	/// Update dialog before showing it
	virtual void update();
};

#endif // FORMBRANCH_H
