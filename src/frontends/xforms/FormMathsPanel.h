// -*- C++ -*-
/**
 * \file FormMathsPanel.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef FORM_MATHSPANEL_H
#define FORM_MATHSPANEL_H


#include "FormBase.h"

class ControlMath;
struct FD_maths_panel;
class FormMathsBitmap;

/**
 * This class provides an XForms implementation of the maths panel.
 */
class FormMathsPanel : public FormCB<ControlMath, FormDB<FD_maths_panel> > {
public:
	///
	FormMathsPanel();

private:
	/// Not needed.
	virtual void apply() {}
	///
	virtual void update() {}

	///
	virtual void build();
	///
	virtual ButtonPolicy::SMInput input(FL_OBJECT *, long);

	/** Add a bitmap dialog to the store of all daughters_ and
	 *  return a pointer to the dialog, so that bitmaps can be added to it.
	 */
	FormMathsBitmap * addDaughter(void * button, string const & title,
				      char const * const * data, int size);
};

#endif //  FORM_MATHSPANEL_H
