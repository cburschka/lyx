// -*- C++ -*-
/**
 * \file ControlGraphics.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Baruch Even
 * \author Angus Leeming
 * \author Herbert Voﬂ
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CONTROLGRAPHICS_H
#define CONTROLGRAPHICS_H

#include "Dialog.h"
#include "support/docstring.h"
#include "insets/InsetGraphics.h"

#include <utility>
#include <vector>

namespace lyx {

class InsetGraphics;
class InsetGraphicsParams;

namespace frontend {

class LyXView;

/** A controller for Graphics dialogs.
 */
class ControlGraphics : public Controller
{
public:
	///
	ControlGraphics(Dialog &);
	///
	virtual bool initialiseParams(std::string const & data);
	/// clean-up on hide.
	virtual void clearParams();
	/// clean-up on hide.
	virtual void dispatchParams();
	///
	virtual bool isBufferDependent() const { return true; }
	///
	InsetGraphicsParams & params() { return params_; }
	///
	InsetGraphicsParams const & params() const { return params_; }

	/// Browse for a file
	docstring const browse(docstring const &) const;
	/// Read the Bounding Box from a eps or ps-file
	std::string const readBB(std::string const & file);
	/// Control the bb
	bool bbChanged;
	/// test if file exist
	bool isFilenameValid(std::string const & fname) const;
	/// edit file
	void editGraphics();

private:
	///
	InsetGraphicsParams params_;
};


/// get the units for the bounding box
std::vector<std::string> const getBBUnits();

/// The (tranlated) GUI std::string and it's LaTeX equivalent.
typedef std::pair<docstring, std::string> RotationOriginPair;
///
std::vector<RotationOriginPair> getRotationOriginData();

} // namespace frontend
} // namespace lyx

#endif // CONTROLGRAPHICS_H
