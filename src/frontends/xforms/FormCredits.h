/**
 * \file FormCredits.h
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Edwin Leuven, leuven@fee.uva.nl
 * \author Angus Leeming, a.leeming@.ac.uk
 */

#ifndef FORMCREDITS_H
#define FORMCREDITS_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBase.h"

class ControlCredits;
struct FD_form_credits;

/** This class provides an XForms implementation of the FormCredits Dialog.
 */
class FormCredits : public FormCB<ControlCredits, FormDB<FD_form_credits> > {
public:
	///
	FormCredits(ControlCredits &);

private:
	/// not needed.
	virtual void apply() {}
	/// not needed.
	virtual void update() {}
	/// Build the dialog
	virtual void build();

	/// Fdesign generated method
	FD_form_credits * build_credits();
};

#endif

