// -*- C++ -*-
/**
 * \file ControlCommand.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 *
 * ControlCommand is a controller class for dialogs that create or modify
 * an inset derived from InsetCommand.
 */

#ifndef CONTROLCOMMAND_H
#define CONTROLCOMMAND_H


#include "Dialog.h"
#include "insets/insetcommandparams.h"
#include "commandtags.h" // kb_action


class ControlCommand : public Dialog::Controller {
public:
	///
	ControlCommand(Dialog &, kb_action=LFUN_NOACTION);
	///
	InsetCommandParams & params() { return params_; }
	///
	InsetCommandParams const & params() const { return params_; }
	///
	virtual void initialiseParams(string const & data);
	/// clean-up on hide.
	virtual void clearParams();
	/// clean-up on hide.
	virtual void dispatchParams();
	///
	virtual bool isBufferDependent() const { return true; }

private:
	///
	InsetCommandParams params_;
	/// Flags what action is taken by Kernel::dispatch()
	kb_action const action_;
};


#endif // CONTROLCOMMAND_H
