// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2000-2001 The LyX Team.
 *
 * ======================================================
 *
 * \file FormGraphics.h
 * \author Baruch Even, baruch.even@writeme.com
 * \author Herbert Voss, voss@lyx.org
 */

#ifndef FORMGRAPHICS_H
#define FORMGRAPHICS_H

#include <boost/smart_ptr.hpp>

#ifdef __GNUG__
#pragma interface
#endif 

#include "FormBase.h"
#include "RadioButtonGroup.h"

class ControlGraphics;
struct FD_form_graphics;
struct FD_form_file;
struct FD_form_size;
struct FD_form_special;

/** This class provides an XForms implementation of the Graphics Dialog.
 */
class FormGraphics : public FormCB<ControlGraphics, FormDB<FD_form_graphics> > {
public:
	///
	FormGraphics(ControlGraphics &);

private:

	/** Redraw the form (on receipt of a Signal indicating, for example,
	    that the xforms colours have been re-mapped). */
	virtual void redraw();
	/// Set the Params variable for the Controller.
	virtual void apply();
	/// Build the dialog.
	virtual void build();
	/// Update dialog before/whilst showing it.
	virtual void update();
	/// Filter the inputs on callback from xforms
	virtual ButtonPolicy::SMInput input(FL_OBJECT *, long);
	/// 
	void help();

	/// Verify that the input is correct. If not disable ok/apply buttons.
	ButtonPolicy::SMInput checkInput();

	/// Fdesign generated method
	FD_form_graphics * build_graphics();
	///
	FD_form_file * build_file();
	///
	FD_form_size * build_size();
	///
	FD_form_special * build_special();

	/// Real GUI implementation.
	boost::scoped_ptr<FD_form_file> file_;
	///
	boost::scoped_ptr<FD_form_size> size_;
	///
	boost::scoped_ptr<FD_form_special> special_;
};

#endif  // FORMGRAPHICS_H
