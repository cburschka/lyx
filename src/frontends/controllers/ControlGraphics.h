// -*- C++ -*-
/**
 * \file ControlGraphics.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Baruch Even
 * \author Angus Leeming
 * \author Herbert Voss
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef CONTROLGRAPHICS_H
#define CONTROLGRAPHICS_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlInset.h"

// needed to instatiate inset->hideDialog in ControlInset
#include "insets/insetgraphics.h"

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
