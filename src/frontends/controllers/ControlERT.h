// -*- C++ -*-
/**
 * \file ControlERT.h
 * Read the file COPYING
 *
 * \author Jürgen Vigna
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef CONTROLERT_H
#define CONTROLERT_H

#include <vector>

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlInset.h"
#include "insets/insetert.h" // InsetERT::Status

/** This should be moved back into insetert.h and InsetERT should
    contain an instance of it. */

struct ERTParams {
	///
	ERTParams();
	///
	ERTParams(InsetERT const &);
	///
	InsetERT::ERTStatus status;
};


///
bool operator==(ERTParams const &, ERTParams const &);
///
bool operator!=(ERTParams const &, ERTParams const &);


/** A controller for ERT dialogs.
 */
class ControlERT : public ControlInset<InsetERT, ERTParams>  {
public:
	///
	ControlERT(LyXView &, Dialogs &);

private:
	/// Dispatch the changed parameters to the kernel.
	virtual void applyParamsToInset();
	///
	virtual void applyParamsNoInset();
	/// get the parameters from the string passed to createInset.
	virtual ERTParams const getParams(string const &)
		{ return ERTParams(); }
	/// get the parameters from the inset passed to showInset.
	virtual ERTParams const getParams(InsetERT const &);
};

#endif
