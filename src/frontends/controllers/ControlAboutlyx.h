// -*- C++ -*-
/**
 * \file ControlAboutlyx.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef CONTROLABOUTLYX_H
#define CONTROLABOUTLYX_H

#include "Dialog.h"
#include <iosfwd>

/** A controller for the About LyX dialogs.
 */
class ControlAboutlyx : public Dialog::Controller {
public:
	///
	ControlAboutlyx(Dialog &);
	///
	virtual void initialiseParams(string const &) {}
	///
	virtual void clearParams() {}
	///
	virtual void dispatchParams() {}
	///
	virtual bool isBufferDependent() const { return false; }

	///
	void getCredits(std::ostream &) const;
	///
	string const getCopyright() const;
	///
	string const getLicense() const;
	///
	string const getDisclaimer() const;
	///
	string const getVersion() const;
};

#endif // CONTROLABOUTLYX_H
