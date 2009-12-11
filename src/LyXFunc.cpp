/**
 * \file LyXFunc.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alfredo Braunstein
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author Angus Leeming
 * \author John Levon
 * \author André Pönitz
 * \author Allan Rae
 * \author Dekel Tsur
 * \author Martin Vermeer
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "LyXFunc.h"

#include "LayoutFile.h"
#include "BranchList.h"
#include "buffer_funcs.h"
#include "Buffer.h"
#include "BufferList.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "CmdDef.h"
#include "Color.h"
#include "Converter.h"
#include "Cursor.h"
#include "CutAndPaste.h"
#include "DispatchResult.h"
#include "Encoding.h"
#include "ErrorList.h"
#include "Format.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "InsetIterator.h"
#include "KeyMap.h"
#include "Language.h"
#include "Lexer.h"
#include "LyXAction.h"
#include "lyxfind.h"
#include "LyX.h"
#include "LyXRC.h"
#include "LyXVC.h"
#include "Paragraph.h"
#include "ParagraphParameters.h"
#include "ParIterator.h"
#include "Row.h"
#include "Session.h"
#include "SpellChecker.h"

#include "frontends/alert.h"
#include "frontends/Application.h"
#include "frontends/KeySymbol.h"
#include "frontends/LyXView.h"
#include "frontends/Selection.h"

#include "support/debug.h"
#include "support/environment.h"
#include "support/FileName.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/lassert.h"
#include "support/lstrings.h"
#include "support/Package.h"
#include "support/convert.h"
#include "support/os.h"

#include <sstream>
#include <vector>

using namespace std;
using namespace lyx::support;

namespace lyx {

using frontend::LyXView;

namespace Alert = frontend::Alert;

LyXFunc::LyXFunc()
{
}


//FIXME: bookmark handling is a frontend issue. This code should be transferred
// to GuiView and be GuiView and be window dependent.
void LyXFunc::gotoBookmark(unsigned int idx, bool openFile, bool switchToBuffer)
{
	LyXView * lv = theApp()->currentWindow();
	LASSERT(lv, /**/);
	if (!theSession().bookmarks().isValid(idx))
		return;
	BookmarksSection::Bookmark const & bm = theSession().bookmarks().bookmark(idx);
	LASSERT(!bm.filename.empty(), /**/);
	string const file = bm.filename.absFilename();
	// if the file is not opened, open it.
	if (!theBufferList().exists(bm.filename)) {
		if (openFile)
			dispatch(FuncRequest(LFUN_FILE_OPEN, file));
		else
			return;
	}
	// open may fail, so we need to test it again
	if (!theBufferList().exists(bm.filename))
		return;

	// bm can be changed when saving
	BookmarksSection::Bookmark tmp = bm;

	// Special case idx == 0 used for back-from-back jump navigation
	if (idx == 0)
		dispatch(FuncRequest(LFUN_BOOKMARK_SAVE, "0"));

	// if the current buffer is not that one, switch to it.
	if (!lv->documentBufferView()
		|| lv->documentBufferView()->buffer().fileName() != tmp.filename) {
		if (!switchToBuffer)
			return;
		dispatch(FuncRequest(LFUN_BUFFER_SWITCH, file));
	}

	// moveToPosition try paragraph id first and then paragraph (pit, pos).
	if (!lv->documentBufferView()->moveToPosition(
		tmp.bottom_pit, tmp.bottom_pos, tmp.top_id, tmp.top_pos))
		return;

	// bm changed
	if (idx == 0)
		return;

	// Cursor jump succeeded!
	Cursor const & cur = lv->documentBufferView()->cursor();
	pit_type new_pit = cur.pit();
	pos_type new_pos = cur.pos();
	int new_id = cur.paragraph().id();

	// if bottom_pit, bottom_pos or top_id has been changed, update bookmark
	// see http://www.lyx.org/trac/ticket/3092
	if (bm.bottom_pit != new_pit || bm.bottom_pos != new_pos 
		|| bm.top_id != new_id) {
		const_cast<BookmarksSection::Bookmark &>(bm).updatePos(
			new_pit, new_pos, new_id);
	}
}


