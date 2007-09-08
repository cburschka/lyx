// -*- C++ -*-
/**
 * \file ControlListings.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Bo Peng
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CONTROLLISTINGS_H
#define CONTROLLISTINGS_H

#include "Dialog.h"
#include "insets/InsetListingsParams.h"

namespace lyx {

class InsetListingsParams;

namespace frontend {

class ControlListings : public Controller  {
public:
	///
	ControlListings(Dialog &);
	///
	virtual bool initialiseParams(std::string const & data);
	/// clean-up on hide.
	virtual void clearParams();
	/// clean-up on hide.
	virtual void dispatchParams();
	///
	virtual bool isBufferDependent() const { return true; }
	///
	InsetListingsParams & params() { return params_; }
	///
	InsetListingsParams const & params() const { return params_; }
	///
	void setParams(InsetListingsParams const &);
private:
	///
	InsetListingsParams params_;
};

} // namespace frontend
} // namespace lyx

#endif
