// -*- C++ -*-
/**
 * \file FormMathsBitmap.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Alejandro Aguilar Sierra
 * \author John Levon, moz@compsoc.man.ac.uk
 * \author Angus Leeming <leeming@lyx.org>
 */

#ifndef FORM_MATHSBITMAP_H
#define FORM_MATHSBITMAP_H

#include "LString.h"
#include "FormMathsPanel.h"

#include <vector>
#include "support/smart_ptr.h"

#ifdef __GNUG__
#pragma interface
#endif

/**
 * This class provides an XForms implementation of a maths bitmap form.
 */
class FormMathsBitmap : public FormMathsSub {
	///
	friend class FormMathsPanel;

public:
	///
	typedef lyx::shared_c_ptr<FL_OBJECT> bm_ptr;
	///
	typedef lyx::shared_c_ptr<FL_FORM> fl_ptr;

	///
	FormMathsBitmap(LyXView &, Dialogs & d, FormMathsPanel const &,
			string const &, std::vector<string> const &);
	///
	~FormMathsBitmap();
	///
	void addBitmap(int, int, int, int, int, unsigned char const *,
		       bool = true);

private:
	///
	int GetIndex(FL_OBJECT *);

	/// Build the dialog
	virtual void build();
	/// apply the data
	virtual void apply();
	/// input handler
	bool input(FL_OBJECT *, long);

	/// Pointer to the actual instantiation of the xforms form
	virtual FL_FORM * form() const;

	/// The latex names associated with each symbol
	std::vector<string> latex_;
	/// The latex name chosen
	string latex_chosen_;
	/// Real GUI implementation
	fl_ptr form_;
	/// The bitmap tables
	std::vector<bm_ptr> bitmaps_;

	/// Border width
	int ww_;
	///
	int x_;
	///
	int y_;
	///
	int w_;
	///
	int h_;
};


#endif // FORM_MATHSBITMAP_H
