// -*- C++ -*-
/**
 * \file QCharacter.h
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Edwin Leuven
 */

#ifndef QCHARACTER_H
#define QCHARACTER_H

#include "DialogBase.h"

class LyXView;
class Dialogs;
class QCharacterDialog;

class QCharacter : public DialogBase {
public:
	///
	QCharacter(LyXView *, Dialogs *);
	///
	~QCharacter();

	/// Apply changes.
	void apply();
	/// Close connections.
	void close();

private:

	/// Show the dialog.
	void show();
	/// Hide the dialog.
	void hide();
	/// Update the dialog.
	void update(bool switched = false);

	/// Real GUI implementation.
	QCharacterDialog * dialog_;

	/// the LyXView we belong to.
	LyXView * lv_;

	/** Which Dialogs do we belong to?
	 *  Used so we can get at the signals we have to connect to.
	 */
	Dialogs * d_;

	/// is the buffer readonly?
	bool readonly;

	/// Hide connection.
	SigC::Connection h_;

	/// Update connection.
	SigC::Connection u_;
};

#endif // QCHARACTER_H
