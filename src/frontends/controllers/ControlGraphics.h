/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2001 The LyX Team.
 *
 * ======================================================
 *
 * \file ControlGraphics.h
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#ifndef CONTROLGRAPHICS_H
#define CONTROLGRAPHICS_H

#ifdef __GNUG__
#pragma interface
#endif 

#include "ControlInset.h"

class InsetGraphics;
class InsetGraphicsParams;
class LyXView;

/** A controller for Graphics dialogs.
 */
class ControlGraphics
	: public ControlInset<InsetGraphics, InsetGraphicsParams> {
public:
	///
	ControlGraphics(LyXView &, Dialogs &);

	/// Browse for a file
	string const Browse(string const &);

private:
	/// Dispatch the changed parameters to the kernel.
	virtual void applyParamsToInset();
	/// 
	virtual void applyParamsNoInset();
	/// get the parameters from the string passed to createInset.
	virtual InsetGraphicsParams const getParams(string const &);
	/// get the parameters from the inset passed to showInset.
	virtual InsetGraphicsParams const getParams(InsetGraphics const &);
};

#endif // CONTROLGRAPHICS_H
