// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *          Copyright 2001 The LyX Team.
 *
 *======================================================
 *
 * \file ControlMinipage.h
 * \author Juergen Vigna, jug@sad.it
 * \author Angus Leeming, a.leeming@ic.ac.uk
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
	string width;
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

/// Helper functions
namespace minipage {

	std::vector<string> const getUnits();

} //  namespace minipage

#endif
