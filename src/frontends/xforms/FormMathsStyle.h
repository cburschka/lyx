// -*- C++ -*-
/**
 * \file FormMathsStyle.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef FORM_MATHSSTYLE_H
#define FORM_MATHSSTYLE_H


#include "FormBase.h"

class ControlMathSub;
struct FD_maths_style;

/**
 * This class provides an XForms implementation of the maths style.
 */
class FormMathsStyle : public FormCB<ControlMathSub, FormDB<FD_maths_style> > {
public:
	///
	FormMathsStyle();

private:
	///
	virtual void apply();
	///
	virtual void build();
	///
	virtual ButtonPolicy::SMInput input(FL_OBJECT *, long);
	/// Not needed.
	virtual void update() {}

	/// The current choice.
	int style_;
};

#endif //  FORM_MATHSSTYLE_H
