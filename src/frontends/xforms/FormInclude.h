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

#include "FormBase.h"
#include "insets/insetinclude.h"

class InsetInclude;
 
struct FD_form_include;

/** This class provides an XForms implementation of the FormInclude Dialog.
 */
class FormInclude : public FormBaseBD {
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
 
	/// Slot launching dialog to an existing inset
	void showInclude(InsetInclude *);

	/// Connect signals. Also perform any necessary initialisation.
	virtual void connect();
	/// Disconnect signals. Also perform any necessary housekeeping.
	virtual void disconnect();

	/// Build the dialog
	virtual void build();
	/// Filter the inputs
	virtual bool input( FL_OBJECT *, long );
	/// Update dialog before showing it
	virtual void update();
	/// Apply from dialog (modify or create inset)
	virtual void apply();
	/// Pointer to the actual instantiation of the xforms form
	virtual FL_FORM * form() const;
	/// bool indicates if a buffer switch took place
	virtual void updateSlot(bool);


	/// Type definition from the fdesign produced header file.
	FD_form_include * build_include();

	/// Real GUI implementation.
	FD_form_include * dialog_;
 
	/// inset::hide connection.
	Connection ih_;
 
	/// pointer to the inset passed through showInset
	InsetInclude * inset_;
	/// the nitty-gritty. What is modified and passed back
	InsetInclude::InsetIncludeParams params;
};

#endif
