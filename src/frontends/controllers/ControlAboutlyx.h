// -*- C++ -*-
/**
 * \file ControlAboutlyx.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CONTROLABOUTLYX_H
#define CONTROLABOUTLYX_H

#include "Dialog.h"
#include <iosfwd>

/** \c ControlAboutlyx is a controller for the "About LyX" dialogs.
 */
class ControlAboutlyx : public Dialog::Controller {
public:
	ControlAboutlyx(Dialog & parent);

	//@{
	/// Instantiate Dialog::Controller methods.
	virtual bool initialiseParams(std::string const &) { return true; }
	virtual void clearParams() {}
	virtual void dispatchParams() {}
	virtual bool isBufferDependent() const { return false; }
	//@}

	//@{
	/// Provide the View with specific pieces of information.
	void getCredits(std::ostream &) const;
	std::string const getCopyright() const;
	std::string const getLicense() const;
	std::string const getDisclaimer() const;
	std::string const getVersion() const;
	//@}
};

#endif // CONTROLABOUTLYX_H
