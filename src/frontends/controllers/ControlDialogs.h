/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2001 The LyX Team.
 *
 * ======================================================
 *
 * \file ControlDialogs.h
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 *
 * ControlDialogs.h contains the definition of two template controller classes,
 * ControlDialog and ControlInset, rather clumsy names for classes that
 * control the showing, updating and hiding of popups.
 *
 * ControlInset is to be used as a parent class for popups that display and
 * can perhaps modify the contents of an individual inset. An example being the
 * ubiquitous Citation popup.
 *
 * ControlDialog is to be used as a parent class for popups that are not
 * Inset-popups. (An ugly description I know, but I hope the meaning is clear!
 * Can anyone do any better?) Examples would be the Document and Paragraph
 * popups.
 *
 * At the moment, ControlDialog is reaching a state of maturity as several
 * controllers are now derived from it and its required functionality
 * becaomes clear.
 *
 * ControlInset is still in a state of flux as currently only InsetCommand-type
 * insets have a controller.
 *
 */

#ifndef CONTROLCONNECTIONS2_H
#define CONTROLCONNECTIONS2_H

#include "ControlConnections.h"
#include "LyXView.h"

/** Base class to control connection/disconnection of signals with the LyX
    kernel for dialogs NOT used with insets.
    The Base class will be either ControlConnectBI or ControlConnectBD.
 */
template <class Base>
class ControlDialog : public Base
{
public:
	///
	ControlDialog(LyXView &, Dialogs &);

protected:
	/// Show the dialog.
	virtual void show();
	/// Hide the dialog.
	virtual void hide();
	/// Update the dialog.
	virtual void update();

	/// set the params before show or update
	virtual void setParams() {}
	/// clean-up on hide.
	virtual void clearParams() {}
};


/** Base class to control connection/disconnection of signals with the LyX
    kernel for Inset dialogs.
 */
class Inset;

template <class Inset>
class ControlInset : public ControlConnectBD
{
public:
	///
	ControlInset(LyXView &, Dialogs &);

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





template <class Base>
ControlDialog<Base>::ControlDialog(LyXView & lv, Dialogs & d)
	: Base(lv, d)
{}


template <class Base>
void ControlDialog<Base>::show()
{
	if (isBufferDependent() && !lv_.view()->available())
		return;

	setParams();

	bc().readOnly(isReadonly());
	view().show();
}

template <class Base>
void ControlDialog<Base>::update()
{
	if (isBufferDependent() && !lv_.view()->available())
		return;

	setParams();
	
	bc().readOnly(isReadonly());
	view().update();
}

template <class Base>
void ControlDialog<Base>::hide()
{
	clearParams();

	disconnect();
	view().hide();
}


template <class Inset>
ControlInset<Inset>::ControlInset(LyXView & lv, Dialogs & d)
	: ControlConnectBD(lv, d),
	  inset_(0), ih_(0)
{}


template <class Inset>
void ControlInset<Inset>::updateSlot(bool switched)
{
	if (switched)
		hide();
	else
		update();
}


template <class Inset>
void ControlInset<Inset>::disconnect()
{
	inset_ = 0;
	ih_.disconnect();
	ControlConnectBD::disconnect();
}


template <class Inset>
void ControlInset<Inset>::connectInset(Inset * inset)
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


template <class Inset>
void ControlInset<Inset>::disconnectInset()
{
	ih_.disconnect();
}


#endif // CONTROLCONNECTIONS2_H
