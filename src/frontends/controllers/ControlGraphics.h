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
#include <utility>
#include <vector>

class InsetGraphics;
class InsetGraphicsParams;
class LyXView;

/** A controller for Graphics dialogs.
 */

class ControlGraphics : public Dialog::Controller {
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
	InsetGraphicsParams & params() { return *params_.get(); }
	///
	InsetGraphicsParams const & params() const { return *params_.get(); }

	/// Browse for a file
	std::string const browse(std::string const &) const;
	/// Read the Bounding Box from a eps or ps-file
	std::string const readBB(std::string const & file);
	/// Control the bb
	bool bbChanged;
	/// test if file exist
	bool isFilenameValid(std::string const & fname) const;

private:
	///
	boost::scoped_ptr<InsetGraphicsParams> params_;
};

namespace frnt {

/// get the units for the bounding box
std::vector<std::string> const getBBUnits();

/// The (tranlated) GUI std::string and it's LaTeX equivalent.
typedef std::pair<std::string, std::string> RotationOriginPair;
///
std::vector<RotationOriginPair> getRotationOriginData();

} // namespace frnt

#endif // CONTROLGRAPHICS_H
