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
 * \file ControlRef.h
 * \author Angus Leeming <leeming@lyx.org>
 */

#ifndef CONTROLREF_H
#define CONTROLREF_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlCommand.h"

/** A controller for the Ref Dialog.
 */
class ControlRef : public ControlCommand {
public:
	///
	ControlRef(LyXView &, Dialogs &);

	///
	std::vector<string> const getLabelList(string const &) const;
	///
	void gotoRef(string const &);
	///
	void gotoBookmark();
	///
	std::vector<string> const getBufferList() const;
	///
	int getBufferNum() const;
	///
	string const getBufferName(int num) const;
private:
	/** disconnect from the inset when the Apply button is pressed.
	 Allows easy insertion of multiple references. */
	virtual bool disconnectOnApply() { return true; }
};

#endif // CONTROLREF_H
