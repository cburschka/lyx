// -*- C++ -*-
/**
 * \file ControlMinipage.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef CONTROLMINIPAGE_H
#define CONTROLMINIPAGE_H

#include <vector>

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlInset.h"
#include "insets/insetminipage.h" // InsetMinipage::Position

/** This should be moved back into insetminipage.h and InsetMinipage should
    contain an instance of it. */

struct MinipageParams {
	///
	MinipageParams();
	///
	MinipageParams(InsetMinipage const &);
	///
	LyXLength pageWidth;
	///
	InsetMinipage::Position pos;
};


///
bool operator==(MinipageParams const &, MinipageParams const &);
///
bool operator!=(MinipageParams const &, MinipageParams const &);


/** A controller for Minipage dialogs.
 */
class ControlMinipage : public ControlInset<InsetMinipage, MinipageParams>  {
public:
	///
	ControlMinipage(LyXView &, Dialogs &);

private:
	/// Dispatch the changed parameters to the kernel.
	virtual void applyParamsToInset();
	///
	virtual void applyParamsNoInset();
	/// get the parameters from the string passed to createInset.
	virtual MinipageParams const getParams(string const &)
		{ return MinipageParams(); }
	/// get the parameters from the inset passed to showInset.
	virtual MinipageParams const getParams(InsetMinipage const &);
};

#endif
