// -*- C++ -*-
/**
 * \file xforms/FormToc.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming 
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef FORMTOC_H
#define FORMTOC_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBase.h"
#include "toc.h"

class ControlToc;
struct FD_toc;

/** This class provides an XForms implementation of the FormToc Dialog.
 */
class FormToc : public FormCB<ControlToc, FormDB<FD_toc> > {
public:
	///
	FormToc();
private:
	/// not needed
	virtual void apply() {}
	/// Build the dialog
	virtual void build();
	/// Update dialog before showing it
	virtual void update();
	/// Filter the inputs on callback from xforms
	virtual ButtonPolicy::SMInput input(FL_OBJECT *, long);

	///
	void updateType();
	///
	void updateContents();

	///
	toc::Toc toc_;
};

#endif // FORMTOC_H
