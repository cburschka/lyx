/**
 * \file FormInclude.h
 * Copyright 2001 the LyX Team
 * See the file COPYING
 *
 * \author Alejandro Aguilar Sierra
 * \author John Levon
 */
#ifndef FORMINCLUDE_H
#define FORMINCLUDE_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormInset.h"

class InsetInclude;
 
struct FD_form_include;

/** This class provides an XForms implementation of the FormInclude Dialog.
 */
class FormInclude : public FormCommand {
public:
	///
	FormInclude(LyXView *, Dialogs *);
	///
	~FormInclude();
private:
	///
	enum State {
		/// the browse button
		BROWSE=0,
		/// the load file button
		LOAD=5,
		/// the verbatim radio choice
		VERBATIM=10,
		/// the input and include radio choices
		INPUTINCLUDE=11
	};
 
	/// Build the dialog
	virtual void build();
	/// Filter the inputs
	virtual bool input( FL_OBJECT *, long );
	/// Update dialog before showing it
	virtual void update();
	/// Apply from dialog (modify or create inset)
	virtual void apply();
	/// Pointer to the actual instantiation of the xform's form
	virtual FL_FORM * form() const;

	/// Type definition from the fdesign produced header file.
	FD_form_include * build_include();

	/// Real GUI implementation.
	FD_form_include * dialog_;
};

#endif
