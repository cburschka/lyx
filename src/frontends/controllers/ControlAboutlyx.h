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

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlDialog_impl.h"
#include "Lsstream.h"
#include "LString.h"

/** A controller for the About LyX dialogs.
 */
class ControlAboutlyx : public ControlDialogBI {
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

