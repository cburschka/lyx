// -*- C++ -*-
/**
 * \file ControlAboutLyX.h
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Edwin Leuven, leuven@fee.uva.nl
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#ifndef CONTROLABOUTLYX_H
#define CONTROLABOUTLYX_H

#include "Lsstream.h"

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlDialogs.h"

/** A controller for the About LyX dialogs.
 */
class ControlAboutlyx : public ControlDialog<ControlConnectBI> {
public:
	///
	ControlAboutlyx(LyXView &, Dialogs &);

	///
	stringstream & getCredits(stringstream &) const;
	
	///
	string const getCopyright() const;

	///
	string const getLicense() const;

	///
	string const getDisclaimer() const;

	///
	string const getVersion() const;
	

private:
	/// not needed.
	virtual void apply() {}
};

#endif // CONTROLABOUTLYX_H

