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

#ifndef FORMTOC_H
#define FORMTOC_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormCommand.h"
struct FD_form_toc;

/** This class provides an XForms implementation of the FormToc Dialog.
 */
class FormToc : public FormCommand, public noncopyable {
public:
	/**@name Constructors and Destructors */
	//@{
	///
	FormToc(LyXView *, Dialogs *);
	///
	~FormToc();

private:
	/**@name Slot Methods */
	//@{
	///
	virtual void update();
	/// Apply from dialog
	virtual void apply();
	///
	virtual void input( long ) {};
	/// Build the dialog
	virtual void build();
	///
	virtual FL_FORM * const form() const;
	///
	void updateToc();
	///
	FD_form_toc * build_toc();
	//@}

	/// Real GUI implementation.
	FD_form_toc * dialog_;
};

#endif
