// -*- C++ -*-
/**
 * \file ControlERT.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CONTROLERT_H
#define CONTROLERT_H


#include "Dialog.h"
#include "insets/InsetERT.h" // InsetERT::ERTStatus

namespace lyx {
namespace frontend {

class ControlERT : public Controller {
public:
	///
	ControlERT(Dialog &);
	///
	InsetCollapsable::CollapseStatus status() const { return status_; }
	///
	void setStatus(InsetCollapsable::CollapseStatus status) { status_ = status; }
	///
	virtual bool initialiseParams(std::string const & data);
	/// clean-up on hide.
	virtual void clearParams();
	/// clean-up on hide.
	virtual void dispatchParams();
	///
	virtual bool isBufferDependent() const { return true; }
private:
	///
	InsetCollapsable::CollapseStatus status_;
};

} // namespace frontend
} // namespace lyx

#endif
