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
class DispatchResult;
class DocumentClass;
class FuncRequest;
class FuncStatus;
class KeySymbol;
class Text;

namespace support {
class FileName;
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

	/// LyX dispatcher: executes lyx actions and returns result.
	void dispatch(FuncRequest const &, DispatchResult &);

	/// LyX dispatcher: executes lyx actions and does necessary
	/// screen updates depending on results.
	void dispatch(FuncRequest const &);

	///
	FuncStatus getStatus(FuncRequest const & action) const;

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
	/// cursor position before dispatch started
	int cursorPosBeforeDispatchX_;
	int cursorPosBeforeDispatchY_;
};

/// Implementation is in LyX.cpp
extern LyXFunc & theLyXFunc();

/// Implementation is in LyX.cpp
extern FuncStatus getStatus(FuncRequest const & action);

/// Implementation is in LyX.cpp
extern void dispatch(FuncRequest const & action);

/// Implementation is in LyX.cpp
extern void dispatch(FuncRequest const & action, DispatchResult & dr);

} // namespace lyx

#endif
