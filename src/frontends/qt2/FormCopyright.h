/***************************************************************************
                          formcopyright.h  -  description
                             -------------------
    begin                : Thu Feb 3 2000
    copyright            : (C) 2000 by Jürgen Vigna
    email                : jug@sad.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef FORMCOPYRIGHT_H
#define FORMCOPYRIGHT_H

#include "DialogBase.h"
#include "boost/utility.hpp"

class Dialogs;
class LyXView;
class FormCopyrightDialog;

/**
  @author Jürgen Vigna
  */
class FormCopyright : public DialogBase, public noncopyable {
public: 
	FormCopyright(LyXView *, Dialogs *);
	~FormCopyright();

private: 
	/// Create the dialog if necessary, update it and display it.
	void show();
	/// Hide the dialog.
	void hide();

	/// Real GUI implementation.
	FormCopyrightDialog * dialog_;
	/** Which LyXFunc do we use?
	    We could modify Dialogs to have a visible LyXFunc* instead and
	    save a couple of bytes per dialog.
	*/
	LyXView * lv_;
	/** Which Dialogs do we belong to?
	    Used so we can get at the signals we have to connect to.
	*/
	Dialogs * d_;
	/// Hide connection.
	Connection h_;
};

#endif
