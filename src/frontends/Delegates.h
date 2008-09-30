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
class DocIterator;
class Inset;

namespace frontend {

class GuiBufferViewDelegate 
{
public:
	virtual ~GuiBufferViewDelegate() {}

	/// This function is called when some message shows up.
	virtual void message(docstring const & msg) = 0;

	/// This function is called when some inset dialogs needs to be shown.
	/** \param name == "bibtex", "citation" etc; an identifier used to
	    launch a particular dialog.
	    \param data is a string representation of the Inset contents.
	    It is often little more than the output from Inset::write.
	    It is passed to, and parsed by, the frontend dialog.
	    Several of these dialogs do not need any data.
	    \param inset ownership is _not_ passed to the frontend dialog.
	    It is stored internally and used by the kernel to ascertain
	    what to do with the FuncRequest dispatched from the frontend
	    dialog on 'Apply'; should it be used to create a new inset at
	    the current cursor position or modify an existing, 'open' inset?
	*/
	virtual void showDialog(std::string const & name,
		std::string const & data, Inset * inset = 0) = 0;

	/// This function is called when some dialogs needs to be reset.
	/** \param name == "citation", "bibtex" etc; an identifier used
	    to reset the contents of a particular dialog with \param data.
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
	/// This function is called when the buffer structure has been updated.
	virtual void updateTocItem(std::string const &, DocIterator const &) = 0;
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
