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
#include "buffer.h"
struct FD_form_toc;

/** This class provides an XForms implementation of the FormToc Dialog.
 */
class FormToc : public FormCommand {
public:
	///
	FormToc(LyXView *, Dialogs *);
	///
	~FormToc();
private:
	/// Build the dialog
	virtual void build();
	/// Not used but must be instantiated
	virtual void input(long) {}
	/// Update dialog before showing it
	virtual void update();
	/// Apply from dialog (modify or create inset)
	virtual void apply();
	/// delete derived class variables from hide()
	virtual void clearStore();
	/// Pointer to the actual instantiation of the xform's form
	virtual FL_FORM * const form() const;
	///
	void updateToc();
	///
	FD_form_toc * build_toc();

	/// Real GUI implementation.
	FD_form_toc * dialog_;
	///
	std::vector<Buffer::TocItem> toclist;
};

#endif
