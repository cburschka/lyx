/**
 * \file FormToc.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#ifndef FORMTOC_H
#define FORMTOC_H

#include "DialogBase.h"
#include "LString.h"
#include "boost/utility.hpp"
#include "insets/insetcommand.h"
#include "buffer.h"

class Dialogs;
class TocDialog;

class FormToc : public DialogBase {
public:
	FormToc(LyXView *, Dialogs *);
 
	~FormToc();

	/// Selected a tree item
	void select(char const *);
	/// Choose which type
	void set_type(Buffer::TocType);
	/// Update the dialog.
	void update(bool = false);
	/// change the depth shown
	void set_depth(int);
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
	
	/// update the Toc
	void updateToc(int);

	/// set the type
	void setType(Buffer::TocType);

	/// Real GUI implementation.
	TocDialog * dialog_;

	/// the LyXView we belong to
	LyXView * lv_;

	/// Dialogs object
	Dialogs * d_;
 
	/// pointer to the inset if any
	InsetCommand * inset_;
	/// insets params
	InsetCommandParams params;
	
	/// Hide connection.
	SigC::Connection h_;
	/// Update connection.
	SigC::Connection u_;
	/// Inset hide connection.
	SigC::Connection ih_;

	/// the toc list
	std::vector <Buffer::TocItem> toclist;

	/// type currently being shown
	Buffer::TocType type;

	/// depth of list shown
	int depth;
};

#endif // FORMTOC_H
