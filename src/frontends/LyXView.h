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
class DispatchResult;
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
	virtual ~LyXView() {}

	/// \name Generic accessor functions
	//@{
	/// \return the currently selected buffer view.
	virtual BufferView * currentBufferView() = 0;
	virtual BufferView const * currentBufferView() const = 0;
	/// \return the current document buffer view.
	virtual BufferView * documentBufferView() = 0;
	virtual BufferView const * documentBufferView() const = 0;
	//@}

	/// display a message in the view
	virtual void message(docstring const &) = 0;

	///
	virtual bool getStatus(FuncRequest const & cmd, FuncStatus & flag) = 0;
	/// dispatch command.
	/// \return true if the \c FuncRequest has been dispatched.
	virtual void dispatch(FuncRequest const & cmd, DispatchResult & dr) = 0;

	///
	virtual void restartCursor() = 0;

	/// Update the completion popup and the inline completion state.
	/// If \c start is true, then a new completion might be started.
	/// If \c keep is true, an active completion will be kept active
	/// even though the cursor moved. The update flags of \c cur might
	/// be changed.
	virtual void updateCompletion(Cursor & cur, bool start, bool keep) = 0;
};

} // namespace frontend
} // namespace lyx

#endif // LYXVIEW_H
