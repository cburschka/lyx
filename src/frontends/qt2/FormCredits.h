/**
 * \file FormCredits.h
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Edwin Leuven, leuven@fee.uva.nl
 * \author Angus Leeming, a.leeming@.ac.uk
 * \author Kalle Dalheimer, kalle@klaralvdalens-datakonsult.se
 */

#ifndef FORMCREDITS_H
#define FORMCREDITS_H

#ifdef __GNUG__
#pragma interface
#endif

#include "Qt2Base.h"

class ControlCredits;
class FormCreditsDialogImpl;

/** This class provides a Qt2 implementation of the FormCredits Dialog.
 */
class FormCredits : public Qt2CB<ControlCredits, Qt2DB<FormCreditsDialogImpl> > {
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
};

#endif

