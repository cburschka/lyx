// -*- C++ -*-
/**
 * \file FormMathsStyle.h
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author John Levon, moz@compsoc.man.ac.uk
 * \author Angus Leeming <leeming@lyx.org>
 */

#ifndef FORM_MATHSSTYLE_H
#define FORM_MATHSSTYLE_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormMathsPanel.h"

#include <boost/scoped_ptr.hpp>

struct FD_maths_style;

/**
 * This class provides an XForms implementation of the maths style.
 */
class FormMathsStyle : public FormMathsSub {
public:
	///
	FormMathsStyle(LyXView &, Dialogs &, FormMathsPanel const &);

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
	boost::scoped_ptr<FD_maths_style> dialog_;

	/// The current choice.
	int style_;
};

#endif //  FORM_MATHSSTYLE_H
