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

#include "FormBase.h"

class ControlMathSub;
struct FD_maths_deco;

/**
 * This class provides an XForms implementation of the maths deco.
 */
class FormMathsDeco : public FormCB<ControlMathSub, FormDB<FD_maths_deco> > {
public:
	///
	FormMathsDeco();

private:
	///
	virtual void apply();
	///
	virtual void build();
	///
	virtual ButtonPolicy::SMInput input(FL_OBJECT *, long);
	/// Not needed.
	virtual void update() {}

	/// Current choice
	int deco_;
};

#endif //  FORM_MATHSDECO_H
