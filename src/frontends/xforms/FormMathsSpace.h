// -*- C++ -*-
/**
 * \file FormMathsSpace.h
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author John Levon, moz@compsoc.man.ac.uk
 * \author Angus Leeming, a.leeming@ic.ac.uk
 */

#ifndef FORM_MATHSSPACE_H
#define FORM_MATHSSPACE_H

#include <boost/smart_ptr.hpp>

#ifdef __GNUG__
#pragma interface
#endif

#include "FormMathsPanel.h"

struct FD_form_maths_space;

/**
 * This class provides an XForms implementation of the maths space.
 */
class FormMathsSpace : public FormMathsSub {
public:
	///
	FormMathsSpace(LyXView *, Dialogs * , FormMathsPanel const &);

private:
	/// Build the dialog
	virtual void build();
	/// input handler
	virtual bool input(FL_OBJECT *, long);
	/// Apply from dialog (modify or create inset)
	virtual void apply();

	/// Pointer to the actual instantiation of the xforms form
	virtual FL_FORM * form() const;

	// build the form
	FD_form_maths_space * build_maths_space();

	// Real GUI implementation
	boost::scoped_ptr<FD_form_maths_space> dialog_;

	/// The current choice.
	int space_;
};

#endif //  FORM_MATHSSPACE_H
