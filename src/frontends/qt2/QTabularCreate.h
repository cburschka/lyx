// -*- C++ -*-
/**
 * \file QTabularCreate.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef QTABULARCREATE_H
#define QTABULARCREATE_H

#include "DialogBase.h"
#include "LString.h"
#include "boost/utility.hpp"

class Dialogs;
class LyXView; 
class QTabularCreateDialog;

class QTabularCreate : public DialogBase {
public: 
	QTabularCreate(LyXView *, Dialogs *);
	~QTabularCreate();

	/// create the table 
	void apply(int rows, int cols);
	/// close the connections
	void close();
 
private:
	/// Create the dialog if necessary, update it and display it.
	void show();
	/// Hide the dialog.
	void hide();
 
	/// Real GUI implementation.
	QTabularCreateDialog * dialog_;

	/// the LyXView we belong to
	LyXView * lv_;
 
	/** Which Dialogs do we belong to?
	    Used so we can get at the signals we have to connect to.
	*/
	Dialogs * d_;
	
	/// Hide connection.
	SigC::Connection h_;
};

#endif // QTABULARCREATE_H
