// -*- C++ -*-
/**
 * \file FormSpellchecker.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS
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
	FormSpellchecker();
private:
	/// not needed.
	void apply() {}
	/// Build the dialog
	void build();
	///
	void update();

	/// set suggestions and exit message
	void partialUpdate(int);

	/// Filter the inputs
	virtual ButtonPolicy::SMInput input(FL_OBJECT *, long);
};

#endif // FORMSPELLCHECKER_H
