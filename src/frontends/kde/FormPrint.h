/**
 * \file FormPrint.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#ifndef FORMPRINT_H
#define FORMPRINT_H

#include "DialogBase.h"
#include "boost/utility.hpp"

class Dialogs;
class LyXView;
class PrintDialog;

#ifdef SIGC_CXX_NAMESPACES
using SigC::Connection;
#endif

class FormPrint : public DialogBase {
public: 
	FormPrint(LyXView *, Dialogs *);

	~FormPrint();

	/// start print
	void print();
	/// close
	void close();
 
private: 
	/// Create the dialog if necessary, update it and display it.
	void show();
	/// Hide the dialog.
	void hide();
	/// Update the dialog.
	void update(bool = false);

	/// Real GUI implementation.
	PrintDialog * dialog_;

	/// the LyXView we belong to
	LyXView * lv_;
 
	/** Which Dialogs do we belong to?
	    Used so we can get at the signals we have to connect to.
	*/
	Dialogs * d_;
	
	/// Hide connection.
	Connection h_;
	/// Update connection.
	Connection u_;
};

#endif // FORMPRINT_H
