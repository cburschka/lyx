// -*- C++ -*-
/**
 * \file FormMathsMatrix.h
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author John Levon, moz@compsoc.man.ac.uk
 * \author Angus Leeming, a.leeming@ic.ac.uk
 */

#ifndef FORM_MATHSMATRIX_H
#define FORM_MATHSMATRIX_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormMathsPanel.h"

#include <boost/scoped_ptr.hpp>

struct FD_maths_matrix;

/**
 * This class provides an XForms implementation of the maths matrix.
 */
class FormMathsMatrix : public FormMathsSub {
public:
	FormMathsMatrix(LyXView *, Dialogs *, FormMathsPanel const &);
	///
	int AlignFilter(char const *, int);

private:
	/// Build the dialog
	virtual void build();
	/// input handler
	virtual bool input(FL_OBJECT *, long);
	/// Apply from dialog (modify or create inset)
	virtual void apply();

	/// Pointer to the actual instantiation of the xforms form
	virtual FL_FORM * form() const;

	// Real GUI implementation
	boost::scoped_ptr<FD_maths_matrix> dialog_;
};

#endif //  FORM_MATHSMATRIX_H
