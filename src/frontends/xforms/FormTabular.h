// -*- C++ -*-
/**
 * \file FormTabular.h
 * Copyright 1995 Matthias Ettrich
 * Copyright 1995-2001 the LyX Team
 * Read the file COPYING
 *
 * \author Jürgen Vigna, jug@sad.it
 */

#ifndef FORMTABULAR_H
#define FORMTABULAR_H

#include <boost/scoped_ptr.hpp>

#ifdef __GNUG__
#pragma interface
#endif

#include "FormInset.h"

class InsetTabular;
struct FD_tabular;
struct FD_tabular_options;
struct FD_tabular_column;
struct FD_tabular_cell;
struct FD_tabular_longtable;

/** This class provides an XForms implementation of the FormTabular Dialog.
    The tabular dialog allows users to set/save their tabular.
 */
class FormTabular : public FormInset {
public:
	/// #FormTabular x(LyXFunc ..., Dialogs ...);#
	FormTabular(LyXView &, Dialogs &);

	/// Slot launching dialog to an existing inset
	void showInset(InsetTabular *);
	/// Slot launching dialog to an existing inset
	void updateInset(InsetTabular *);

private:
	/// Pointer to the actual instantiation of the ButtonController.
	virtual xformsBC & bc();
	/** Redraw the form (on receipt of a Signal indicating, for example,
	    that the xforms colours have been re-mapped). */
	virtual void redraw();
	/// Disconnect signals. Also perform any necessary housekeeping.
	virtual void disconnect();

	/// Update dialog before showing it
	virtual void update();
	/// Build the dialog
	virtual void build();
	/// Filter the inputs
	virtual bool input(FL_OBJECT *, long);
	/// Pointer to the actual instantiation of the xforms form
	virtual FL_FORM * form() const;
	///
	virtual int checkLongtableOptions(FL_OBJECT *, string &);

	/// Real GUI implementation.
	boost::scoped_ptr<FD_tabular> dialog_;
	///
	boost::scoped_ptr<FD_tabular_options> tabular_options_;
	///
	boost::scoped_ptr<FD_tabular_column> column_options_;
	///
	boost::scoped_ptr<FD_tabular_cell> cell_options_;
	///
	boost::scoped_ptr<FD_tabular_longtable> longtable_options_;

	/// pointer to the inset passed through showInset
	InsetTabular * inset_;
	///
	int actCell_;
	/// The ButtonController
	ButtonController<OkCancelReadOnlyPolicy, xformsBC> bc_;
	/// if we are applying stuff during a close of the dialog
	bool closing_;
};


inline
xformsBC & FormTabular::bc()
{
	return bc_;
}
#endif