FuncStatus LyXFunc::getStatus(FuncRequest const & cmd) const
{
	//lyxerr << "LyXFunc::getStatus: cmd: " << cmd << endl;
	FuncStatus flag;

	if (cmd.action == LFUN_NOACTION) {
		flag.message(from_utf8(N_("Nothing to do")));
		flag.setEnabled(false);
		return flag;
	}

	if (cmd.action == LFUN_UNKNOWN_ACTION) {
		flag.unknown(true);
		flag.setEnabled(false);
		flag.message(from_utf8(N_("Unknown action")));
		return flag;
	}

	// I would really like to avoid having this switch and rather try to
	// encode this in the function itself.
	// -- And I'd rather let an inset decide which LFUNs it is willing
	// to handle (Andre')
	bool enable = true;
	switch (cmd.action) {

	// This could be used for the no-GUI version. The GUI version is handled in
	// LyXView::getStatus(). See above.
	/*
	case LFUN_BUFFER_WRITE:
	case LFUN_BUFFER_WRITE_AS: {
		Buffer * b = theBufferList().getBuffer(FileName(cmd.getArg(0)));
		enable = b && (b->isUnnamed() || !b->isClean());
		break;
	}
	*/

	case LFUN_BOOKMARK_GOTO: {
		const unsigned int num = convert<unsigned int>(to_utf8(cmd.argument()));
		enable = theSession().bookmarks().isValid(num);
		break;
	}

	case LFUN_BOOKMARK_CLEAR:
		enable = theSession().bookmarks().hasValid();
		break;

	// this one is difficult to get right. As a half-baked
	// solution, we consider only the first action of the sequence
	case LFUN_COMMAND_SEQUENCE: {
		// argument contains ';'-terminated commands
		string const firstcmd = token(to_utf8(cmd.argument()), ';', 0);
		FuncRequest func(lyxaction.lookupFunc(firstcmd));
		func.origin = cmd.origin;
		flag = getStatus(func);
		break;
	}

	// we want to check if at least one of these is enabled
	case LFUN_COMMAND_ALTERNATIVES: {
		// argument contains ';'-terminated commands
		string arg = to_utf8(cmd.argument());
		while (!arg.empty()) {
			string first;
			arg = split(arg, first, ';');
			FuncRequest func(lyxaction.lookupFunc(first));
			func.origin = cmd.origin;
			flag = getStatus(func);
			// if this one is enabled, the whole thing is
			if (flag.enabled())
				break;
		}
		break;
	}

	case LFUN_CALL: {
		FuncRequest func;
		string name = to_utf8(cmd.argument());
		if (theTopLevelCmdDef().lock(name, func)) {
			func.origin = cmd.origin;
			flag = getStatus(func);
			theTopLevelCmdDef().release(name);
		} else {
			// catch recursion or unknown command
			// definition. all operations until the
			// recursion or unknown command definition
			// occurs are performed, so set the state to
			// enabled
			enable = true;
		}
		break;
	}

	case LFUN_CURSOR_FOLLOWS_SCROLLBAR_TOGGLE:
	case LFUN_REPEAT:
	case LFUN_PREFERENCES_SAVE:
	case LFUN_BUFFER_SAVE_AS_DEFAULT:
		// these are handled in our dispatch()
		break;

	default:
		if (!theApp()) {
			enable = false;
			break;
		}
		if (theApp()->getStatus(cmd, flag))
			break;

		// Does the view know something?
		LyXView * lv = theApp()->currentWindow();
		if (!lv) {
			enable = false;
			break;
		}
		if (lv->getStatus(cmd, flag))
			break;

		BufferView * bv = lv->currentBufferView();
		BufferView * doc_bv = lv->documentBufferView();
		// If we do not have a BufferView, then other functions are disabled
		if (!bv) {
			enable = false;
			break;
		}
		// try the BufferView
		bool decided = bv->getStatus(cmd, flag);
		if (!decided)
			// try the Buffer
			decided = bv->buffer().getStatus(cmd, flag);
		if (!decided && doc_bv)
			// try the Document Buffer
			decided = doc_bv->buffer().getStatus(cmd, flag);
	}

	if (!enable)
		flag.setEnabled(false);

	// the default error message if we disable the command
	if (!flag.enabled() && flag.message().empty())
		flag.message(from_utf8(N_("Command disabled")));

	return flag;
}

/// send a post-dispatch status message
static docstring sendDispatchMessage(docstring const & msg, FuncRequest const & cmd)
{
	const bool verbose = (cmd.origin == FuncRequest::MENU
			      || cmd.origin == FuncRequest::TOOLBAR
			      || cmd.origin == FuncRequest::COMMANDBUFFER);

	if (cmd.action == LFUN_SELF_INSERT || !verbose) {
		LYXERR(Debug::ACTION, "dispatch msg is " << msg);
		return msg;
	}

	docstring dispatch_msg = msg;
	if (!dispatch_msg.empty())
		dispatch_msg += ' ';

	docstring comname = from_utf8(lyxaction.getActionName(cmd.action));

	bool argsadded = false;

	if (!cmd.argument().empty()) {
		if (cmd.action != LFUN_UNKNOWN_ACTION) {
			comname += ' ' + cmd.argument();
			argsadded = true;
		}
	}
	docstring const shortcuts = theTopLevelKeymap().
		printBindings(cmd, KeySequence::ForGui);

	if (!shortcuts.empty())
		comname += ": " + shortcuts;
	else if (!argsadded && !cmd.argument().empty())
		comname += ' ' + cmd.argument();

	if (!comname.empty()) {
		comname = rtrim(comname);
		dispatch_msg += '(' + rtrim(comname) + ')';
	}
	LYXERR(Debug::ACTION, "verbose dispatch msg " << to_utf8(dispatch_msg));
	return dispatch_msg;
}


