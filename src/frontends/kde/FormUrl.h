/**
 * \file FormUrl.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#ifndef FORMURL_H
#define FORMURL_H

#include "DialogBase.h"
#include "LString.h"
#include "boost/utility.hpp"
#include "insets/inseturl.h"

class Dialogs;
class LyXView;
class UrlDialog;

class FormUrl : public DialogBase, public noncopyable {
public: 
	FormUrl(LyXView *, Dialogs *);

	~FormUrl();

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
	void update(bool switched = false);

	/// create a URL inset
	void createUrl(string const &);
	/// edit a URL inset
	void showUrl(InsetCommand * const);
 
	/// Real GUI implementation.
	UrlDialog * dialog_;

	/// the LyXView we belong to
	LyXView * lv_;
 
	/// Dialogs object
	Dialogs * d_;
 
	/// pointer to the inset if any
	InsetCommand * inset_;
	/// insets params
	InsetCommandParams params;
	/// is the inset we are reading from a readonly buffer
	bool readonly;
	
	/// Hide connection.
	Connection h_;
	/// Update connection.
	Connection u_;
	/// Inset hide connection.
	Connection ih_;
};

#endif // FORMURL_H
