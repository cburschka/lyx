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
 * Full author contact details are available in file CREDITS.
 */

#ifndef FORM_MATHSPANEL_H
#define FORM_MATHSPANEL_H


#include "FormDialogView.h"


class ControlMath;
struct FD_maths_panel;

/**
 * This class provides an XForms implementation of the maths panel.
 */
class FormMathsPanel
	: public FormController<ControlMath, FormView<FD_maths_panel> > {
public:
	///
	FormMathsPanel(Dialog &);

private:
	/// Not needed.
	virtual void apply() {}
	///
	virtual void update() {}

	///
	virtual void build();
	///
	virtual ButtonPolicy::SMInput input(FL_OBJECT *, long);
};

#endif //  FORM_MATHSPANEL_H
