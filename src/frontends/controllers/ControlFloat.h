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


#include "ControlInset.h"
#include "insets/insetfloat.h"

inline
bool operator==(InsetFloatParams const & p1, InsetFloatParams const & p2)
{
	return p1.placement == p2.placement && p1.wide == p2.wide;
}


inline
bool operator!=(InsetFloatParams const & p1, InsetFloatParams const & p2)
{
	return !(p1 == p2);
}


/** A controller for Minipage dialogs.
 */
class ControlFloat : public ControlInset<InsetFloat, InsetFloatParams>  {
public:
	///
	ControlFloat(LyXView &, Dialogs &);
private:
	/// Dispatch the changed parameters to the kernel.
	virtual void applyParamsToInset();
	///
	virtual void applyParamsNoInset();
	/// get the parameters from the string passed to createInset.
	virtual InsetFloatParams const getParams(string const &)
		{ return InsetFloatParams(); }
	/// get the parameters from the inset passed to showInset.
	virtual InsetFloatParams const getParams(InsetFloat const &);
};

#endif
