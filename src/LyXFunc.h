// -*- C++ -*-
/**
 * \file LyXFunc.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author John Levon
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYXFUNC_H
#define LYXFUNC_H

#include "FuncCode.h"
#include "KeySequence.h"

#include "support/docstring.h"

namespace lyx {

class Buffer;
class BufferView;
class DocumentClass;
class FuncRequest;
class FuncStatus;
class KeySymbol;
class Text;

namespace support {
class FileName;
}

namespace frontend {
class LyXView;
}

/** This class encapsulates all the LyX command operations.
    This is the class of the LyX's "high level event handler".
    Every user command is processed here, either invocated from
    keyboard or from the GUI. All GUI objects, including buttons and
    menus should use this class and never call kernel functions directly.
*/
class LyXFunc
{
public:
	///
	explicit LyXFunc();

	/// LyX dispatcher, executes lyx actions.
	void dispatch(FuncRequest const &);

	///
	void setLyXView(frontend::LyXView * lv);
	
	///
	void initKeySequences(KeyMap * kb);

	/// return the status bar state string
	docstring viewStatusMessage();

	///
	void processKeySym(KeySymbol const & key, KeyModifier state);

	///
	FuncStatus getStatus(FuncRequest const & action) const;

	/// The last key was meta
	bool wasMetaKey() const;

	/// True if lyxfunc reports an error
	bool errorStat() const { return errorstat; }
	/// Buffer to store result messages
	void setMessage(docstring const & m) const;
	/// Buffer to store result messages
	void setErrorMessage(docstring const &) const;
	/// Buffer to store result messages
	docstring const getMessage() const { return dispatch_buffer; }
	/// Handle a accented char key sequence
	void handleKeyFunc(FuncCode action);
	/// goto a bookmark
	/// openFile: whether or not open a file if the file is not opened
	/// switchToBuffer: whether or not switch to buffer if the buffer is
	///		not the current buffer
	void gotoBookmark(unsigned int idx, bool openFile, bool switchToBuffer);

	/// cursor x position before dispatch started
	int cursorBeforeDispatchX() const { return cursorPosBeforeDispatchX_; }
	/// cursor y position before dispatch started
	int cursorBeforeDispatchY() const { return cursorPosBeforeDispatchY_; }

private:
	///
	BufferView * view() const;

	///
	frontend::LyXView * lyx_view_;

	/// the last character added to the key sequence, in UCS4 encoded form
	char_type encoded_last_key;

	///
	KeySequence keyseq;
	///
	KeySequence cancel_meta_seq;
	///
	KeyModifier meta_fake_bit;

	/// cursor position before dispatch started
	int cursorPosBeforeDispatchX_;
	int cursorPosBeforeDispatchY_;

	/// Error status, only Dispatch can change this flag
	mutable bool errorstat;

	/** Buffer to store messages and result data. Is there a
	    good reason to have this one as static in Dispatch? (Ale)
	*/
	mutable docstring dispatch_buffer;

	/// send a post-dispatch status message
	void sendDispatchMessage(docstring const & msg,
		FuncRequest const & ev);

	///
	void reloadBuffer();
	///
	bool ensureBufferClean(BufferView * bv);
	///
	void updateLayout(DocumentClass const * const oldlayout, Buffer * buffer);
};

/// Implementation is in LyX.cpp
extern LyXFunc & theLyXFunc();

/// Implementation is in LyX.cpp
extern FuncStatus getStatus(FuncRequest const & action);

/// Implementation is in LyX.cpp
extern void dispatch(FuncRequest const & action);

} // namespace lyx

#endif
