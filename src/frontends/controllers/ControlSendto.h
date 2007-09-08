// -*- C++ -*-
/**
 * \file ControlSendto.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CONTROLSENDTO_H
#define CONTROLSENDTO_H

#include "Dialog.h"
#include <vector>


namespace lyx {

class Format;

namespace frontend {

/** A controller for the Custom Export dialogs.
 */
class ControlSendto : public Controller {
public:
	///
	ControlSendto(Dialog &);
	///
	virtual bool initialiseParams(std::string const & data);
	///
	virtual void clearParams() {}
	///
	virtual void dispatchParams();
	///
	virtual bool isBufferDependent() const { return true; }
	///
	virtual kb_action getLfun() const { return LFUN_BUFFER_EXPORT_CUSTOM; }

	/// Return a vector of those formats that can be exported from "lyx".
	std::vector<Format const *> const allFormats() const;

	/// The format to export to
	Format const * getFormat() { return format_; }
	void setFormat(Format const *);

	/// The command to be executed
	std::string const getCommand() const { return command_; };
	void setCommand(std::string const &);
private:
	///
	Format const * format_;
	///
	std::string command_;
};

} // namespace frontend
} // namespace lyx

#endif // CONTROLSENDTO_H
