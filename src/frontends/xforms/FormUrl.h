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

#ifndef FORMURL_H
#define FORMURL_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormCommand.h"
struct FD_form_url;

/** This class provides an XForms implementation of the FormUrl Dialog.
 */
class FormUrl : public FormCommand {
public:
	/**@name Constructors and Destructors */
	//@{
	///
	FormUrl(LyXView *, Dialogs *);
	///
	~FormUrl();
	//@}

private:
	/**@name Slot Methods */
	//@{
	///
	virtual void update();
	/// Apply from dialog
	void apply();
	///
	virtual void input( long ) {};
	/// Build the dialog
	void build();
	///
	virtual FL_FORM * const form() const;
	///
	FD_form_url * build_url();
	//@}

	/// Real GUI implementation.
	FD_form_url * dialog_;
};

#endif
