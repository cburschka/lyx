/**
 * \file FormLog.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

 
#ifndef FORMLOG_H
#define FORMLOG_H

#include "DialogBase.h"
#include "LString.h"
#include "boost/utility.hpp"

class Dialogs;
class LyXView;
class LogDialog;

class FormLog : public DialogBase, public noncopyable {
public: 
	FormLog(LyXView *, Dialogs *);

	~FormLog();

	/// close the connections
	virtual void close();
	/// Update the dialog
	virtual void update();
	/// Update the dialog from slot
	virtual void supdate(bool = false);
 
protected: 
	/// Create the dialog if necessary, update it and display it
	virtual void show();
	/// Hide the dialog
	virtual void hide();

	/// Real GUI implementation.
	LogDialog * dialog_;

	/// the LyXView we belong to
	LyXView * lv_;
 
	/// the Dialogs object we belong to
	Dialogs * d_;
	
	/// Hide connection.
	Connection h_;
	/// Update connection.
	Connection u_;
};

#endif // FORMLOG_H
