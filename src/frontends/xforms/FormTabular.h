// -*- C++ -*-
/**
 * \file FormTabular.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef FORMTABULAR_H
#define FORMTABULAR_H

#include "FormDialogView.h"
#include <boost/scoped_ptr.hpp>

class InsetTabular;
class ControlTabular;
struct FD_tabular;
struct FD_tabular_options;
struct FD_tabular_column;
struct FD_tabular_cell;
struct FD_tabular_longtable;

/** This class provides an XForms implementation of the FormTabular Dialog.
    The tabular dialog allows users to set/save their tabular.
 */
class FormTabular
	: public FormController<ControlTabular, FormView<FD_tabular> > {
public:

	FormTabular(Dialog &);

private:
	/** Redraw the form (on receipt of a Signal indicating, for example,
	    that the xforms colours have been re-mapped). */
	virtual void redraw();

	/// not used
	virtual void apply() {}
	/// Update dialog before showing it
	virtual void update();
	/// Build the dialog
	virtual void build();
	/// Filter the inputs
	virtual ButtonPolicy::SMInput input(FL_OBJECT *, long);

	///
	virtual int checkLongtableOptions(FL_OBJECT *, std::string &);

	///
	boost::scoped_ptr<FD_tabular_options> tabular_options_;
	///
	boost::scoped_ptr<FD_tabular_column> column_options_;
	///
	boost::scoped_ptr<FD_tabular_cell> cell_options_;
	///
	boost::scoped_ptr<FD_tabular_longtable> longtable_options_;

	///
	bool closing_;

	///
	int actCell_;
};

#endif // FORMTABULAR_H
