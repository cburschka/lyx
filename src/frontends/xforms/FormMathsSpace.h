// -*- C++ -*-
/**
 * \file FormMathsSpace.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef FORM_MATHSSPACE_H
#define FORM_MATHSSPACE_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBase.h"

class ControlMathSub;
struct FD_maths_space;

/**
 * This class provides an XForms implementation of the maths space.
 */
class FormMathsSpace : public FormCB<ControlMathSub, FormDB<FD_maths_space> > {
public:
	///
	FormMathsSpace();

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
	int space_;
};

#endif //  FORM_MATHSSPACE_H
