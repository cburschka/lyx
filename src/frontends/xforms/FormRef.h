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

#include "FormCommand.h"
struct FD_form_ref;

/** This class provides an XForms implementation of the FormRef Dialog.
 */
class FormRef : public FormCommand {
public:
	/**@name Constructors and Destructors */
	//@{
	///
	FormRef(LyXView *, Dialogs *);
	///
	~FormRef();
	//@}

private:
	///
	enum Type{ REF=0, PAGEREF, VREF, VPAGEREF, PRETTYREF };
	///
	enum Goto{ GOREF=0, GOBACK, GOFIRST };

	/// Build the dialog
	virtual void build();
	/// Filter the input
	virtual void input( long );
	/// Update dialog before showing it
	virtual void update();
	/// Not used but must be instantiated
	virtual void apply();
	/// delete derived class variables from hide()
	virtual void clearStore();
	/// Pointer to the actual instantiation of the xform's form
	virtual FL_FORM * const form() const;

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

	/// Real GUI implementation.
	FD_form_ref * dialog_;

	///
	Goto toggle;
	/// 
	std::vector<string> refs;
};

#endif
