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
 * \file ControlInsets.h
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 *
 * ControlInset is to be used as a parent class for dialogs that display and
 * can perhaps modify the contents of an individual inset. An example being the
 * ubiquitous Citation dialog.
 */

#ifndef CONTROLINSET_H
#define CONTROLINSET_H

#include "ControlConnections.h"
#include "LString.h"

#include <boost/signals/connection.hpp>

class Inset;

template <class Inset, class Params>
class ControlInset : public ControlConnectBD
{
public:
	///
	ControlInset(LyXView &, Dialogs &);
	/// Allow the View access to the local copy.
	Params & params();
	///
	Params const & params() const;

protected:
	/// Slots connected in the daughter classes c-tor.
	/// Slot launching dialog to (possibly) create a new inset.
	void createInset(string const &);
	/// Slot launching dialog to an existing inset.
	void showInset(Inset *);
	/// Allow the daughter methods to access the inset.
	Inset * inset() const;

private:
	/** These 7 methods are all that the individual daughter classes
	    should need to instantiate. */

	/// if the inset exists then do this...
	virtual void applyParamsToInset() = 0;
	/// else this...
	virtual void applyParamsNoInset() = 0;

	/// get the parameters from the string passed to createInset.
	virtual Params const getParams(string const &) = 0;
	/// get the parameters from the inset passed to showInset.
	virtual Params const getParams(Inset const &) = 0;

	/** Most derived classes won't need these two, so they default to empty.
	 */

	/// set any daughter class-particular data on show().
	virtual void setDaughterParams() {}
	/// clean-up any daughter class-particular data on hide().
	virtual void clearDaughterParams() {}

	/** Some dialogs may find it beneficial to disconnect from the inset
	 when the Apply button is pressed. E.g., doing this with the citation
	 dialog allows multiple citiations to be inserted easily. */
	virtual bool disconnectOnApply() { return false; }



	/// Instantiation of ControlButtons virtual methods.

	/// Get changed parameters and Dispatch them to the kernel.
	virtual void apply();
	/// Disconnect signals and hide View.
	virtual void hide();
	/// Update the dialog.
	virtual void update();

	/** Instantiation of ControlConnectBD private virtual method.
	    Slot connected to update signal. */
	virtual void updateSlot(bool);

	/// Show the dialog.
	void show(Params const &);
	/// Connect signals
	void connectInset(Inset * = 0);

	/// pointer to the inset passed through connectInset
	Inset * inset_;
	/// inset::hide connection.
	boost::signals::connection ih_;
	/** A local copy of the inset's params.
	    Memory is allocated only whilst the dialog is visible.
	*/
	Params * params_;

	/// is the dialog built ?
	bool dialog_built_;

};


#endif // CONTROLINSET_H
