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
 * Full author contact details are available in file CREDITS.
 */

#ifndef FORM_MATHSDELIM_H
#define FORM_MATHSDELIM_H


#include "FormDialogView.h"


class ControlMath;
struct FD_maths_delim;

/**
 * This class provides an XForms implementation of the maths delim.
 */
class FormMathsDelim
	: public FormController<ControlMath, FormView<FD_maths_delim> > {
public:
	///
	FormMathsDelim(Dialog &);

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

#endif //  FORM_MATHSDELIM_H
