// -*- C++ -*-
/**
 * \file FormParagraph.h
 * Copyright 2001 LyX Team
 * see the file COPYING
 *
 * \author John Levon, moz@compsoc.man.ac.uk
 */

#ifndef FORMPARAGRAPH_H
#define FORMPARAGRAPH_H

#include "DialogBase.h"
#include "LString.h"
#include "boost/utility.hpp"

class Dialogs;
class LyXView;
class ParagraphDlgImpl;

class FormParagraph : public DialogBase {
public: 
	/**@name Constructors and Destructors */
	//@{
	///
	FormParagraph(LyXView *, Dialogs *);
	/// 
	~FormParagraph();
	//@}

	/// Apply changes
	void apply();
	/// Update the dialog.
	void update(bool switched = false);
	/// Close the connections
	void close();
 
private: 
	/// Create the dialog if necessary, update it and display it.
	void show();
	/// Hide the dialog.
	void hide();

	/// Real GUI implementation.
	ParagraphDlgImpl * dialog_;

	/// the LyXView we belong to
	LyXView * lv_;
 
	/// Used so we can get at the signals we have to connect to.
	Dialogs * d_;
	
	/// Hide connection.
	SigC::Connection h_;
	
	/// readonly file or not
	bool readonly; 
};

#endif
