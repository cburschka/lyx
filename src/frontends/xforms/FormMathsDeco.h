// -*- C++ -*-
/**
 * \file FormMathsDeco.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author John Levon
 * \author Angus Leeming 
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef FORM_MATHSDECO_H
#define FORM_MATHSDECO_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormMathsPanel.h"

#include <boost/scoped_ptr.hpp>

struct FD_maths_deco;

/**
 * This class provides an XForms implementation of the maths deco.
 */
class FormMathsDeco : public FormMathsSub {
public:
	///
	FormMathsDeco(LyXView &, Dialogs &, FormMathsPanel const &);

private:
	/// Build the dialog
	virtual void build();
	/// Apply from dialog (modify or create inset)
	virtual void apply();
	/// Input selection:
	virtual bool input(FL_OBJECT *, long);

	/// Pointer to the actual instantiation of the xforms form
	virtual FL_FORM * form() const;

	// Real GUI implementation
	boost::scoped_ptr<FD_maths_deco> dialog_;

	/// Current choice
	int deco_;

};

#endif //  FORM_MATHSDECO_H
