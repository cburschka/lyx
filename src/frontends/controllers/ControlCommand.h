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
 *
 * The class is likely to be changed as other Inset controllers are created
 * and it becomes clear just what functionality can be moved back into
 * ControlInset.
 *
 */

#ifndef CONTROLCOMMAND_H
#define CONTROLCOMMAND_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlInset.h"
#include "insets/insetcommand.h"
#include "commandtags.h" // kb_action

/** The Inset dialog controller. Connects/disconnects signals, launches
    GUI-dependent View and returns the output from this View to the kernel.
 */
class ControlCommand : public ControlInset<InsetCommand, InsetCommandParams>
{
public:
	///
	ControlCommand(LyXView &, Dialogs &, kb_action=LFUN_NOACTION);

private:
	/// Dispatch the changed parameters to the kernel.
	virtual void applyParamsToInset();
	///
	virtual void applyParamsNoInset();
	/// get the parameters from the string passed to createInset.
	virtual InsetCommandParams const getParams(string const &);
	/// get the parameters from the inset passed to showInset.
	virtual InsetCommandParams const getParams(InsetCommand const &);

	/// Controls what is done in LyXFunc::Dispatch()
	kb_action const action_;
};


#endif // CONTROLCOMMAND_H
