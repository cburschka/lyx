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

#include <boost/smart_ptr.hpp>

#ifdef __GNUG__
#pragma interface
#endif

#include "FormInset.h"
#include "buffer.h"

struct FD_form_toc;

/** This class provides an XForms implementation of the FormToc Dialog.
 */
class FormToc : public FormCommand {
public:
	///
	FormToc(LyXView *, Dialogs *);
private:
	/// Pointer to the actual instantiation of the ButtonController.
	virtual xformsBC & bc();
	/// Disconnect signals. Also perform any necessary housekeeping.
	virtual void disconnect();

	/// Build the dialog
	virtual void build();
	/// bool indicates if a buffer switch took place
	virtual void updateSlot(bool) { update(); }
	/// Update dialog before showing it
	virtual void update();
	/// Filter the inputs on callback from xforms
	virtual bool input( FL_OBJECT *, long);
	/// Pointer to the actual instantiation of the xforms form
	virtual FL_FORM * form() const;
	///
	void updateToc();
	///
	FD_form_toc * build_toc();

	///
	Buffer::SingleList toclist;
	/// Real GUI implementation.
	boost::scoped_ptr<FD_form_toc> dialog_;
	/// The ButtonController
	ButtonController<OkCancelPolicy, xformsBC> bc_;
};


inline
xformsBC & FormToc::bc()
{
	return bc_;
}
#endif
