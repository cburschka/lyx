/**
 * \file FormCopyright.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Jürgen Vigna
 */
#ifndef FORMCOPYRIGHT_H
#define FORMCOPYRIGHT_H

#include "DialogBase.h"
#include "boost/utility.hpp"

class Dialogs;
class LyXView;
class CopyrightDialog;

class FormCopyright : public DialogBase, public noncopyable {
public: 
	FormCopyright(LyXView *, Dialogs *);
 
	~FormCopyright();

private: 
	/// Create the dialog if necessary, update it and display it
	void show();
	/// Hide the dialog
	void hide();
	/// Not used but we've got to implement it
	void update(bool) {}

	/// Real GUI implementation
	CopyrightDialog * dialog_;
	/// Which LyXFunc do we use ?
	LyXView * lv_;
	/// dialogs object
	Dialogs * d_;
	/// Hide connection.
	Connection h_;
};

#endif // FORMCOPYRIGHT_H
