// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-2001 The LyX Team.
 *
 *======================================================*/
/* FormTabular.h
 * FormTabular Interface Class
 */

#ifndef FORMTABULAR_H
#define FORMTABULAR_H

#include <boost/smart_ptr.hpp>

#ifdef __GNUG__
#pragma interface
#endif

#include "FormInset.h"

class InsetTabular;
struct FD_form_tabular;
struct FD_form_tabular_options;
struct FD_form_column_options;
struct FD_form_cell_options;
struct FD_form_longtable_options;

/** This class provides an XForms implementation of the FormTabular Dialog.
    The tabular dialog allows users to set/save their tabular.
 */
class FormTabular : public FormInset {
public:
	/// #FormTabular x(LyXFunc ..., Dialogs ...);#
	FormTabular(LyXView *, Dialogs *);

private:
	/// Pointer to the actual instantiation of the ButtonController.
	virtual xformsBC & bc();
	/** Redraw the form (on receipt of a Signal indicating, for example,
	    that the xforms colours have been re-mapped). */
	virtual void redraw();
	/// Disconnect signals. Also perform any necessary housekeeping.
	virtual void disconnect();

	/// Slot launching dialog to an existing inset
	void showInset(InsetTabular *);
	/// Slot launching dialog to an existing inset
	void updateInset(InsetTabular *);
	/// Update dialog before showing it
	virtual void update();
	/// Build the dialog
	virtual void build();
	/// Filter the inputs
	virtual bool input(FL_OBJECT *, long);
	/// Pointer to the actual instantiation of the xforms form
	virtual FL_FORM * form() const;

	/// Fdesign generated methods
	FD_form_tabular * build_tabular();
	///
	FD_form_tabular_options * build_tabular_options();
	///
	FD_form_column_options * build_column_options();
	///
	FD_form_cell_options * build_cell_options();
	///
	FD_form_longtable_options * build_longtable_options();

	/// Real GUI implementation.
	boost::scoped_ptr<FD_form_tabular> dialog_;
	///
	boost::scoped_ptr<FD_form_tabular_options> tabular_options_;
	///
	boost::scoped_ptr<FD_form_column_options> column_options_;
	///
	boost::scoped_ptr<FD_form_cell_options> cell_options_;
	///
	boost::scoped_ptr<FD_form_longtable_options> longtable_options_;

	/// pointer to the inset passed through showInset
	InsetTabular * inset_;
	///
	int actCell_;
	/// The ButtonController
	ButtonController<OkCancelReadOnlyPolicy, xformsBC> bc_;
};


inline
xformsBC & FormTabular::bc()
{
	return bc_;
}
#endif
