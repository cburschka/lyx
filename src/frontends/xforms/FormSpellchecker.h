// -*- C++ -*-
/**
 * \file FormSpellchecker.h
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Edwin Leuven
 */

#ifndef FORMSPELLCHECKER_H
#define FORMSPELLCHECKER_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBase.h"

class ControlSpellchecker;
struct FD_form_spellchecker;

/** This class provides an XForms implementation of the FormSpellchecker Dialog.
 */
class FormSpellchecker : public FormCB<ControlSpellchecker, FormDB<FD_form_spellchecker> > {
public:
	///
	FormSpellchecker(ControlSpellchecker &);
   
private:
	/// not needed.
	virtual void apply() {}
	/// Build the dialog
	virtual void build();
	/// not needed.
	virtual void update();

	/// update progress bar and suggestions
	void partialUpdate(int);

	/// show an error message
	void showMessage(const char * msg);

	/// line clicked in browser, necessart for double clicking
	int clickline_;
	
	/// Filter the inputs
	virtual ButtonPolicy::SMInput input(FL_OBJECT *, long);
   
	/// Fdesign generated method
	FD_form_spellchecker  * build_spellchecker();
};

#endif // FORMSPELLCHECKER_H
