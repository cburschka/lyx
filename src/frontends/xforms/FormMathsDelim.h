// -*- C++ -*-
/**
 * \file FormMathsDelim.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author John Levon
 * \author Angus Leeming 
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef FORM_MATHSDELIM_H
#define FORM_MATHSDELIM_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormMathsPanel.h"

#include <boost/scoped_ptr.hpp>

struct FD_maths_delim;

/**
 * This class provides an XForms implementation of the maths delim.
 */
class FormMathsDelim : public FormMathsSub {
public:
	///
	FormMathsDelim(LyXView &, Dialogs &, FormMathsPanel const &);

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
	boost::scoped_ptr<FD_maths_delim> dialog_;
};

#endif //  FORM_MATHSDELIM_H
