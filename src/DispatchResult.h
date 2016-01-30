// -*- C++ -*-
/**
 * \file DispatchResult.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Peter Kümmel
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef DISPATCH_RESULT_H
#define DISPATCH_RESULT_H

#include "update_flags.h"

#include "support/docstring.h"

namespace lyx {


class DispatchResult
{
public:
	///
	DispatchResult() :
			dispatched_(false),
			error_(false),
			update_(Update::None),
			need_buf_update_(false),
			need_msg_update_(true),
			need_changes_update_(false)
	{}
	///
	DispatchResult(bool dispatched, Update::flags f) :
			dispatched_(dispatched),
			error_(false),
			update_(f),
			need_buf_update_(false),
			need_msg_update_(true),
			need_changes_update_(false)
	{}
	///
	bool dispatched() const { return dispatched_; }
	///
	void dispatched(bool disp) { dispatched_ = disp; }
	///
	bool error() const { return error_; }
	///
	void setError(bool e) { error_ = e; }
	///
	docstring message() { return message_; }
	///
	void setMessage(docstring const & m) { message_ = m; }
	///
	void setMessage(std::string const & m) { message_ = from_utf8(m); }
	///
	Update::flags screenUpdate() const { return update_; }
	///
	void screenUpdate(Update::flags f) { update_ = f; }

	/// Does the buffer need updating?
	bool needBufferUpdate() const { return need_buf_update_; }
	/// Force the buffer to be updated
	void forceBufferUpdate() { need_buf_update_ = true; }
	/// Clear the flag indicating we need an update
	void clearBufferUpdate() { need_buf_update_ = false; }

	/// Do we need to display a message in the status bar?
	bool needMessageUpdate() const { return need_msg_update_; }
	/// Force the message to be displayed
	void forceMessageUpdate() { need_msg_update_ = true; }
	/// Clear the flag indicating we need to display the message
	void clearMessageUpdate() { need_msg_update_ = false; }

	/// Do we need to update the change tracking presence flag?
	bool needChangesUpdate() { return need_changes_update_; }
	/// Force the change tracking presence flag to be updated
	void forceChangesUpdate() { need_changes_update_ = true; }
	/// Clear the flag indicating that we need to update the change tracking
	/// presence flag
	void clearChangesUpdate() { need_changes_update_ = false; }

private:
	/// was the event fully dispatched?
	bool dispatched_;
	/// was there an error?
	bool error_;
	/// do we need to redraw the screen afterwards?
	Update::flags update_;
	///
	docstring message_;
	/// 
	bool need_buf_update_;
	///
	bool need_msg_update_;
	///
	bool need_changes_update_;
};


} // namespace lyx

#endif // DISPATCH_RESULT_H
