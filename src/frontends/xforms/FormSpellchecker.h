// -*- C++ -*-
/**
 * \file FormSpellchecker.h
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Edwin Leuven, leuven@fee.uva.nl
 */

#ifndef FORMSPELLCHECKER_H
#define FORMSPELLCHECKER_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBase.h"

class ControlSpellchecker;
struct FD_spellchecker;

/** This class provides an XForms implementation of the FormSpellchecker Dialog.
 */
class FormSpellchecker : public FormCB<ControlSpellchecker, FormDB<FD_spellchecker> > {
public:
	///
	FormSpellchecker(ControlSpellchecker &);

private:
	/// not needed.
	void apply() {}
	/// Build the dialog
	void build();
	///
	void update();

	/// enable/disable widgets when start/stop
	void stop(bool);

	/// update progress bar, set suggestions, exit message
	void partialUpdate(int);

	/// show an error message
	void showMessage(const char * msg);

	/// Filter the inputs
	virtual ButtonPolicy::SMInput input(FL_OBJECT *, long);
};

#endif // FORMSPELLCHECKER_H
