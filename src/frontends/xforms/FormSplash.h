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

#include "DialogBase.h"
#include "xformsBC.h"
 
struct FD_form_splash;
class Dialogs; 
class LyXView;

/** The startup splash screen
 */
class FormSplash : public DialogBase {
public:
	///
	FormSplash(LyXView *, Dialogs *);

	/// hide (and destroy) the dialog
	void hide();
 
private:
	/// show the dialog
	void show(); 
	/// Build the dialog
	void build();
	/// Fdesign generated method
	FD_form_splash * build_splash();

	/// Real GUI implementation.
	boost::scoped_ptr<FD_form_splash> dialog_;
	/// our container
	Dialogs * d_; 
	/// the show connection
	SigC::Connection c_; 
};

#endif // FORMSPLASH_H
