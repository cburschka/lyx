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
#include "FormBaseDeprecated.h"
#include "xformsBC.h"
#if 1
#include "lyxparagraph.h"
#endif

#ifdef __GNUG_
#pragma interface
#endif

struct FD_form_tabbed_paragraph;
struct FD_form_paragraph_general;
struct FD_form_paragraph_extra;

/** This class provides an XForms implementation of the FormParagraph Popup.
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
	/// Build the popup
	virtual void build();
	/// Apply from popup
	virtual void apply();
	/// Update the popup.
	virtual void update();
	/// Filter the inputs on callback from xforms
	virtual bool input(FL_OBJECT * ob, long);

	///
	virtual FL_FORM * form() const;
	
	///
	void general_update();
#ifndef NO_PEXTRA
	///
	void extra_update();
#endif
	///
	void general_apply();
#ifndef NO_PEXTRA
	///
	void extra_apply();
#endif
	/// Typedefinitions from the fdesign produced Header file
	FD_form_tabbed_paragraph * build_tabbed_paragraph();
	///
	FD_form_paragraph_general * build_paragraph_general();
//#ifndef NO_PEXTRA
	///
	FD_form_paragraph_extra * build_paragraph_extra();
//#endif
	/// Real GUI implementation.
	boost::scoped_ptr<FD_form_tabbed_paragraph> dialog_;
	///
	boost::scoped_ptr<FD_form_paragraph_general> general_;
#ifndef NO_PEXTRA
	///
	boost::scoped_ptr<FD_form_paragraph_extra> extra_;
#endif
	/// The ButtonController
	ButtonController<NoRepeatedApplyReadOnlyPolicy, xformsBC> bc_;
};


inline
xformsBC & FormParagraph::bc()
{
	return bc_;
}
#endif
