// -*- C++ -*-
/**
 * \file ControlFloat.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CONTROLFLOAT_H
#define CONTROLFLOAT_H

#include "Dialog.h"
#include "insets/InsetFloat.h"

namespace lyx {
namespace frontend {

class ControlFloat : public Controller
{
public:
	///
	ControlFloat(Dialog &);
	///
	virtual bool initialiseParams(std::string const & data);
	/// clean-up on hide.
	virtual void clearParams();
	/// clean-up on hide.
	virtual void dispatchParams();
	///
	virtual bool isBufferDependent() const { return true; }
	///
	InsetFloatParams & params() { return params_; }
	///
	InsetFloatParams const & params() const { return params_; }
private:
	///
	InsetFloatParams params_;
};

} // namespace frontend
} // namespace lyx

#endif
