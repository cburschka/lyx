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
#include "Intl.h"
#include "KeyMap.h"
#include "Language.h"
#include "LaTeXFeatures.h"
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
#include "Server.h"
#include "Session.h"
#include "SpellChecker.h"

#include "insets/InsetCommand.h"

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
#include "support/Path.h"
#include "support/Package.h"
#include "support/Systemcall.h"
#include "support/convert.h"
#include "support/os.h"

#include <sstream>
#include <vector>

using namespace std;
using namespace lyx::support;

namespace lyx {

using frontend::LyXView;

namespace Alert = frontend::Alert;

namespace {


// This function runs "configure" and then rereads lyx.defaults to
// reconfigure the automatic settings.
void reconfigure(LyXView * lv, string const & option)
{
	// emit message signal.
	if (lv)
		lv->message(_("Running configure..."));

	// Run configure in user lyx directory
	PathChanger p(package().user_support());
	string configure_command = package().configure_command();
	configure_command += option;
	Systemcall one;
	int ret = one.startscript(Systemcall::Wait, configure_command);
	p.pop();
	// emit message signal.
	if (lv)
		lv->message(_("Reloading configuration..."));
	lyxrc.read(libFileSearch(string(), "lyxrc.defaults"));
	// Re-read packages.lst
	LaTeXFeatures::getAvailable();

	if (ret)
		Alert::information(_("System reconfiguration failed"),
			   _("The system reconfiguration has failed.\n"
				  "Default textclass is used but LyX may "
				  "not be able to work properly.\n"
				  "Please reconfigure again if needed."));
	else

		Alert::information(_("System reconfigured"),
			   _("The system has been reconfigured.\n"
			     "You need to restart LyX to make use of any\n"
			     "updated document class specifications."));
}

}


LyXFunc::LyXFunc()
	: encoded_last_key(0), meta_fake_bit(NoModifier)
{
}


void LyXFunc::initKeySequences(KeyMap * kb)
{
	keyseq = KeySequence(kb, kb);
	cancel_meta_seq = KeySequence(kb, kb);
}


void LyXFunc::handleKeyFunc(FuncCode action)
{
	char_type c = encoded_last_key;

	if (keyseq.length())
		c = 0;
	LyXView * lv = theApp()->currentWindow();
	LASSERT(lv && lv->currentBufferView(), /**/);
	BufferView * bv = lv->currentBufferView();
	bv->getIntl().getTransManager().deadkey(
		c, get_accent(action).accent, bv->cursor().innerText(),
		bv->cursor());
	// Need to clear, in case the minibuffer calls these
	// actions
	keyseq.clear();
	// copied verbatim from do_accent_char
	bv->cursor().resetAnchor();
	bv->processUpdateFlags(Update::FitCursor);
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
	// see http://bugzilla.lyx.org/show_bug.cgi?id=3092
	if (bm.bottom_pit != new_pit || bm.bottom_pos != new_pos 
		|| bm.top_id != new_id) {
		const_cast<BookmarksSection::Bookmark &>(bm).updatePos(
			new_pit, new_pos, new_id);
	}
}


void LyXFunc::processKeySym(KeySymbol const & keysym, KeyModifier state)
{
	LYXERR(Debug::KEY, "KeySym is " << keysym.getSymbolName());

	LyXView * lv = theApp()->currentWindow();

	// Do nothing if we have nothing (JMarc)
	if (!keysym.isOK()) {
		LYXERR(Debug::KEY, "Empty kbd action (probably composing)");
		lv->restartCursor();
		return;
	}

	if (keysym.isModifier()) {
		LYXERR(Debug::KEY, "isModifier true");
		if (lv)
			lv->restartCursor();
		return;
	}

	//Encoding const * encoding = lv->documentBufferView()->cursor().getEncoding();
	//encoded_last_key = keysym.getISOEncoded(encoding ? encoding->name() : "");
	// FIXME: encoded_last_key shadows the member variable of the same
	// name. Is that intended?
	char_type encoded_last_key = keysym.getUCSEncoded();

	// Do a one-deep top-level lookup for
	// cancel and meta-fake keys. RVDK_PATCH_5
	cancel_meta_seq.reset();

	FuncRequest func = cancel_meta_seq.addkey(keysym, state);
	LYXERR(Debug::KEY, "action first set to [" << func.action << ']');

	// When not cancel or meta-fake, do the normal lookup.
	// Note how the meta_fake Mod1 bit is OR-ed in and reset afterwards.
	// Mostly, meta_fake_bit = NoModifier. RVDK_PATCH_5.
	if ((func.action != LFUN_CANCEL) && (func.action != LFUN_META_PREFIX)) {
		// remove Caps Lock and Mod2 as a modifiers
		func = keyseq.addkey(keysym, (state | meta_fake_bit));
		LYXERR(Debug::KEY, "action now set to [" << func.action << ']');
	}

	// Dont remove this unless you know what you are doing.
	meta_fake_bit = NoModifier;

	// Can this happen now ?
	if (func.action == LFUN_NOACTION)
		func = FuncRequest(LFUN_COMMAND_PREFIX);

	LYXERR(Debug::KEY, " Key [action=" << func.action << "]["
		<< keyseq.print(KeySequence::Portable) << ']');

	// already here we know if it any point in going further
	// why not return already here if action == -1 and
	// num_bytes == 0? (Lgb)

	if (keyseq.length() > 1)
		lv->message(keyseq.print(KeySequence::ForGui));


	// Maybe user can only reach the key via holding down shift.
	// Let's see. But only if shift is the only modifier
	if (func.action == LFUN_UNKNOWN_ACTION && state == ShiftModifier) {
		LYXERR(Debug::KEY, "Trying without shift");
		func = keyseq.addkey(keysym, NoModifier);
		LYXERR(Debug::KEY, "Action now " << func.action);
	}

	if (func.action == LFUN_UNKNOWN_ACTION) {
		// Hmm, we didn't match any of the keysequences. See
		// if it's normal insertable text not already covered
		// by a binding
		if (keysym.isText() && keyseq.length() == 1) {
			LYXERR(Debug::KEY, "isText() is true, inserting.");
			func = FuncRequest(LFUN_SELF_INSERT,
					   FuncRequest::KEYBOARD);
		} else {
			LYXERR(Debug::KEY, "Unknown, !isText() - giving up");
			lv->message(_("Unknown function."));
			lv->restartCursor();
			return;
		}
	}

	if (func.action == LFUN_SELF_INSERT) {
		if (encoded_last_key != 0) {
			docstring const arg(1, encoded_last_key);
			dispatch(FuncRequest(LFUN_SELF_INSERT, arg,
					     FuncRequest::KEYBOARD));
			LYXERR(Debug::KEY, "SelfInsert arg[`" << to_utf8(arg) << "']");
		}
	} else {
		dispatch(func);
		if (!lv)
			return;
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

	switch (cmd.action) {
	case LFUN_UNKNOWN_ACTION:
		flag.unknown(true);
		flag.setEnabled(false);
		break;

	default:
		break;
	}

	if (flag.unknown()) {
		flag.message(from_utf8(N_("Unknown action")));
		return flag;
	}

	if (!flag.enabled()) {
		if (flag.message().empty())
			flag.message(from_utf8(N_("Command disabled")));
		return flag;
	}

	// I would really like to avoid having this switch and rather try to
	// encode this in the function itself.
	// -- And I'd rather let an inset decide which LFUNs it is willing
	// to handle (Andre')
	bool enable = true;
	switch (cmd.action) {

	case LFUN_CITATION_INSERT: {
		FuncRequest fr(LFUN_INSET_INSERT, "citation");
		enable = getStatus(fr).enabled();
		break;
	}
	
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

	case LFUN_COMMAND_PREFIX:
	case LFUN_CANCEL:
	case LFUN_META_PREFIX:
	case LFUN_RECONFIGURE:
	case LFUN_DROP_LAYOUTS_CHOICE:
	case LFUN_SERVER_GET_FILENAME:
	case LFUN_SERVER_NOTIFY:
	case LFUN_CURSOR_FOLLOWS_SCROLLBAR_TOGGLE:
	case LFUN_REPEAT:
	case LFUN_PREFERENCES_SAVE:
	case LFUN_INSET_EDIT:
	case LFUN_BUFFER_SAVE_AS_DEFAULT:
	case LFUN_LYXRC_APPLY:
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


namespace {

void actOnUpdatedPrefs(LyXRC const & lyxrc_orig, LyXRC const & lyxrc_new);

} //namespace anon


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
		Buffer * buffer = 0;
		if (lv && lv->currentBufferView())
			buffer = &lv->currentBufferView()->buffer();
		switch (action) {

		case LFUN_COMMAND_PREFIX:
			LASSERT(lv, /**/);
			lv->message(keyseq.printOptions(true));
			break;

		case LFUN_CANCEL:
			LASSERT(lv && lv->currentBufferView(), /**/);
			keyseq.reset();
			meta_fake_bit = NoModifier;
			if (buffer)
				// cancel any selection
				dispatch(FuncRequest(LFUN_MARK_OFF));
			setMessage(from_ascii(N_("Cancel")));
			break;

		case LFUN_META_PREFIX:
			meta_fake_bit = AltModifier;
			setMessage(keyseq.print(KeySequence::ForGui));
			break;

		// --- Menus -----------------------------------------------
		case LFUN_RECONFIGURE:
			// argument is any additional parameter to the configure.py command
			reconfigure(lv, argument);
			break;

		// --- lyxserver commands ----------------------------
		case LFUN_SERVER_GET_FILENAME:
			LASSERT(lv && buffer, /**/);
			setMessage(from_utf8(buffer->absFileName()));
			LYXERR(Debug::INFO, "FNAME["
				<< buffer->absFileName() << ']');
			break;

		case LFUN_SERVER_NOTIFY:
			dispatch_buffer = keyseq.print(KeySequence::Portable);
			theServer().notifyClient(to_utf8(dispatch_buffer));
			break;

		case LFUN_CITATION_INSERT: {
			LASSERT(lv, /**/);
			if (!argument.empty()) {
				// we can have one optional argument, delimited by '|'
				// citation-insert <key>|<text_before>
				// this should be enhanced to also support text_after
				// and citation style
				string arg = argument;
				string opt1;
				if (contains(argument, "|")) {
					arg = token(argument, '|', 0);
					opt1 = token(argument, '|', 1);
				}
				InsetCommandParams icp(CITE_CODE);
				icp["key"] = from_utf8(arg);
				if (!opt1.empty())
					icp["before"] = from_utf8(opt1);
				string icstr = InsetCommand::params2string("citation", icp);
				FuncRequest fr(LFUN_INSET_INSERT, icstr);
				dispatch(fr);
			} else
				dispatch(FuncRequest(LFUN_DIALOG_SHOW_NEW_INSET, "citation"));
			break;
		}

		case LFUN_CURSOR_FOLLOWS_SCROLLBAR_TOGGLE:
			LASSERT(lv, /**/);
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
			if (theBufferList().isLoaded(buffer))
				buffer->undo().beginUndoGroup();
			while (!arg.empty()) {
				string first;
				arg = split(arg, first, ';');
				FuncRequest func(lyxaction.lookupFunc(first));
				func.origin = cmd.origin;
				dispatch(func);
			}
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

		case LFUN_LYXRC_APPLY: {
			// reset active key sequences, since the bindings
			// are updated (bug 6064)
			keyseq.reset();
			LyXRC const lyxrc_orig = lyxrc;

			istringstream ss(argument);
			bool const success = lyxrc.read(ss) == 0;

			if (!success) {
				lyxerr << "Warning in LFUN_LYXRC_APPLY!\n"
				       << "Unable to read lyxrc data"
				       << endl;
				break;
			}

			actOnUpdatedPrefs(lyxrc_orig, lyxrc);

			setSpellChecker();

			theApp()->resetGui();

			/// We force the redraw in any case because there might be
			/// some screen font changes.
			/// FIXME: only the current view will be updated. the Gui
			/// class is able to furnish the list of views.
			updateFlags = Update::Force;
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
			LASSERT(theApp(), /**/);
			// Let the frontend dispatch its own actions.
			if (theApp()->dispatch(cmd))
				// Nothing more to do.
				return;

			// Everything below is only for active window
			if (lv == 0)
				break;

			// Start an undo group. This may be needed for
			// some stuff like inset-apply on labels.
			if (theBufferList().isLoaded(buffer))
				buffer->undo().beginUndoGroup();
				
			// Let the current LyXView dispatch its own actions.
			if (lv->dispatch(cmd)) {
				BufferView * bv = lv->currentBufferView();
				if (bv) {
					buffer = &(bv->buffer());
					updateFlags = bv->cursor().result().update();
					if (theBufferList().isLoaded(buffer))
						buffer->undo().endUndoGroup();
				}
				break;
			}

			BufferView * bv = lv->currentBufferView();
			LASSERT(bv, /**/);

			// Let the current BufferView dispatch its own actions.
			if (bv->dispatch(cmd)) {
				// The BufferView took care of its own updates if needed.
				buffer = &(bv->buffer());
				updateFlags = Update::None;
				if (theBufferList().isLoaded(buffer))
					buffer->undo().endUndoGroup();
				break;
			}

			BufferView * doc_bv = lv->documentBufferView();
			// Try with the document BufferView dispatch if any.
			if (doc_bv && doc_bv->dispatch(cmd)) {
				// The BufferView took care of its own updates if needed.
				buffer = &(doc_bv->buffer());
				updateFlags = Update::None;
				if (theBufferList().isLoaded(buffer))
					buffer->undo().endUndoGroup();
				break;
			}

			// OK, so try the current Buffer itself...
			DispatchResult dr;
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

			// Is this a function that acts on inset at point?
			Inset * inset = bv->cursor().nextInset();
			if (lyxaction.funcHasFlag(action, LyXAction::AtPoint)
			    && inset) {
				bv->cursor().result().dispatched(true);
				bv->cursor().result().update(Update::FitCursor | Update::Force);
				FuncRequest tmpcmd = cmd;
				inset->dispatch(bv->cursor(), tmpcmd);
				if (bv->cursor().result().dispatched()) {
					updateFlags = bv->cursor().result().update();
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

			if (theBufferList().isLoaded(buffer))
				buffer->undo().endUndoGroup();

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
		if (theBufferList().isLoaded(buffer) && flag.enabled()
		    && !lyxaction.funcHasFlag(action, LyXAction::NoBuffer)
		    && !lyxaction.funcHasFlag(action, LyXAction::ReadOnly))
			buffer->markDirty();			

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
		sendDispatchMessage(translateIfPossible(getMessage()), cmd);
	}
}


void LyXFunc::sendDispatchMessage(docstring const & msg, FuncRequest const & cmd)
{
	const bool verbose = (cmd.origin == FuncRequest::MENU
			      || cmd.origin == FuncRequest::TOOLBAR
			      || cmd.origin == FuncRequest::COMMANDBUFFER);

	LyXView * lv = theApp()->currentWindow();
	if (cmd.action == LFUN_SELF_INSERT || !verbose) {
		LYXERR(Debug::ACTION, "dispatch msg is " << to_utf8(msg));
		if (!msg.empty())
			lv->message(msg);
		return;
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

	docstring const shortcuts = theTopLevelKeymap().printBindings(cmd, KeySequence::ForGui);

	if (!shortcuts.empty())
		comname += ": " + shortcuts;
	else if (!argsadded && !cmd.argument().empty())
		comname += ' ' + cmd.argument();

	if (!comname.empty()) {
		comname = rtrim(comname);
		dispatch_msg += '(' + rtrim(comname) + ')';
	}

	LYXERR(Debug::ACTION, "verbose dispatch msg " << to_utf8(dispatch_msg));
	if (!dispatch_msg.empty())
		lv->message(dispatch_msg);
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


docstring LyXFunc::viewStatusMessage()
{
	// When meta-fake key is pressed, show the key sequence so far + "M-".
	if (wasMetaKey())
		return keyseq.print(KeySequence::ForGui) + "M-";

	// Else, when a non-complete key sequence is pressed,
	// show the available options.
	if (keyseq.length() > 0 && !keyseq.deleted())
		return keyseq.printOptions(true);

	LyXView * lv = theApp()->currentWindow();
	LASSERT(lv, /**/);
	if (!lv->currentBufferView())
		return _("Welcome to LyX!");

	return lv->currentBufferView()->cursor().currentState();
}


bool LyXFunc::wasMetaKey() const
{
	return (meta_fake_bit != NoModifier);
}


namespace {

void actOnUpdatedPrefs(LyXRC const & lyxrc_orig, LyXRC const & lyxrc_new)
{
	// Why the switch you might ask. It is a trick to ensure that all
	// the elements in the LyXRCTags enum is handled. As you can see
	// there are no breaks at all. So it is just a huge fall-through.
	// The nice thing is that we will get a warning from the compiler
	// if we forget an element.
	LyXRC::LyXRCTags tag = LyXRC::RC_LAST;
	switch (tag) {
	case LyXRC::RC_ACCEPT_COMPOUND:
	case LyXRC::RC_ALT_LANG:
	case LyXRC::RC_PLAINTEXT_LINELEN:
	case LyXRC::RC_PLAINTEXT_ROFF_COMMAND:
	case LyXRC::RC_AUTOCORRECTION_MATH:
	case LyXRC::RC_AUTOREGIONDELETE:
	case LyXRC::RC_AUTORESET_OPTIONS:
	case LyXRC::RC_AUTOSAVE:
	case LyXRC::RC_AUTO_NUMBER:
	case LyXRC::RC_BACKUPDIR_PATH:
	case LyXRC::RC_BIBTEX_ALTERNATIVES:
	case LyXRC::RC_BIBTEX_COMMAND:
	case LyXRC::RC_BINDFILE:
	case LyXRC::RC_CHECKLASTFILES:
	case LyXRC::RC_COMPLETION_CURSOR_TEXT:
	case LyXRC::RC_COMPLETION_INLINE_DELAY:
	case LyXRC::RC_COMPLETION_INLINE_DOTS:
	case LyXRC::RC_COMPLETION_INLINE_MATH:
	case LyXRC::RC_COMPLETION_INLINE_TEXT:
	case LyXRC::RC_COMPLETION_POPUP_AFTER_COMPLETE:
	case LyXRC::RC_COMPLETION_POPUP_DELAY:
	case LyXRC::RC_COMPLETION_POPUP_MATH:
	case LyXRC::RC_COMPLETION_POPUP_TEXT:
	case LyXRC::RC_USELASTFILEPOS:
	case LyXRC::RC_LOADSESSION:
	case LyXRC::RC_CHKTEX_COMMAND:
	case LyXRC::RC_CONVERTER:
	case LyXRC::RC_CONVERTER_CACHE_MAXAGE:
	case LyXRC::RC_COPIER:
	case LyXRC::RC_CURSOR_FOLLOWS_SCROLLBAR:
	case LyXRC::RC_SCROLL_BELOW_DOCUMENT:
	case LyXRC::RC_DATE_INSERT_FORMAT:
	case LyXRC::RC_DEFAULT_LANGUAGE:
	case LyXRC::RC_GUI_LANGUAGE:
	case LyXRC::RC_DEFAULT_PAPERSIZE:
	case LyXRC::RC_DEFAULT_VIEW_FORMAT:
	case LyXRC::RC_DEFFILE:
	case LyXRC::RC_DIALOGS_ICONIFY_WITH_MAIN:
	case LyXRC::RC_DISPLAY_GRAPHICS:
	case LyXRC::RC_DOCUMENTPATH:
		if (lyxrc_orig.document_path != lyxrc_new.document_path) {
			FileName path(lyxrc_new.document_path);
			if (path.exists() && path.isDirectory())
				package().document_dir() = FileName(lyxrc.document_path);
		}
	case LyXRC::RC_EDITOR_ALTERNATIVES:
	case LyXRC::RC_ESC_CHARS:
	case LyXRC::RC_EXAMPLEPATH:
	case LyXRC::RC_FONT_ENCODING:
	case LyXRC::RC_FORMAT:
	case LyXRC::RC_GROUP_LAYOUTS:
	case LyXRC::RC_HUNSPELLDIR_PATH:
	case LyXRC::RC_INDEX_ALTERNATIVES:
	case LyXRC::RC_INDEX_COMMAND:
	case LyXRC::RC_JBIBTEX_COMMAND:
	case LyXRC::RC_JINDEX_COMMAND:
	case LyXRC::RC_NOMENCL_COMMAND:
	case LyXRC::RC_INPUT:
	case LyXRC::RC_KBMAP:
	case LyXRC::RC_KBMAP_PRIMARY:
	case LyXRC::RC_KBMAP_SECONDARY:
	case LyXRC::RC_LABEL_INIT_LENGTH:
	case LyXRC::RC_LANGUAGE_AUTO_BEGIN:
	case LyXRC::RC_LANGUAGE_AUTO_END:
	case LyXRC::RC_LANGUAGE_COMMAND_BEGIN:
	case LyXRC::RC_LANGUAGE_COMMAND_END:
	case LyXRC::RC_LANGUAGE_COMMAND_LOCAL:
	case LyXRC::RC_LANGUAGE_GLOBAL_OPTIONS:
	case LyXRC::RC_LANGUAGE_PACKAGE:
	case LyXRC::RC_LANGUAGE_USE_BABEL:
	case LyXRC::RC_MAC_LIKE_WORD_MOVEMENT:
	case LyXRC::RC_MACRO_EDIT_STYLE:
	case LyXRC::RC_MAKE_BACKUP:
	case LyXRC::RC_MARK_FOREIGN_LANGUAGE:
	case LyXRC::RC_MOUSE_WHEEL_SPEED:
	case LyXRC::RC_NUMLASTFILES:
	case LyXRC::RC_PARAGRAPH_MARKERS:
	case LyXRC::RC_PATH_PREFIX:
		if (lyxrc_orig.path_prefix != lyxrc_new.path_prefix) {
			prependEnvPath("PATH", lyxrc.path_prefix);
		}
	case LyXRC::RC_PERS_DICT:
	case LyXRC::RC_PREVIEW:
	case LyXRC::RC_PREVIEW_HASHED_LABELS:
	case LyXRC::RC_PREVIEW_SCALE_FACTOR:
	case LyXRC::RC_PRINTCOLLCOPIESFLAG:
	case LyXRC::RC_PRINTCOPIESFLAG:
	case LyXRC::RC_PRINTER:
	case LyXRC::RC_PRINTEVENPAGEFLAG:
	case LyXRC::RC_PRINTEXSTRAOPTIONS:
	case LyXRC::RC_PRINTFILEEXTENSION:
	case LyXRC::RC_PRINTLANDSCAPEFLAG:
	case LyXRC::RC_PRINTODDPAGEFLAG:
	case LyXRC::RC_PRINTPAGERANGEFLAG:
	case LyXRC::RC_PRINTPAPERDIMENSIONFLAG:
	case LyXRC::RC_PRINTPAPERFLAG:
	case LyXRC::RC_PRINTREVERSEFLAG:
	case LyXRC::RC_PRINTSPOOL_COMMAND:
	case LyXRC::RC_PRINTSPOOL_PRINTERPREFIX:
	case LyXRC::RC_PRINTTOFILE:
	case LyXRC::RC_PRINTTOPRINTER:
	case LyXRC::RC_PRINT_ADAPTOUTPUT:
	case LyXRC::RC_PRINT_COMMAND:
	case LyXRC::RC_RTL_SUPPORT:
	case LyXRC::RC_SCREEN_DPI:
	case LyXRC::RC_SCREEN_FONT_ROMAN:
	case LyXRC::RC_SCREEN_FONT_ROMAN_FOUNDRY:
	case LyXRC::RC_SCREEN_FONT_SANS:
	case LyXRC::RC_SCREEN_FONT_SANS_FOUNDRY:
	case LyXRC::RC_SCREEN_FONT_SCALABLE:
	case LyXRC::RC_SCREEN_FONT_SIZES:
	case LyXRC::RC_SCREEN_FONT_TYPEWRITER:
	case LyXRC::RC_SCREEN_FONT_TYPEWRITER_FOUNDRY:
	case LyXRC::RC_GEOMETRY_SESSION:
	case LyXRC::RC_SCREEN_ZOOM:
	case LyXRC::RC_SERVERPIPE:
	case LyXRC::RC_SET_COLOR:
	case LyXRC::RC_SHOW_BANNER:
	case LyXRC::RC_OPEN_BUFFERS_IN_TABS:
	case LyXRC::RC_SPELL_COMMAND:
	case LyXRC::RC_SPELLCHECKER:
	case LyXRC::RC_SPELLCHECK_CONTINUOUSLY:
	case LyXRC::RC_SPLITINDEX_COMMAND:
	case LyXRC::RC_TEMPDIRPATH:
	case LyXRC::RC_TEMPLATEPATH:
	case LyXRC::RC_TEX_ALLOWS_SPACES:
	case LyXRC::RC_TEX_EXPECTS_WINDOWS_PATHS:
		if (lyxrc_orig.windows_style_tex_paths != lyxrc_new.windows_style_tex_paths) {
			os::windows_style_tex_paths(lyxrc_new.windows_style_tex_paths);
		}
	case LyXRC::RC_THESAURUSDIRPATH:
	case LyXRC::RC_UIFILE:
	case LyXRC::RC_USER_EMAIL:
	case LyXRC::RC_USER_NAME:
	case LyXRC::RC_USETEMPDIR:
	case LyXRC::RC_USE_ALT_LANG:
	case LyXRC::RC_USE_CONVERTER_CACHE:
	case LyXRC::RC_USE_ESC_CHARS:
	case LyXRC::RC_USE_INP_ENC:
	case LyXRC::RC_USE_PERS_DICT:
	case LyXRC::RC_USE_TOOLTIP:
	case LyXRC::RC_USE_PIXMAP_CACHE:
	case LyXRC::RC_USE_SPELL_LIB:
	case LyXRC::RC_VIEWDVI_PAPEROPTION:
	case LyXRC::RC_SORT_LAYOUTS:
	case LyXRC::RC_FULL_SCREEN_LIMIT:
	case LyXRC::RC_FULL_SCREEN_SCROLLBAR:
	case LyXRC::RC_FULL_SCREEN_MENUBAR:
	case LyXRC::RC_FULL_SCREEN_TABBAR:
	case LyXRC::RC_FULL_SCREEN_TOOLBARS:
	case LyXRC::RC_FULL_SCREEN_WIDTH:
	case LyXRC::RC_VISUAL_CURSOR:
	case LyXRC::RC_VIEWER:
	case LyXRC::RC_VIEWER_ALTERNATIVES:
	case LyXRC::RC_LAST:
		break;
	}
}

} // namespace anon
} // namespace lyx
