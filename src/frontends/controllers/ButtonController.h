/*
 * \file ButtonController.h
 *
 * This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2000-2001 The LyX Team.
 *
 * ======================================================
 *
 * \author Allan Rae, rae@lyx.org
 */

#ifndef BUTTONCONTROLLER_H
#define BUTTONCONTROLLER_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ButtonPolicies.h"
#include "LString.h"
#include "gettext.h"

/** Abstract base class for a ButtonController

 * Controls the activation of the OK, Apply and Cancel buttons.
 * Actually supports 4 buttons in all and it's up to the user to decide on
 * the activation policy and which buttons correspond to which output of the
 * state machine.
 * Author: Allan Rae <rae@lyx.org>.
 * This abstract base class stripped of xforms-specific code by
 * Angus Leeming <a.leeming@ic.ac.uk>
 */
class ButtonControllerBase : public boost::noncopyable
{
public:
	/** Constructor.
	    The cancel/close label entries are _not_ managed within the class
	    thereby allowing you to reassign at will and to use static labels.
	    It also means if you really don't want to have the Cancel button
	    label be different when there is nothing changed in the dialog then
	    you can just assign "Cancel" to both labels.  Or even reuse this
	    class for something completely different.
	 */
	ButtonControllerBase(string const & cancel, string const & close);
	///
	virtual ~ButtonControllerBase() {}
	///
	virtual void refresh() = 0;
	///
	virtual ButtonPolicy & bp() = 0;
	///
	virtual void input(ButtonPolicy::SMInput);
	///
	void ok();
	///
	void apply();
	///
	void cancel();
	///
	void undoAll();
	///
	void hide();
	/// Passthrough function -- returns its input value
	bool readOnly(bool = true);
	///
	void readWrite();
	///
	void valid(bool = true);
	///
	void invalid();

protected:
	///
	string cancel_label;
	///
	string close_label;	
};


template <class BP, class GUIBC>
class ButtonController: public GUIBC
{
public:
	///
	ButtonController(string const & = _("Cancel"),
			 string const & = _("Close"));
	///
	virtual ButtonPolicy & bp() { return bp_; }

protected:
	///
	BP bp_;
};


template <class BP, class GUIBC>
ButtonController<BP, GUIBC>::ButtonController(string const & cancel,
					      string const & close)
	: GUIBC(cancel, close)
{}

#endif // BUTTONCONTROLLER_H
