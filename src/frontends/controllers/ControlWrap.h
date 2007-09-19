// -*- C++ -*-
/**
 * \file ControlWrap.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Dekel Tsur
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CONTROLWRAP_H
#define CONTROLWRAP_H

#include "Dialog.h"
#include "insets/InsetWrap.h"

namespace lyx {
namespace frontend {

class ControlWrap : public Controller
{
public:
	///
	ControlWrap(Dialog &);
	///
	virtual bool initialiseParams(std::string const & data);
	/// clean-up on hide.
	virtual void clearParams();
	/// clean-up on hide.
	virtual void dispatchParams();
	///
	virtual bool isBufferDependent() const { return true; }
	///
	InsetWrapParams & params() { return params_; }
	///
	InsetWrapParams const & params() const { return params_; }
private:
	///
	InsetWrapParams params_;
};

} // namespace frontend
} // namespace lyx

#endif
