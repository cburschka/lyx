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

#include "support/LAssert.h"
#include "ControlConnections.h"

class Inset;

template <class Inset, class Params>
class ControlInset : public ControlConnectBD
{
public:
	///
	ControlInset(LyXView &, Dialogs &);
	/// Allow the View access to the local copy.
	Params & params() const;

protected:
	/// Slots connected in the daughter classes c-tor.
	/// Slot launching dialog to (possibly) create a new inset.
	void createInset(string const &);
	/// Slot launching dialog to an existing inset.
	void showInset(Inset *);
	/// Allow the daughter methods to access the inset.
	Inset * inset() const;

private:
	/** These 6 methods are all that the individual daughter classes
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
	SigC::Connection ih_;
	/** A local copy of the inset's params.
	    Memory is allocated only whilst the dialog is visible.
	*/
	Params * params_;
};


template <class Inset, class Params>
ControlInset<Inset, Params>::ControlInset(LyXView & lv, Dialogs & d)
	: ControlConnectBD(lv, d),
	  inset_(0), ih_(0), params_(0)
{}


template <class Inset, class Params>
void ControlInset<Inset, Params>::showInset(Inset * inset)
{
	if (inset == 0) return;  // maybe we should Assert this?

	connectInset(inset);
	show(getParams(*inset));
}


template <class Inset, class Params>
void ControlInset<Inset, Params>::createInset(string const & arg)
{
	connectInset();

	if ( !arg.empty() )
		bc().valid(); // so that the user can press Ok

	show(getParams(arg));
}


template <class Inset, class Params>
void ControlInset<Inset, Params>::show(Params const & params)
{
	if (params_) delete params_;
	params_ = new Params(params);

	setDaughterParams();

	bc().readOnly(isReadonly());
	view().show();
}


template <class Inset, class Params>
void ControlInset<Inset, Params>::hide()
{
	if (params_) {
		delete params_;
		params_ = 0;
	}
	inset_ = 0;

	clearDaughterParams();

	ih_.disconnect();
	disconnect();
	view().hide();
}


template <class Inset, class Params>
void ControlInset<Inset, Params>::update()
{
	if (params_) delete params_;

	if (inset_)
		params_ = new Params(getParams(*inset_));
	else
		params_ = new Params();

	bc().readOnly(isReadonly());
	view().update();
}


template <class Inset, class Params>
void ControlInset<Inset, Params>::apply()
{
	if (lv_.buffer()->isReadonly())
		return;

	view().apply();

	if (inset_ && params() != getParams(*inset_))
		applyParamsToInset();
	else
		applyParamsNoInset();
}


template <class Inset, class Params>
Params & ControlInset<Inset, Params>::params() const
{
	lyx::Assert(params_);
	return *params_;
}


template <class Inset, class Params>
Inset * ControlInset<Inset, Params>::inset() const
{
	lyx::Assert(inset_);
	return inset_;
}


template <class Inset, class Params>
void ControlInset<Inset, Params>::updateSlot(bool switched)
{
	if (switched)
		hide();
	else
		update();
}


template <class Inset, class Params>
void ControlInset<Inset, Params>::connectInset(Inset * inset)
{
	// If connected to another inset, disconnect from it.
	if (inset_) {
		ih_.disconnect();
		inset_ = 0;
	}

	if (inset) {
		inset_ = inset;
		ih_ = inset->hideDialog.connect(
			SigC::slot(this, &ControlInset::hide));
	}
	connect();
}
#endif // CONTROLINSET_H
