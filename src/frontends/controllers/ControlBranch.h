// -*- C++ -*-
/**
 * \file ControlBranch.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Martin Vermeer
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CONTROLBRANCH_H
#define CONTROLBRANCH_H


#include "Dialog.h"

namespace lyx {

class BranchList;
class InsetBranchParams;

namespace frontend {

class ControlBranch : public Controller {
public:
	///
	ControlBranch(Dialog &);
	///
	virtual bool initialiseParams(std::string const & data);
	///
	virtual void clearParams();
	///
	virtual void dispatchParams();
	///
	virtual bool isBufferDependent() const { return true; }
	///
	InsetBranchParams & params() { return *params_.get(); }
	InsetBranchParams const & params() const { return *params_.get(); }
	///
	BranchList const & branchlist() const;

private:
	///
	boost::scoped_ptr<InsetBranchParams> params_;
};

} // namespace frontend
} // namespace lyx

#endif // CONTROLBRANCH_H
