// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2000 The LyX Team.
 *
 * ======================================================
 *
 * \file ControlConnections.h
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#ifndef CONTROLCONNECTIONS_H
#define CONTROLCONNECTIONS_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlBase.h"

/** Base class to control connection/disconnection of signals with the LyX
    kernel. It is meant to be used solely as the parent class to
    ControlConnectBI and ControlConnectBD.
*/
class ControlConnectBase : public ControlBase
{
public:
	///
	ControlConnectBase(LyXView &, Dialogs &);

protected:
	/// Connect signals
	virtual void connect();
	/// Disconnect signals
	virtual void disconnect();

	/** Redraw the dialog (on receipt of a Signal indicating, for example,
	    its colors have been re-mapped).
	*/
	void redraw();

	/// Contains the signals we have to connect to.
	Dialogs & d_;
	/// Hide connection.
	SigC::Connection h_;
	/// Redraw connection.
	SigC::Connection r_;
};


/** Base class to control connection/disconnection of signals with the LyX
    kernel for Buffer Independent dialogs.
    Such dialogs do not require an update Connection although they may use
    an update() function which is also supported by the Restore button.
 */

class ControlConnectBI : public ControlConnectBase
{
public:
        ///
        ControlConnectBI(LyXView &, Dialogs &);

protected:
	/// Connect signals
	virtual void connect();
};


/** Base class to control connection/disconnection of signals with the LyX
    kernel for Buffer Dependent dialogs.
 */
class ControlConnectBD : public ControlConnectBase
{
public:
	///
	ControlConnectBD(LyXView &, Dialogs &);

protected:
	/** Slot connected to update signal.
	    Bool indicates if a buffer switch took place.
	    Default behaviour is to ignore this and simply update().
	*/
	virtual void updateSlot(bool) { update(); }
	/// Connect signals
	virtual void connect();
	/// Disconnect signals
	virtual void disconnect();

private:
	/// Update connection.
	SigC::Connection u_;
};

/** Base class to control connection/disconnection of signals with the LyX
    kernel for Inset dialogs.
 */
class Inset;

template <class Inset>
class ControlConnectInset : public ControlConnectBD
{
public:
	///
	ControlConnectInset(LyXView &, Dialogs &);

protected:
	/// Slot connected to update signal.
	virtual void updateSlot(bool);
	/// Connect signals
	void connectInset(Inset * = 0);
	/// Disconnect signals
	virtual void disconnect();
	///
	void disconnectInset();

protected:
	/// pointer to the inset passed through connectInset
	Inset * inset_;

private:
	/// inset::hide connection.
	SigC::Connection ih_;
};


template <class Inset>
ControlConnectInset<Inset>::ControlConnectInset(LyXView & lv, Dialogs & d)
	: ControlConnectBD(lv, d),
	  inset_(0), ih_(0)
{}


template <class Inset>
void ControlConnectInset<Inset>::updateSlot(bool switched)
{
	if (switched)
		hide();
	else
		update();
}


template <class Inset>
void ControlConnectInset<Inset>::disconnect()
{
	inset_ = 0;
	ih_.disconnect();
	ControlConnectBD::disconnect();
}


template <class Inset>
void ControlConnectInset<Inset>::connectInset(Inset * inset)
{
	// If connected to another inset, disconnect from it.
	if (inset_) {
		ih_.disconnect();
		inset_ = 0;
	}

	if (inset) {
		inset_ = inset;
		ih_ = inset->hideDialog.connect(
			SigC::slot(this, &ControlConnectInset::hide));
	}
	connect();
}


template <class Inset>
void ControlConnectInset<Inset>::disconnectInset()
{
	ih_.disconnect();
}


#endif // CONTROLCONNECTIONS_H
