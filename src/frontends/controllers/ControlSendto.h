// -*- C++ -*-
/**
 * \file ControlSendto.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef CONTROLSENDTO_H
#define CONTROLSENDTO_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlDialog_impl.h"
#include "LString.h"

#include <vector>

class Format;

/** A controller for the Custom Export dialogs.
 */
class ControlSendto : public ControlDialogBD {
public:
	///
	ControlSendto(LyXView &, Dialogs &);

	/// Return a vector of those formats that can be exported from "lyx".
	std::vector<Format const *> const allFormats() const;

	/// The format to export to
	Format const * getFormat() { return format_; }
	void setFormat(Format const *);

	/// The command to be executed
	string const getCommand() const { return command_; };
	void setCommand(string const &);
private:
	///
	virtual void apply();
	///
	Format const * format_;
	///
	string command_;
};

#endif // CONTROLSENDTO_H
