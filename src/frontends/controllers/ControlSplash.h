/**
 * \file ControlSplash.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Allan Rae
 * \author John Levon
 * \author Angus Leeming, a.leeming@ic.ac.uk
 */

#ifndef CONTROLSPLASH_H
#define CONTROLSPLASH_H

#ifdef __GNUG__
#pragma interface
#endif

#include "DialogBase.h"
#include "LString.h"

class Dialogs;
class ViewSplash;

/** The startup splash screen
 */
class ControlSplash : public DialogBase {
public:
	///
	ControlSplash(Dialogs &);
	///
	string const & bannerFile() const;
	///
	string const & LyXVersion() const;
	/// hide (and destroy) the dialog
	void hide();

private:
	/** Allow the Controller to access the View. This method must be
	    instantiated in a daughter class that creates the actual instance
	    of the View. */
	virtual ViewSplash & view() = 0;

	/// show the dialog
	void show(); 

	/// our container
	Dialogs & d_; 
	/// the show connection
	SigC::Connection c_;

	///
	string banner_file_;
	///
	string version_;
};

#endif // CONTROLSPLASH_H
