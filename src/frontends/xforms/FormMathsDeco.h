// -*- C++ -*-
/**
 * \file FormMathsDeco.h
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author John Levon, moz@compsoc.man.ac.uk
 * \author Angus Leeming, a.leeming@ic.ac.uk
 */

#ifndef FORM_MATHSDECO_H
#define FORM_MATHSDECO_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormMathsPanel.h"

#include <boost/scoped_ptr.hpp>

struct FD_form_maths_deco;

/**
 * This class provides an XForms implementation of the maths deco.
 */
class FormMathsDeco : public FormMathsSub {
public:
	///
	FormMathsDeco(LyXView *, Dialogs *, FormMathsPanel const &);

private:
	/// Build the dialog
	virtual void build();
	/// Apply from dialog (modify or create inset)
	virtual void apply();
	/// Input selection:
	virtual bool input(FL_OBJECT *, long);

	/// Pointer to the actual instantiation of the xforms form
	virtual FL_FORM * form() const;

	// build the form
	FD_form_maths_deco * build_maths_deco();

	// Real GUI implementation
	boost::scoped_ptr<FD_form_maths_deco> dialog_;

	/// Current choice
	int deco_;

};

#endif //  FORM_MATHSDECO_H
