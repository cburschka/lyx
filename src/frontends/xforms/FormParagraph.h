// -*- C++ -*-
/**
 * \file FormParagraph.h
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Jürgen Vigna, jug@sad.it
 */

#ifndef FORM_PARAGRAPH_H
#define FORM_PARAGRAPH_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBase.h"

struct FD_paragraph;
class ControlParagraph;

/** This class provides an XForms implementation of the FormParagraph dialog.
 */
class FormParagraph
	: public FormCB<ControlParagraph, FormDB<FD_paragraph> > {
public:
	///
	FormParagraph(ControlParagraph &);
private:
	/// Build the dialog
	virtual void build();
	/// Apply from dialog
	virtual void apply();
	/// Update the dialog
	virtual void update();
	
	/// Filter the inputs on callback from xforms
	virtual ButtonPolicy::SMInput input(FL_OBJECT *, long);
};

#endif
