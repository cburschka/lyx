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

#ifdef __GNUG__
#pragma interface
#endif

#include "DialogBase.h"
 
struct FD_form_splash;
class Dialogs; 
class LyXView;

/** The startup splash screen
 */
class FormSplash : public DialogBase {
public:
	FormSplash(LyXView *, Dialogs *);

	~FormSplash();

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
	FD_form_splash * dialog_;
	/// our container
	Dialogs * d_; 
	/// the show connection
	Connection c_; 
};

#endif // FORMSPLASH_H
