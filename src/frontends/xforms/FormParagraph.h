// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright (C) 2000 The LyX Team.
 *
 *           @author Jürgen Vigna
 *
 *======================================================*/

#ifndef FORM_PARAGRAPH_H
#define FORM_PARAGRAPH_H

#include <boost/smart_ptr.hpp>

#ifdef __GNUG_
#pragma interface
#endif

#include "FormBaseDeprecated.h"

struct FD_form_tabbed_paragraph;
struct FD_form_paragraph_general;
struct FD_form_paragraph_extra;

/** This class provides an XForms implementation of the FormParagraph dialog.
 *           @author Jürgen Vigna
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

	///
	virtual FL_FORM * form() const;
	
	///
	void general_update();
	///
	void general_apply();

	/// Fdesign generated methods
	FD_form_tabbed_paragraph * build_tabbed_paragraph();
	///
	FD_form_paragraph_general * build_paragraph_general();
	///
	FD_form_paragraph_extra * build_paragraph_extra();
	/// Real GUI implementation.
	boost::scoped_ptr<FD_form_tabbed_paragraph> dialog_;
	///
	boost::scoped_ptr<FD_form_paragraph_general> general_;
	/// The ButtonController
	ButtonController<NoRepeatedApplyReadOnlyPolicy, xformsBC> bc_;
};


inline
xformsBC & FormParagraph::bc()
{
	return bc_;
}
#endif
