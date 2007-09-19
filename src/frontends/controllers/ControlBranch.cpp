/**
 * \file ControlBranch.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Martin Vermeer
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlBranch.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "BranchList.h"
#include "FuncRequest.h"

using std::string;
using std::vector;

namespace lyx {
namespace frontend {

ControlBranch::ControlBranch(Dialog & parent)
	: Controller(parent)
{}


bool ControlBranch::initialiseParams(string const & data)
{
	InsetBranchMailer::string2params(data, params_);
	return true;
}


void ControlBranch::clearParams()
{
	params_ = InsetBranchParams();
}

void ControlBranch::dispatchParams()
{
	string const lfun = InsetBranchMailer::params2string(params());
	dispatch(FuncRequest(getLfun(), lfun));
}


BranchList const & ControlBranch::branchlist() const
{
	return buffer().params().branchlist();
}

} // namespace frontend
} // namespace lyx
