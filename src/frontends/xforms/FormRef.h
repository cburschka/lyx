// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2000 The LyX Team.
 *
 * ======================================================
 */

#ifndef FORMREF_H
#define FORMREF_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormInset.h"
struct FD_form_ref;

/** This class provides an XForms implementation of the FormRef Dialog.
 */
class FormRef : public FormCommand {
public:
	///
	FormRef(LyXView *, Dialogs *);
	///
	~FormRef();
private:
	///
	enum Type{
		///
		REF,
		///
		PAGEREF,
		///
		VREF,
		///
		VPAGEREF,
		///
		PRETTYREF
	};
	///
	enum Goto{
		///
		GOREF,
		///
		GOBACK,
		///
		GOFIRST
	};

	/// Connect signals etc. Set form's max size.
	virtual void connect();
	/// Disconnect signals. Also perform any necessary housekeeping.
	virtual void disconnect();

	/// Build the dialog
	virtual void build();
	/// Filter the input
	virtual bool input( FL_OBJECT *, long );
	/// Update dialog before showing it
	virtual void update(bool switched = false);
	/// Not used but must be instantiated
	virtual void apply();
	/// Pointer to the actual instantiation of the xform's form
	virtual FL_FORM * form() const;

	///
	void updateBrowser( std::vector<string> ) const;
	///
	void showBrowser() const;
	///
	void hideBrowser() const;
	///
	void setSize( int, int, int ) const;
	///
	FD_form_ref * build_ref();
	///
	Type getType() const;
	///
	string getName( Type type ) const;

	///
	Goto toggle;
	/// 
	std::vector<string> refs;

	/// Real GUI implementation.
	FD_form_ref * dialog_;
};

#endif
