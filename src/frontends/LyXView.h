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

#include "support/strfwd.h"

namespace lyx {

namespace support { class FileName; }

class Buffer;
class BufferView;
class Cursor;
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
	///
	virtual bool closeBuffer() = 0;

	/// load a document into the current workarea.
	virtual Buffer * loadDocument(
		support::FileName const &  name, ///< File to load.
		bool tolastfiles = true  ///< append to the "Open recent" menu?
		) = 0;
	///
	virtual void newDocument(std::string const & filename,
		bool fromTemplate) = 0;

	//@}

	/// display a message in the view
	virtual void message(docstring const &) = 0;

	///
	virtual bool getStatus(FuncRequest const & cmd, FuncStatus & flag) = 0;
	/// dispatch command.
	/// \return true if the \c FuncRequest has been dispatched.
	virtual bool dispatch(FuncRequest const & cmd) = 0;

	///
	virtual void restartCursor() = 0;

	/// Update the completion popup and the inline completion state.
	/// If \c start is true, then a new completion might be started.
	/// If \c keep is true, an active completion will be kept active
	/// even though the cursor moved. The update flags of \c cur might
	/// be changed.
	virtual void updateCompletion(Cursor & cur, bool start, bool keep) = 0;

private:
	/// noncopyable
	LyXView(LyXView const &);
	void operator=(LyXView const &);
};

} // namespace frontend
} // namespace lyx

#endif // LYXVIEW_H
