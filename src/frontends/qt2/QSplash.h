// -*- C++ -*-
/**
 * \file QSplash.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Edwin Leuven
 */

#ifndef QSPLASH_H
#define QSPLASH_H

#include "DialogBase.h"
 
class Dialogs; 
class LyXView;
class QSplashDialog;

class QSplash : public DialogBase {

public:
	QSplash(LyXView *, Dialogs *);

	~QSplash();
   
  	/// hide the dialog
	void hide(); 

private:
	/// show the dialog
	void show(); 

	/// Real GUI implementation.
	QSplashDialog * dialog_;
	/// our container
	Dialogs * d_; 
	/// the show connection
	SigC::Connection c_; 
};

#endif // QSPLASH_H
