// -*- C++ -*-
/**
 * \file FormWrap.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Dekel Tsur
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef FORMWRAP_H
#define FORMWRAP_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBase.h"
#include "RadioButtonGroup.h"

class ControlWrap;
struct FD_wrap;

/** This class provides an XForms implementation of the Wrap
    Dialog.
 */
class FormWrap
	: public FormCB<ControlWrap, FormDB<FD_wrap> > {
public:
	///
	FormWrap();
private:
	/// Set the Params variable for the Controller.
	virtual void apply();
	/// Build the dialog.
	virtual void build();
	/// Update dialog before/whilst showing it.
	virtual void update();

	/// placement
	RadioButtonGroup placement_;
};

#endif // FORMWRAP_H
