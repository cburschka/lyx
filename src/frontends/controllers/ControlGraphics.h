// -*- C++ -*-
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
 * \author Herbert Voss <voss@perce.de>
*/

#ifndef CONTROLGRAPHICS_H
#define CONTROLGRAPHICS_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlInset.h"
#include <utility>
#include <vector>

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
	/// Read the Bounding Box from a eps or ps-file
	string const readBB(string const & file);
	/// Control the bb
	bool bbChanged;
	/// test if file exist
	bool isFilenameValid(string const & fname) const;

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

namespace frnt {
	/// The (tranlated) GUI string and it's LaTeX equivalent.
	typedef std::pair<string, string> RotationOriginPair;
	///
	std::vector<RotationOriginPair> getRotationOriginData();
} // namespace frnt

#endif // CONTROLGRAPHICS_H
