/* FormParagraph.h
 * (C) 2000 LyX Team
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

#endif
