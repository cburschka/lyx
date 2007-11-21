// -*- C++ -*-
/**
 * \file Delegates.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef DELEGATES_H
#define DELEGATES_H

#include "support/strfwd.h"

namespace lyx {

class Buffer;
class Inset;

namespace frontend {

class GuiBufferViewDelegate 
{
public:
	virtual ~GuiBufferViewDelegate() {}

	/// This function is called when some message shows up.
	virtual void message(docstring const & msg) = 0;

	/// This function is called when some inset dialogs needs to be shown.
	virtual void showDialog(std::string const & name,
		std::string const & data, Inset * inset = 0) = 0;

	/// This function is called when some dialogs needs to be updated.
	/** \param name == "citation", "bibtex" etc; an identifier used
	    to update the contents of a particular dialog with \param data.
	    See the comments to 'show', above.
	*/
	virtual void updateDialog(std::string const & name,
		std::string const & data) = 0;
};


class GuiBufferDelegate
{
public:
	virtual ~GuiBufferDelegate() {}
	/// This function is called when the buffer structure is changed.
	virtual void structureChanged() = 0;
	/// This function is called when some parsing error shows up.
	virtual void errors(std::string const &) = 0;
	/// This function is called when some message shows up.
	virtual void message(docstring const &) = 0;
	/// This function is called when the buffer busy status change.
	virtual void setBusy(bool) = 0;
	/// Reset autosave timers for all users.
	virtual void resetAutosaveTimers() = 0;
};

} // namespace frontend
} // namespace lyx

#endif
