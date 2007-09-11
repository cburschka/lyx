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
#include <boost/scoped_ptr.hpp>


namespace lyx {

class InsetWrapParams;

namespace frontend {

class ControlWrap : public Controller  {
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
	InsetWrapParams & params() { return *params_.get(); }
	///
	InsetWrapParams const & params() const { return *params_.get(); }
private:
	///
	boost::scoped_ptr<InsetWrapParams> params_;
};

} // namespace frontend
} // namespace lyx

#endif
