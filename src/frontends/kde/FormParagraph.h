/**
 * \file FormParagraph.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#ifndef FORMPARAGRAPH_H
#define FORMPARAGRAPH_H

#include "DialogBase.h"
#include "LString.h"
#include "boost/utility.hpp"

class Dialogs;
class LyXView;
class ParaDialog;

class FormParagraph : public DialogBase, public noncopyable {
public: 
	FormParagraph(LyXView *, Dialogs *);

	~FormParagraph();

	/// Apply changes
	void apply();
	/// Update the dialog.
	void update(bool switched = false);
	/// close the connections
	void close();
 
private: 
	/// Create the dialog if necessary, update it and display it.
	void show();
	/// Hide the dialog.
	void hide();

	/// Real GUI implementation.
	ParaDialog * dialog_;

	/// the LyXView we belong to
	LyXView * lv_;
 
	/// Used so we can get at the signals we have to connect to.
	Dialogs * d_;
	
	/// Hide connection.
	Connection h_;
	
	/// readonly file or not
	bool readonly; 
};

#endif // FORMPARAGRAPH_H
