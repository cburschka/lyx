// -*- C++ -*-
/**
 * \file ControlFloat.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef CONTROLFLOAT_H
#define CONTROLFLOAT_H

#include <vector>

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlInset.h"

// needed to instatiate inset->hideDialog in ControlInset
#include "insets/insetfloat.h"

class InsetFloat;

///
struct FloatParams {
	///
	FloatParams();
	///
	FloatParams(InsetFloat const &);
	///
	string placement;
	///
	bool wide;
};


inline
bool operator==(FloatParams const & p1, FloatParams const & p2)
{
	return p1.placement == p2.placement && p1.wide == p2.wide;
}


inline
bool operator!=(FloatParams const & p1, FloatParams const & p2)
{
	return !(p1 == p2);
}


/** A controller for Minipage dialogs.
 */
class ControlFloat : public ControlInset<InsetFloat, FloatParams>  {
public:
	///
	ControlFloat(LyXView &, Dialogs &);
private:
	/// Dispatch the changed parameters to the kernel.
	virtual void applyParamsToInset();
	///
	virtual void applyParamsNoInset();
	/// get the parameters from the string passed to createInset.
	virtual FloatParams const getParams(string const &)
		{ return FloatParams(); }
	/// get the parameters from the inset passed to showInset.
	virtual FloatParams const getParams(InsetFloat const &);
};

#endif
