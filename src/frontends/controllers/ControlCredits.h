/**
 * \file ControlCredits.h
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Edwin Leuven, leuven@fee.uva.nl
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#ifndef CONTROLCREDITS_H
#define CONTROLCREDITS_H

#include <vector>

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlDialogs.h"

/** A controller for the Credits dialogs.
 */
class ControlCredits : public ControlDialog<ControlConnectBI> {
public:
	///
	ControlCredits(LyXView &, Dialogs &);

	///
	std::vector<string> const getCredits() const;

private:
	/// not needed.
	virtual void apply() {}
};

#endif // CONTROLCREDITS_H

