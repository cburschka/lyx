// -*- C++ -*-
/**
 * \file ControlCommand.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 *
 * ControlCommand is a controller class for dialogs that create or modify
 * an inset derived from InsetCommand.
 */

#ifndef CONTROLCOMMAND_H
#define CONTROLCOMMAND_H

#include "frontends/Dialog.h"
#include "insets/InsetCommandParams.h"

namespace lyx {
namespace frontend {

class ControlCommand : public Controller {
public:
	/// We need to know with what sort of inset we're associated.
	ControlCommand(Dialog &, std::string const & insetType);
	///
	virtual ~ControlCommand() {}
	///
	InsetCommandParams & params() { return params_; }
	///
	InsetCommandParams const & params() const { return params_; }
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
	InsetCommandParams params_;
	//FIXME It should be possible to eliminate lfun_name_
	//now and recover that information from params().insetType().
	//But let's not do that quite yet.
	/// Flags what action is taken by Kernel::dispatch()
	std::string const lfun_name_;
};

} // namespace frontend
} // namespace lyx

#endif // CONTROLCOMMAND_H
