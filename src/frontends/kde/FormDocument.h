/**
 * \file FormDocument.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */
#ifndef FORMDOCUMENT_H
#define FORMDOCUMENT_H

#include <vector>

#include "DialogBase.h"
#include "LString.h"
#include "boost/utility.hpp"

class Dialogs;
class LyXView;
class DocDialog;
class BufferParams;

/**
 * \brief the LyXian side of the complex Document dialog
 */
class FormDocument : public DialogBase {
public:
	FormDocument(LyXView *, Dialogs *);
 
	~FormDocument();

	/// Apply changes
	void apply();
	/// Update the dialog.
	void update(bool switched = false);
	/// close the connections
	void close();
 	/// change the doc class
	bool changeClass(BufferParams & params, int new_class);
	
private:
	/// Create the dialog if necessary, update it and display it.
	void show();
	/// Hide the dialog.
	void hide();

	/// Real GUI implementation.
	DocDialog * dialog_;

	/// the LyXView we belong to
	LyXView * lv_;

	/// Used so we can get at the signals we have to connect to.
	Dialogs * d_;
	
	/// Hide connection.
	Connection h_;
	
	/// is the buffer readonly ?
	bool readonly;
};

#endif // FORMDOCUMENT_H
