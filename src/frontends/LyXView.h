// -*- C++ -*-
/**
 * \file LyXView.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYXVIEW_H
#define LYXVIEW_H

#include "frontends/Delegates.h"
#include "support/strfwd.h"

namespace lyx {

namespace support { class FileName; }

class Buffer;
class BufferView;
class FuncStatus;
class FuncRequest;
class Inset;

namespace frontend {

/**
 * LyXView - main LyX window
 *
 * This class represents the main LyX window and provides
 * accessor functions to its content.
 *
 * The eventual intention is that LyX will support a number
 * of containing LyXViews. Currently a lot of code still
 * relies on there being a single top-level view.
 *
 * Additionally we would like to support multiple views
 * in a single LyXView.
 */
class LyXView
{
public:
	///
	LyXView() {}
	///
	virtual ~LyXView() {}
	///
	virtual int id() const = 0;
	///
	virtual void close() = 0;

	/// show busy cursor
	virtual void setBusy(bool) = 0;

	//@{ generic accessor functions

	/// \return the current buffer view.
	virtual BufferView * view() = 0;

	/// \return the buffer currently shown in this window
	virtual Buffer * buffer() = 0;
	virtual Buffer const * buffer() const = 0;
	/// set a buffer to the current workarea.
	virtual void setBuffer(Buffer * b) = 0; ///< \c Buffer to set.

	//@}

	/// display a message in the view
	virtual void message(docstring const &) = 0;

	///
	virtual FuncStatus getStatus(FuncRequest const & cmd) = 0;
	/// dispatch to current BufferView
	virtual void dispatch(FuncRequest const & cmd) = 0;

	///
	virtual void restartCursor() = 0;
	
	//
	// GuiBufferDelegate
	//
	virtual void errors(std::string const &) = 0;

	//
	// This View's Dialogs
	//
	
	///
	virtual void disconnectDialog(std::string const & name) = 0;
	///
	virtual Inset * getOpenInset(std::string const & name) const = 0;

private:
	/// noncopyable
	LyXView(LyXView const &);
	void operator=(LyXView const &);
};

} // namespace frontend
} // namespace lyx

#endif // LYXVIEW_H
