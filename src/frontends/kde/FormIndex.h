/**
 * \file FormIndex.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */
 
#ifndef FORMINDEX_H
#define FORMINDEX_H

#include "DialogBase.h"
#include "LString.h"
#include "boost/utility.hpp"
#include "insets/insetindex.h"

class Dialogs;
class LyXView;
class IndexDialog;

class FormIndex : public DialogBase {
public: 
	FormIndex(LyXView *, Dialogs *);

	~FormIndex();

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
	void update(bool = false);

	/// create an Index inset
	void createIndex(string const &);
	/// edit an Index  inset
	void showIndex(InsetCommand * const);
 
	/// Real GUI implementation.
	IndexDialog * dialog_;

	/// the LyXView we belong to
	LyXView * lv_;
 
	/// the Dialogs object we belong to
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

#endif // FORMINDEX_H
