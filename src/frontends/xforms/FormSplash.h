// -*- C++ -*-
/**
 * \file FormSplash.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Allan Rae
 * \author John Levon
 */

#ifndef FORMSPLASH_H
#define FORMSPLASH_H

#include <boost/smart_ptr.hpp>

#ifdef __GNUG__
#pragma interface
#endif

#include "ViewBase.h"

class ControlSplash;
struct FD_form_splash;

/** The startup splash screen
 */
class FormSplash : public ViewSplash {
public:
	///
	FormSplash(ControlSplash &);

private:
	/// Build the dialog
	void build();
	/// hide the dialog
	void hide();
 	/// show the dialog
	void show();

	/// The parent controller
	ControlSplash & controller() const;

	/// Fdesign generated method
	FD_form_splash * build_splash();

	/// Real GUI implementation.
	boost::scoped_ptr<FD_form_splash> dialog_;
};

#endif // FORMSPLASH_H
