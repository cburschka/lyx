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

#include "FormBase.h"

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
	///
	~FormParagraph();

private:
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
	///
	void extra_update();
	///
	void general_apply();
	///
	void extra_apply();

	/// Typedefinitions from the fdesign produced Header file
	FD_form_tabbed_paragraph * build_tabbed_paragraph();
	///
	FD_form_paragraph_general * build_paragraph_general();
	///
	FD_form_paragraph_extra * build_paragraph_extra();

	/// Real GUI implementation.
	FD_form_tabbed_paragraph  * dialog_;
	///
	FD_form_paragraph_general * general_;
	///
	FD_form_paragraph_extra   * extra_;
};

#endif
