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
#include "insets/InsetBranch.h"

namespace lyx {

class BranchList;

namespace frontend {

class ControlBranch : public Controller
{
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
	InsetBranchParams & params() { return params_; }
	///
	InsetBranchParams const & params() const { return params_; }
	///
	BranchList const & branchlist() const;
private:
	///
	InsetBranchParams params_;
};

} // namespace frontend
} // namespace lyx

#endif // CONTROLBRANCH_H
