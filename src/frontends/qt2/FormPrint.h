// -*- C++ -*-
/**
 * \file FormPrint.h
 * Copyright 2001 LyX Team
 * see the file COPYING
 *
 * \author John Levon, moz@compsoc.man.ac.uk
 */
#ifndef FORMPRINT_H
#define FORMPRINT_H

#include "DialogBase.h"
#include "boost/utility.hpp"

class Dialogs;
class LyXView;
class PrintDlgImpl;

class FormPrint : public DialogBase {
public: 
	/**@name Constructors and Destructors */
	//@{
	///
	FormPrint(LyXView *, Dialogs *);
	/// 
	~FormPrint();
	//@}

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
	PrintDlgImpl * dialog_;

	/// the LyXView we belong to
	LyXView * lv_;
 
	/** Which Dialogs do we belong to?
	    Used so we can get at the signals we have to connect to.
	*/
	Dialogs * d_;
	
	/// Hide connection.
	SigC::Connection h_;
	/// Update connection.
	SigC::Connection u_;
};

#endif
