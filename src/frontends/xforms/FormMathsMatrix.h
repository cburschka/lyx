// -*- C++ -*-
/**
 * \file FormMathsMatrix.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef FORM_MATHSMATRIX_H
#define FORM_MATHSMATRIX_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBase.h"

class ControlMathSub;
struct FD_maths_matrix;

/**
 * This class provides an XForms implementation of the maths matrix.
 */
class FormMathsMatrix : public FormCB<ControlMathSub, FormDB<FD_maths_matrix> >
{
public:
	///
	FormMathsMatrix();
	///
	int AlignFilter(char const *, int);

private:
	///
	virtual void apply();
	///
	virtual void build();
	///
	virtual ButtonPolicy::SMInput input(FL_OBJECT *, long);
	///
	virtual void update();
};

#endif //  FORM_MATHSMATRIX_H