void LyXFunc::dispatch(FuncRequest const & cmd)
{
	string const argument = to_utf8(cmd.argument());
	FuncCode const action = cmd.action;

	LYXERR(Debug::ACTION, "\nLyXFunc::dispatch: cmd: " << cmd);
	//lyxerr << "LyXFunc::dispatch: cmd: " << cmd << endl;

	// we have not done anything wrong yet.
	errorstat = false;
	dispatch_buffer.erase();

	// redraw the screen at the end (first of the two drawing steps).
	//This is done unless explicitely requested otherwise
	Update::flags updateFlags = Update::FitCursor;

	LyXView * lv = theApp()->currentWindow();

	FuncStatus const flag = getStatus(cmd);
	if (!flag.enabled()) {
		// We cannot use this function here
		LYXERR(Debug::ACTION, "LyXFunc::dispatch: "
		       << lyxaction.getActionName(action)
		       << " [" << action << "] is disabled at this location");
		setErrorMessage(flag.message());
		if (lv)
			lv->restartCursor();
	} else {
		switch (action) {

		case LFUN_CURSOR_FOLLOWS_SCROLLBAR_TOGGLE:
			lyxrc.cursor_follows_scrollbar = !lyxrc.cursor_follows_scrollbar;
			break;

		case LFUN_REPEAT: {
			// repeat command
			string countstr;
			string rest = split(argument, countstr, ' ');
			istringstream is(countstr);
			int count = 0;
			is >> count;
			//lyxerr << "repeat: count: " << count << " cmd: " << rest << endl;
			for (int i = 0; i < count; ++i)
				dispatch(lyxaction.lookupFunc(rest));
			break;
		}

		case LFUN_COMMAND_SEQUENCE: {
			// argument contains ';'-terminated commands
			string arg = argument;
			// FIXME: this LFUN should also work without any view.
			Buffer * buffer = (lv && lv->documentBufferView())
				? &(lv->documentBufferView()->buffer()) : 0;
			if (buffer)
				buffer->undo().beginUndoGroup();
			while (!arg.empty()) {
				string first;
				arg = split(arg, first, ';');
				FuncRequest func(lyxaction.lookupFunc(first));
				func.origin = cmd.origin;
				dispatch(func);
			}
			// the buffer may have been closed by one action
			if (theBufferList().isLoaded(buffer))
				buffer->undo().endUndoGroup();
			break;
		}

		case LFUN_COMMAND_ALTERNATIVES: {
			// argument contains ';'-terminated commands
			string arg = argument;
			while (!arg.empty()) {
				string first;
				arg = split(arg, first, ';');
				FuncRequest func(lyxaction.lookupFunc(first));
				func.origin = cmd.origin;
				FuncStatus stat = getStatus(func);
				if (stat.enabled()) {
					dispatch(func);
					break;
				}
			}
			break;
		}

		case LFUN_CALL: {
			FuncRequest func;
			if (theTopLevelCmdDef().lock(argument, func)) {
				func.origin = cmd.origin;
				dispatch(func);
				theTopLevelCmdDef().release(argument);
			} else {
				if (func.action == LFUN_UNKNOWN_ACTION) {
					// unknown command definition
					lyxerr << "Warning: unknown command definition `"
						   << argument << "'"
						   << endl;
				} else {
					// recursion detected
					lyxerr << "Warning: Recursion in the command definition `"
						   << argument << "' detected"
						   << endl;
				}
			}
			break;
		}

		case LFUN_PREFERENCES_SAVE: {
			lyxrc.write(makeAbsPath("preferences",
						package().user_support().absFilename()),
				    false);
			break;
		}

		case LFUN_BUFFER_SAVE_AS_DEFAULT: {
			string const fname =
				addName(addPath(package().user_support().absFilename(), "templates/"),
					"defaults.lyx");
			Buffer defaults(fname);

			istringstream ss(argument);
			Lexer lex;
			lex.setStream(ss);
			int const unknown_tokens = defaults.readHeader(lex);

			if (unknown_tokens != 0) {
				lyxerr << "Warning in LFUN_BUFFER_SAVE_AS_DEFAULT!\n"
				       << unknown_tokens << " unknown token"
				       << (unknown_tokens == 1 ? "" : "s")
				       << endl;
			}

			if (defaults.writeFile(FileName(defaults.absFileName())))
				setMessage(bformat(_("Document defaults saved in %1$s"),
						   makeDisplayPath(fname)));
			else
				setErrorMessage(from_ascii(N_("Unable to save document defaults")));
			break;
		}

		case LFUN_BOOKMARK_GOTO:
			// go to bookmark, open unopened file and switch to buffer if necessary
			gotoBookmark(convert<unsigned int>(to_utf8(cmd.argument())), true, true);
			updateFlags = Update::FitCursor;
			break;

		case LFUN_BOOKMARK_CLEAR:
			theSession().bookmarks().clear();
			break;

		default:
			DispatchResult dr;

			LASSERT(theApp(), /**/);
			// Let the frontend dispatch its own actions.
			theApp()->dispatch(cmd, dr);
			if (dr.dispatched())
				// Nothing more to do.
				break;

			// Everything below is only for active window
			if (lv == 0)
				break;

			// Let the current LyXView dispatch its own actions.
			if (lv->dispatch(cmd)) {
				BufferView * bv = lv->currentBufferView();
				if (bv)
					updateFlags = bv->cursor().result().update();
				break;
			}

			BufferView * bv = lv->currentBufferView();
			LASSERT(bv, /**/);

			// Let the current BufferView dispatch its own actions.
			if (bv->dispatch(cmd)) {
				// The BufferView took care of its own updates if needed.
				updateFlags = Update::None;
				break;
			}

			BufferView * doc_bv = lv->documentBufferView();
			// Try with the document BufferView dispatch if any.
			if (doc_bv && doc_bv->dispatch(cmd)) {
				updateFlags = Update::None;
				break;
			}

			// OK, so try the current Buffer itself...
			bv->buffer().dispatch(cmd, dr);
			if (dr.dispatched()) {
				updateFlags = dr.update();
				break;
			}
			// and with the document Buffer.
			if (doc_bv) {
				doc_bv->buffer().dispatch(cmd, dr);
				if (dr.dispatched()) {
					updateFlags = dr.update();
					break;
				}
			}

			// Let the current Cursor dispatch its own actions.
			Cursor old = bv->cursor();
			bv->cursor().getPos(cursorPosBeforeDispatchX_,
						cursorPosBeforeDispatchY_);
			bv->cursor().dispatch(cmd);

			// notify insets we just left
			if (bv->cursor() != old) {
				old.fixIfBroken();
				bool badcursor = notifyCursorLeavesOrEnters(old, bv->cursor());
				if (badcursor)
					bv->cursor().fixIfBroken();
			}

			// update completion. We do it here and not in
			// processKeySym to avoid another redraw just for a
			// changed inline completion
			if (cmd.origin == FuncRequest::KEYBOARD) {
				if (cmd.action == LFUN_SELF_INSERT
				    || (cmd.action == LFUN_ERT_INSERT && bv->cursor().inMathed()))
					lv->updateCompletion(bv->cursor(), true, true);
				else if (cmd.action == LFUN_CHAR_DELETE_BACKWARD)
					lv->updateCompletion(bv->cursor(), false, true);
				else
					lv->updateCompletion(bv->cursor(), false, false);
			}

			updateFlags = bv->cursor().result().update();
		}

		// if we executed a mutating lfun, mark the buffer as dirty
		Buffer * doc_buffer = (lv && lv->documentBufferView())
			? &(lv->documentBufferView()->buffer()) : 0;
		if (doc_buffer && theBufferList().isLoaded(doc_buffer)
			&& flag.enabled()
		    && !lyxaction.funcHasFlag(action, LyXAction::NoBuffer)
		    && !lyxaction.funcHasFlag(action, LyXAction::ReadOnly))
			doc_buffer->markDirty();			

		if (lv && lv->currentBufferView()) {
			// BufferView::update() updates the ViewMetricsInfo and
			// also initializes the position cache for all insets in
			// (at least partially) visible top-level paragraphs.
			// We will redraw the screen only if needed.
			lv->currentBufferView()->processUpdateFlags(updateFlags);

			// Do we have a selection?
			theSelection().haveSelection(
				lv->currentBufferView()->cursor().selection());
			
			// update gui
			lv->restartCursor();
		}
	}
	if (lv) {
		// Some messages may already be translated, so we cannot use _()
		lv->message(sendDispatchMessage(
			translateIfPossible(getMessage()), cmd));
	}
}


// Each LyXView should have it's own message method. lyxview and
// the minibuffer would use the minibuffer, but lyxserver would
// send an ERROR signal to its client.  Alejandro 970603
// This function is bit problematic when it comes to NLS, to make the
// lyx servers client be language indepenent we must not translate
// strings sent to this func.
void LyXFunc::setErrorMessage(docstring const & m) const
{
	dispatch_buffer = m;
	errorstat = true;
}


void LyXFunc::setMessage(docstring const & m) const
{
	dispatch_buffer = m;
}


} // namespace lyx
