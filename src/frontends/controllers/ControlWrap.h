// -*- C++ -*-
/**
 * \file ControlWrap.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Dekel Tsur
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef CONTROLWRAP_H
#define CONTROLWRAP_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlInset.h"
#include "lyxlength.h"

// needed to instatiate inset->hideDialog in ControlInset
#include "insets/insetwrap.h"

class InsetWrap;

///
struct WrapParams {
	///
	WrapParams() {}
	///
	WrapParams(InsetWrap const &);
	///
	LyXLength pageWidth;
	///
	string placement;
};


inline
bool operator==(WrapParams const & p1, WrapParams const & p2)
{
	return p1.pageWidth == p2.pageWidth && p1.placement == p2.placement;
}


inline
bool operator!=(WrapParams const & p1, WrapParams const & p2)
{
	return !(p1 == p2);
}


/** A controller for Minipage dialogs.
 */
class ControlWrap : public ControlInset<InsetWrap, WrapParams>  {
public:
	///
	ControlWrap(LyXView &, Dialogs &);
private:
	/// Dispatch the changed parameters to the kernel.
	virtual void applyParamsToInset();
	///
	virtual void applyParamsNoInset();
	/// get the parameters from the string passed to createInset.
	virtual WrapParams const getParams(string const &)
		{ return WrapParams(); }
	/// get the parameters from the inset passed to showInset.
	virtual WrapParams const getParams(InsetWrap const &);
};

#endif
