/*
 * \file ControlCopyright.C
 * Copyright 2000-2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Allan Rae
 * \author Angus Leeming, a.leeming@.ac.uk
 */

#ifndef CONTROLCOPYRIGHT_H
#define CONTROLCOPYRIGHT_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlDialogs.h"

/** A controller for Copyright dialogs.
 */
class ControlCopyright : public ControlDialog<ControlConnectBI> {
public:
	///
	ControlCopyright(LyXView &, Dialogs &);

	///
	string const getCopyright() const;
	///
	string const getLicence() const;
	///
	string const getDisclaimer() const;

private:
	/// not needed.
	virtual void apply() {}
};

#endif // CONTROLCOPYRIGHT_H
