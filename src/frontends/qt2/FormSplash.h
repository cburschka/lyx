/**
 * \file FormSplash.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Edwin Leuven
 */

#ifndef FORMSPLASH_H
#define FORMSPLASH_H

#include "DialogBase.h"
 
class Dialogs; 
class LyXView;
class FormSplashBase;

/** The startup splash screen
 */
class FormSplash : public DialogBase {

 public:
	FormSplash(LyXView *, Dialogs *);

	~FormSplash();
   
  	/// hide the dialog
	void hide(); 

 private:
	/// show the dialog
	void show(); 

	/// Real GUI implementation.
	FormSplashBase * dialog_;
	/// our container
	Dialogs * d_; 
	/// the show connection
	SigC::Connection c_; 
};

#endif // FORMSPLASH_H
