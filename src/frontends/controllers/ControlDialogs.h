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
 * \file ControlDialogs.h
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 *
 * ControlDialog is to be used as a parent class for dialogs that are not
 * views onto parameters of insets. (An ugly description I know, but I hope 
 * the meaning is clear!  * Can anyone do any better?) Examples would be the 
 * Document and Paragraph dialogs.
 */

#ifndef CONTROLDIALOGS_H
#define CONTROLDIALOGS_H

#include "ControlConnections.h"
#include "debug.h"

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

	/// clean-up on hide.
	virtual void clearParams() {}
	/// set the params before show or update
	virtual void setParams() {}

	/// is the dialog built ?
	bool dialog_built_;
};


#include "LyXView.h"

template <class Base>
ControlDialog<Base>::ControlDialog(LyXView & lv, Dialogs & d)
	: Base(lv, d), dialog_built_(false)
{}


template <class Base>
void ControlDialog<Base>::show()
{
	if (isBufferDependent() && !lv_.view()->available())
		return;

	setParams();

	if (!dialog_built_) {
		view().build();
		dialog_built_ = true;
	}

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

#endif // CONTROLDIALOGS_H
