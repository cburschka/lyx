// -*- C++ -*-
/**
 * \file Tooltips.h
 * Copyright 2000-2001 the LyX Team
 * Read the file COPYING
 *
 * \author Jürgen Vigna, jug@sad.it
 */

#ifndef FORM_PARAGRAPH_H
#define FORM_PARAGRAPH_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBaseDeprecated.h"

#include <boost/scoped_ptr.hpp>
#include <boost/signals/connection.hpp>

class Paragraph;
struct FD_form_paragraph;

/** This class provides an XForms implementation of the FormParagraph dialog.
 */
class FormParagraph : public FormBaseBD {
public:
	///
	FormParagraph(LyXView *, Dialogs *);
private:
	/// Pointer to the actual instantiation of the ButtonController.
	virtual xformsBC & bc();
	/** Redraw the form (on receipt of a Signal indicating, for example,
	    that the xforms colours have been re-mapped). */
	virtual void redraw();
	/// Build the dialog
	virtual void build();
	/// Apply from dialog
	virtual void apply();
	/// Update the dialog
	virtual void update();
	/// Filter the inputs on callback from xforms
	virtual bool input(FL_OBJECT * ob, long);
	/// Connect signals
	virtual void connect();
	/// Disconnect signals
	virtual void disconnect();
	///
	void changedParagraph();
	///
	Paragraph const * getCurrentParagraph() const;
	///
	virtual FL_FORM * form() const;

	/// Fdesign generated method
	FD_form_paragraph * build_paragraph();

	/// Real GUI implementation.
	boost::scoped_ptr<FD_form_paragraph> dialog_;

	/// The ButtonController
	ButtonController<NoRepeatedApplyReadOnlyPolicy, xformsBC> bc_;

	/// Changed Paragraph connection.
	boost::signals::connection cp_;

	/// The current Paragraph
	Paragraph const * par_;
};


inline
xformsBC & FormParagraph::bc()
{
	return bc_;
}
#endif
