// -*- C++ -*-
/**
 * \file FormChanges.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef FORMCHANGES_H
#define FORMCHANGES_H

#include "FormBase.h"

class ControlChanges;
struct FD_changes;

/**
 * This class provides an XForms implementation of the Merge Changes Dialog.
 */
class FormChanges : public FormCB<ControlChanges, FormDB<FD_changes> > {
public:
	FormChanges();

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

#endif // FORMCHANGES_H
