/* FormDocument.h
 * (C) 2001 LyX Team
 * John Levon, moz@compsoc.man.ac.uk
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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
class FormDocument : public DialogBase, public noncopyable {
public:
	/**@name Constructors and Destructors */
	//@{
	///
	FormDocument(LyXView *, Dialogs *);
	///
	~FormDocument();
	//@}

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

#endif
