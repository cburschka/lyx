// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2001 The LyX Team.
 *
 * ======================================================
 *
 * \file ControlCommand.h
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 *
 * Defines a Controller class for dialogs that create or modify
 * an inset derived from InsetCommand.
 * Also defines the abstract base class from which the GUI-dependent Views
 * of this dialog should be derived.
 */

#ifndef CONTROLCOMMAND_H
#define CONTROLCOMMAND_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlConnections.h"
#include "insets/insetcommand.h"
#include "commandtags.h" // kb_action

/** The Inset dialog controller. Connects/disconnects signals, launches 
    GUI-dependent View and returns the output from this View to the kernel.
 */
class ControlCommand : public ControlConnectInset<InsetCommand>
{
public:
	///
	ControlCommand(LyXView &, Dialogs &, kb_action=LFUN_NOACTION);

	/// Allow the View access to the local copy.
	InsetCommandParams & params() const;

protected:
	/// Slots connected in the daughter classes c-tor.
	/// Slot launching dialog to (possibly) create a new inset.
	void createInset(string const &);
	/// Slot launching dialog to an existing inset.
	void showInset(InsetCommand *);

	/// Connect signals and launch View.
	void show(InsetCommandParams const &);

	/// Instantiation of private ControlBase virtual methods.
	/// Get changed parameters and Dispatch them to the kernel.
	virtual void apply();
	/// Disconnect signals and hide View.
	virtual void hide();
	/// Update dialog before showing it.
	virtual void update();

private:
	/** A local copy of the inset's params.
	    Memory is allocated only whilst the dialog is visible.
	*/
	InsetCommandParams * params_;
	/// Controls what is done in LyXFunc::Dispatch()
	kb_action const action_;
};


#endif // CONTROLCOMMAND_H
