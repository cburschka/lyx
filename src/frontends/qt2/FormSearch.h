/** 
 * \file FormSearch.h
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 * 
 * \author Edwin Leuven
 */

#ifndef FORM_SEARCH_H
#define FORM_SEARCH_H

#include "DialogBase.h"
#include "LString.h"
#include "support/lstrings.h"

class LyXView;
class Dialogs;
class SearchDlgImpl;

class FormSearch : public DialogBase {
public:
	///
	FormSearch(LyXView *, Dialogs *);
	///
	~FormSearch();
	
	/// Close connections.
	void close();
	/// find stuff (we need access to lv_).
	void find(string const &, bool const &, bool const &, bool const &);
	/// replace stuff (we need access to lv_).
	void replace(string const &, string const &, 
		bool const &, bool const &, bool const &, bool const &);
	
	
private:
	
	/// Show the dialog.
	void show();
	/// Hide the dialog.
	void hide();
	/// Update the dialog.
	void update(bool switched = false);
	
	/// Real GUI implementation.
	SearchDlgImpl * dialog_;
	
	/// the LyXView we belong to.
	LyXView * lv_;
	
	/** Which Dialogs do we belong to?
	 *  Used so we can get at the signals we have to connect to.
	 */
	Dialogs * d_;
	
	/// Hide connection.
	SigC::Connection h_;

	/// Update connection.
	SigC::Connection u_;
	
};

#endif
