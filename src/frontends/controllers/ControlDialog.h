// -*- C++ -*-
/*
 * \file ControlDialog.h
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 *
 * ControlDialog is to be used as a parent class for dialogs that are not
 * views onto parameters of insets. (An ugly description I know, but I hope
 * the meaning is clear! Can anyone do any better?) Examples would be the
 * Document and Paragraph dialogs.
 */

#ifndef CONTROLDIALOG_H
#define CONTROLDIALOG_H

class LyXView;
class Dialogs;

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

	/** Show the dialog.
	 *  Publicly accessible so that it can be invoked by the Dialogs class.
	 */
	virtual void show();

protected:
	/// Hide the dialog.
	virtual void hide();
	/// Update the dialog.
	virtual void update();

	/// clean-up on hide.
	virtual void clearParams() {}
	/// set the params before show or update
	virtual void setParams() {}

private:
	/// is the dialog built ?
	bool dialog_built_;
};

#endif // CONTROLDIALOG_H
