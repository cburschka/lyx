/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2001 The LyX Team.
 *
 * ======================================================
 *
 * \file ControlConnections.h
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 *
 * ControlConnections.h contains the definition of three controller classes,
 * ControlConnectBase, ControlConnectBI and ControlConnectBD.
 *
 * Together they control the connection/disconnection of signals with the LyX
 * kernel. Controllers of individual dialogs interacting with the kernel through
 * signals/slots will all be derived from ControlConnectBI or ControlConnectBD.
 *
 * A dialog is classed as "Buffer Dependent" if its contents change with the
 * buffer (document). An example would be the Citation dialog. Such a dialog
 * would be derived, therefore, from ControlConnectBD.
 *
 * Conversely, a dialog is "Buffer Independent" if its contents do not change
 * when the buffer changes. An example would be the Copyright dialog. Such a
 * dialog is therefore derived from ControlConnectBI.
 *
 */

#ifndef CONTROLCONNECTIONS_H
#define CONTROLCONNECTIONS_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlButtons.h"

class Dialogs;
class LyXView;

/** Base class to control connection/disconnection of signals with the LyX
    kernel. It is meant to be used solely as the parent class to
    ControlConnectBI and ControlConnectBD.
*/
class ControlConnectBase : public ControlButtons
{
public:
	///
	enum DocTypes {
		///
		LATEX,
		///
		LITERATE,
		///
		LINUXDOC,
		///
		DOCBOOK
	};
	///
	ControlConnectBase(LyXView &, Dialogs &);
	/// The View may need to know if the buffer is read-only.
	bool isReadonly() const;
	/// 
	DocTypes docType() const;

protected:
	/// True if the dialog depends on the buffer, else false.
	virtual bool isBufferDependent() const = 0;

	/// Connect signals
	virtual void connect();
	/// Disconnect signals
	virtual void disconnect();

	/** Redraw the dialog (on receipt of a Signal indicating, for example,
	    its colors have been re-mapped).
	*/
	void redraw();

	/// Get at the kernel Dispatch methods we need to apply() parameters.
	LyXView & lv_;
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
	///
	virtual bool isBufferDependent() const { return false; }
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
	///
	virtual bool isBufferDependent() const { return true; }
	/// Connect signals
	virtual void connect();
	/// Disconnect signals
	virtual void disconnect();

private:
	/** Slot connected to update signal.
	    Bool indicates if a buffer switch took place.
	    Default behaviour is to ignore this and simply update().
	*/
	virtual void updateSlot(bool) { update(); }
	/// Update connection.
	SigC::Connection u_;
};

#endif // CONTROLCONNECTIONS_H
