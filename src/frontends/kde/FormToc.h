/* FormToc.h
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

#ifndef FORMTOC_H
#define FORMTOC_H

#include "DialogBase.h"
#include "LString.h"
#include "support/utility.hpp"
#include "insets/insetcommand.h" 
#include "buffer.h" 

class Dialogs;
class FormTocDialog;

class FormToc : public DialogBase, public noncopyable {
public: 
	/**@name Constructors and Destructors */
	//@{
	///
	FormToc(LyXView *, Dialogs *);
	/// 
	~FormToc();
	//@}

	/// Selected a tree item
	void select(const char *);
	/// Choose which type
	void set_type(Buffer::TocType);
	/// Update the dialog.
	void update();
	/// close the connections
	void close();
 
private: 
	/// Create the dialog if necessary, update it and display it.
	void show();
	/// Hide the dialog.
	void hide();

	/// create a Toc inset
	void createTOC(string const &);
	/// view a Toc inset
	void showTOC(InsetCommand * const);
	 
	/// hierarchical tree
	int doTree(vector < Buffer::TocItem>::const_iterator & , int, int, int);
	/// update the Toc
	void updateToc(void);

	/// set the type
	void setType(Buffer::TocType);
 
	/// Real GUI implementation.
	FormTocDialog * dialog_;

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
	
	/// Hide connection.
	Connection h_;
	/// Update connection.
	Connection u_;
	/// Inset hide connection.
	Connection ih_;

	/// the toc list
	std::vector <Buffer::TocItem> toclist;

	/// type currently being shown
	Buffer::TocType type;
};

#endif
