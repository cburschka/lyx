/**
 * \file FormTabularCreate.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#ifndef FORMTABULARCREATE_H
#define FORMTABULARCREATE_H

#include "DialogBase.h"

class Dialogs;
class LyXView; 
class TabularCreateDialog;

class FormTabularCreate : public DialogBase, public noncopyable {
public: 
	FormTabularCreate(LyXView *, Dialogs *);
 
	~FormTabularCreate();

	/// create the table 
	void apply(unsigned int rows, unsigned int cols);
	/// close the connections
	void close();
 
private:
	/// Create the dialog if necessary, update it and display it.
	void show();
	/// Hide the dialog.
	void hide();
 
	/// Real GUI implementation.
	TabularCreateDialog * dialog_;

	/// the LyXView we belong to
	LyXView * lv_;
 
	/// Dialogs object
	Dialogs * d_;
	
	/// Hide connection.
	Connection h_;
};

#endif // FORMTABULARCREATE_H
