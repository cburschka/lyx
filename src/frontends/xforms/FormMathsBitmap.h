// -*- C++ -*-
/**
 * \file FormMathsBitmap.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author John Levon
 * \author Angus Leeming 
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef FORM_MATHSBITMAP_H
#define FORM_MATHSBITMAP_H

#include "LString.h"
#include "FormMathsPanel.h"

#include <boost/shared_ptr.hpp>

#include <vector>

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
	typedef boost::shared_ptr<FL_OBJECT> bm_ptr;
	///
	typedef boost::shared_ptr<FL_FORM> fl_ptr;

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
