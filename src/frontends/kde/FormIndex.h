/* FormIndex.h
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

#ifndef FORMINDEX_H
#define FORMINDEX_H

#include "DialogBase.h"
#include "LString.h"
#include "support/utility.hpp"
#include "insets/insetindex.h"

class Dialogs;
class LyXView;
class FormIndexDialog;

class FormIndex : public DialogBase, public noncopyable {
public: 
	/**@name Constructors and Destructors */
	//@{
	///
	FormIndex(LyXView *, Dialogs *);
	/// 
	~FormIndex();
	//@}

	/// Apply changes
	void apply();
	/// close the connections
	void close();
 
private: 
	/// Create the dialog if necessary, update it and display it.
	void show();
	/// Hide the dialog.
	void hide();
	/// Update the dialog.
	void update();

	/// create an Index inset
	void createIndex(string const &);
	/// edit an Index  inset
	void showIndex(InsetCommand * const);
 
	/// Real GUI implementation.
	FormIndexDialog * dialog_;

	/// the LyXView we belong to
	LyXView * lv_;
 
	/** Which Dialogs do we belong to?
	    Used so we can get at the signals we have to connect to.
	*/
	Dialogs * d_;
	/// pointer to the inset if any
	InsetCommand * inset_;
	/// insets params
	InsetCommandParams params;
	/// is the inset we are reading from a readonly buffer ?
	bool readonly;
	
	/// Hide connection.
	Connection h_;
	/// Update connection.
	Connection u_;
	/// Inset hide connection.
	Connection ih_;
};

#endif
