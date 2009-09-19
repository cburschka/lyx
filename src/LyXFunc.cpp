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

#include "insets/InsetBox.h"
#include "insets/InsetBranch.h"
#include "insets/InsetCommand.h"
#include "insets/InsetERT.h"
#include "insets/InsetExternal.h"
#include "insets/InsetFloat.h"
#include "insets/InsetGraphics.h"
#include "insets/InsetInclude.h"
#include "insets/InsetListings.h"
#include "insets/InsetNote.h"
#include "insets/InsetPhantom.h"
#include "insets/InsetSpace.h"
#include "insets/InsetTabular.h"
#include "insets/InsetVSpace.h"
#include "insets/InsetWrap.h"

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


bool getLocalStatus(Cursor cursor, FuncRequest const & cmd, FuncStatus & status)
{
	// Try to fix cursor in case it is broken.
	cursor.fixIfBroken();

	// This is, of course, a mess. Better create a new doc iterator and use
	// this in Inset::getStatus. This might require an additional
	// BufferView * arg, though (which should be avoided)
	//Cursor safe = *this;
	bool res = false;
	for ( ; cursor.depth(); cursor.pop()) {
		//lyxerr << "\nCursor::getStatus: cmd: " << cmd << endl << *this << endl;
		LASSERT(cursor.idx() <= cursor.lastidx(), /**/);
		LASSERT(cursor.pit() <= cursor.lastpit(), /**/);
		LASSERT(cursor.pos() <= cursor.lastpos(), /**/);

		// The inset's getStatus() will return 'true' if it made
		// a definitive decision on whether it want to handle the
		// request or not. The result of this decision is put into
		// the 'status' parameter.
		if (cursor.inset().getStatus(cursor, cmd, status)) {
			res = true;
			break;
		}
	}
	return res;
}


/** Return the change status at cursor position, taking in account the
 * status at each level of the document iterator (a table in a deleted
 * footnote is deleted).
 * When \param outer is true, the top slice is not looked at.
 */
Change::Type lookupChangeType(DocIterator const & dit, bool outer = false)
{
	size_t const depth = dit.depth() - (outer ? 1 : 0);

	for (size_t i = 0 ; i < depth ; ++i) {
		CursorSlice const & slice = dit[i];
		if (!slice.inset().inMathed()
		    && slice.pos() < slice.paragraph().size()) {
			Change::Type const ch = slice.paragraph().lookupChange(slice.pos()).type;
			if (ch != Change::UNCHANGED)
				return ch;
		}
	}
	return Change::UNCHANGED;
}

}


LyXFunc::LyXFunc()
	: lyx_view_(0), encoded_last_key(0), meta_fake_bit(NoModifier)
{
}


void LyXFunc::initKeySequences(KeyMap * kb)
{
	keyseq = KeySequence(kb, kb);
	cancel_meta_seq = KeySequence(kb, kb);
}


void LyXFunc::setLyXView(LyXView * lv)
{
	if (lyx_view_ && lyx_view_->currentBufferView() && lyx_view_ != lv)
		// save current selection to the selection buffer to allow
		// middle-button paste in another window
		cap::saveSelection(lyx_view_->currentBufferView()->cursor());
	lyx_view_ = lv;
}


void LyXFunc::handleKeyFunc(FuncCode action)
{
	char_type c = encoded_last_key;

	if (keyseq.length())
		c = 0;

	LASSERT(lyx_view_ && lyx_view_->currentBufferView(), /**/);
	BufferView * bv = lyx_view_->currentBufferView();
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
	LASSERT(lyx_view_, /**/);
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
	if (!lyx_view_->documentBufferView()
		|| lyx_view_->documentBufferView()->buffer().fileName() != tmp.filename) {
		if (!switchToBuffer)
			return;
		dispatch(FuncRequest(LFUN_BUFFER_SWITCH, file));
	}

	// moveToPosition try paragraph id first and then paragraph (pit, pos).
	if (!lyx_view_->documentBufferView()->moveToPosition(
		tmp.bottom_pit, tmp.bottom_pos, tmp.top_id, tmp.top_pos))
		return;

	// bm changed
	if (idx == 0)
		return;

	// Cursor jump succeeded!
	Cursor const & cur = lyx_view_->documentBufferView()->cursor();
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

	// Do nothing if we have nothing (JMarc)
	if (!keysym.isOK()) {
		LYXERR(Debug::KEY, "Empty kbd action (probably composing)");
		lyx_view_->restartCursor();
		return;
	}

	if (keysym.isModifier()) {
		LYXERR(Debug::KEY, "isModifier true");
		if (lyx_view_)
			lyx_view_->restartCursor();
		return;
	}

	//Encoding const * encoding = lyx_view_->documentBufferView()->cursor().getEncoding();
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
		lyx_view_->message(keyseq.print(KeySequence::ForGui));


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
			lyx_view_->message(_("Unknown function."));
			lyx_view_->restartCursor();
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
		if (!lyx_view_)
			return;
	}
}


FuncStatus LyXFunc::getStatus(FuncRequest const & cmd) const
{
	//lyxerr << "LyXFunc::getStatus: cmd: " << cmd << endl;
	FuncStatus flag;

	/* In LyX/Mac, when a dialog is open, the menus of the
	   application can still be accessed without giving focus to
	   the main window. In this case, we want to disable the menu
	   entries that are buffer or view-related.

	   If this code is moved somewhere else (like in
	   GuiView::getStatus), then several functions will not be
	   handled correctly.
	*/
	frontend::LyXView * lv = 0;
	Buffer * buf = 0;
	if (lyx_view_ 
	    && (cmd.origin != FuncRequest::MENU || lyx_view_->hasFocus())) {
		lv = lyx_view_;
		if (lyx_view_->documentBufferView())
			buf = &lyx_view_->documentBufferView()->buffer();
	}

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

	// Check whether we need a buffer
	if (!lyxaction.funcHasFlag(cmd.action, LyXAction::NoBuffer) && !buf) {
		// no, exit directly
		flag.message(from_utf8(N_("Command not allowed with"
				    "out any document open")));
		flag.setEnabled(false);
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

	case LFUN_BUFFER_WRITE_ALL: {
		// We enable the command only if there are some modified buffers
		Buffer * first = theBufferList().first();
		enable = false;
		if (!first)
			break;
		Buffer * b = first;
		// We cannot use a for loop as the buffer list is a cycle.
		do {
			if (!b->isClean()) {
				enable = true;
				break;
			}
			b = theBufferList().next(b);
		} while (b != first); 
		break;
	}

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
	case LFUN_HELP_OPEN:
	case LFUN_DROP_LAYOUTS_CHOICE:
	case LFUN_SERVER_GET_FILENAME:
	case LFUN_SERVER_NOTIFY:
	case LFUN_SERVER_GOTO_FILE_ROW:
	case LFUN_CURSOR_FOLLOWS_SCROLLBAR_TOGGLE:
	case LFUN_REPEAT:
	case LFUN_PREFERENCES_SAVE:
	case LFUN_INSET_EDIT:
	case LFUN_TEXTCLASS_APPLY:
	case LFUN_TEXTCLASS_LOAD:
	case LFUN_BUFFER_SAVE_AS_DEFAULT:
	case LFUN_LAYOUT_MODULES_CLEAR:
	case LFUN_LAYOUT_MODULE_ADD:
	case LFUN_LAYOUT_RELOAD:
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
		// Is this a function that acts on inset at point?
		Inset * inset = bv->cursor().nextInset();
		if (lyxaction.funcHasFlag(cmd.action, LyXAction::AtPoint)
		    && inset && inset->getStatus(bv->cursor(), cmd, flag))
			break;

		bool decided = getLocalStatus(bv->cursor(), cmd, flag);
		if (!decided)
			// try the BufferView
			decided = bv->getStatus(cmd, flag);
		if (!decided)
			// try the Buffer
			decided = bv->buffer().getStatus(cmd, flag);
		if (!decided && doc_bv)
			// try the Document Buffer
			decided = doc_bv->buffer().getStatus(cmd, flag);
	}

	if (!enable)
		flag.setEnabled(false);

	// Can we use a readonly buffer?
	if (buf && buf->isReadonly()
	    && !lyxaction.funcHasFlag(cmd.action, LyXAction::ReadOnly)
	    && !lyxaction.funcHasFlag(cmd.action, LyXAction::NoBuffer)) {
		flag.message(from_utf8(N_("Document is read-only")));
		flag.setEnabled(false);
	}

	// Are we in a DELETED change-tracking region?
	if (lyx_view_ && lyx_view_->documentBufferView()
		&& (lookupChangeType(lyx_view_->documentBufferView()->cursor(), true)
		    == Change::DELETED)
	    && !lyxaction.funcHasFlag(cmd.action, LyXAction::ReadOnly)
	    && !lyxaction.funcHasFlag(cmd.action, LyXAction::NoBuffer)) {
		flag.message(from_utf8(N_("This portion of the document is deleted.")));
		flag.setEnabled(false);
	}

	// the default error message if we disable the command
	if (!flag.enabled() && flag.message().empty())
		flag.message(from_utf8(N_("Command disabled")));

	return flag;
}


namespace {

bool loadLayoutFile(string const & name, string const & buf_path)
{
	if (!LayoutFileList::get().haveClass(name)) {
		lyxerr << "Document class \"" << name
		       << "\" does not exist."
		       << endl;
		return false;
	}

	LayoutFile & tc = LayoutFileList::get()[name];
	if (!tc.load(buf_path)) {
		docstring s = bformat(_("The document class %1$s "
				   "could not be loaded."), from_utf8(name));
		Alert::error(_("Could not load class"), s);
		return false;
	}
	return true;
}


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

	FuncStatus const flag = getStatus(cmd);
	if (!flag.enabled()) {
		// We cannot use this function here
		LYXERR(Debug::ACTION, "LyXFunc::dispatch: "
		       << lyxaction.getActionName(action)
		       << " [" << action << "] is disabled at this location");
		setErrorMessage(flag.message());
		if (lyx_view_)
			lyx_view_->restartCursor();
	} else {
		Buffer * buffer = 0;
		if (lyx_view_ && lyx_view_->currentBufferView())
			buffer = &lyx_view_->currentBufferView()->buffer();
		switch (action) {

		case LFUN_COMMAND_PREFIX:
			LASSERT(lyx_view_, /**/);
			lyx_view_->message(keyseq.printOptions(true));
			break;

		case LFUN_CANCEL:
			LASSERT(lyx_view_ && lyx_view_->currentBufferView(), /**/);
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
			reconfigure(lyx_view_, argument);
			break;

		case LFUN_HELP_OPEN: {
			if (lyx_view_ == 0)
				theApp()->dispatch(FuncRequest(LFUN_WINDOW_NEW));
			string const arg = argument;
			if (arg.empty()) {
				setErrorMessage(from_utf8(N_("Missing argument")));
				break;
			}
			FileName fname = i18nLibFileSearch("doc", arg, "lyx");
			if (fname.empty()) 
				fname = i18nLibFileSearch("examples", arg, "lyx");

			if (fname.empty()) {
				lyxerr << "LyX: unable to find documentation file `"
							 << arg << "'. Bad installation?" << endl;
				break;
			}
			lyx_view_->message(bformat(_("Opening help file %1$s..."),
				makeDisplayPath(fname.absFilename())));
			Buffer * buf = lyx_view_->loadDocument(fname, false);
			if (buf) {
				buf->updateLabels();
				lyx_view_->setBuffer(buf);
				buf->errors("Parse");
			}
			updateFlags = Update::None;
			break;
		}

		// --- lyxserver commands ----------------------------
		case LFUN_SERVER_GET_FILENAME:
			LASSERT(lyx_view_ && buffer, /**/);
			setMessage(from_utf8(buffer->absFileName()));
			LYXERR(Debug::INFO, "FNAME["
				<< buffer->absFileName() << ']');
			break;

		case LFUN_SERVER_NOTIFY:
			dispatch_buffer = keyseq.print(KeySequence::Portable);
			theServer().notifyClient(to_utf8(dispatch_buffer));
			break;

		case LFUN_SERVER_GOTO_FILE_ROW: {
			LASSERT(lyx_view_, /**/);
			string file_name;
			int row;
			istringstream is(argument);
			is >> file_name >> row;
			file_name = os::internal_path(file_name);
			Buffer * buf = 0;
			bool loaded = false;
			string const abstmp = package().temp_dir().absFilename();
			string const realtmp = package().temp_dir().realPath();
			// We have to use os::path_prefix_is() here, instead of
			// simply prefixIs(), because the file name comes from
			// an external application and may need case adjustment.
			if (os::path_prefix_is(file_name, abstmp, os::CASE_ADJUSTED)
			    || os::path_prefix_is(file_name, realtmp, os::CASE_ADJUSTED)) {
				// Needed by inverse dvi search. If it is a file
				// in tmpdir, call the apropriated function.
				// If tmpdir is a symlink, we may have the real
				// path passed back, so we correct for that.
				if (!prefixIs(file_name, abstmp))
					file_name = subst(file_name, realtmp, abstmp);
				buf = theBufferList().getBufferFromTmp(file_name);
			} else {
				// Must replace extension of the file to be .lyx
				// and get full path
				FileName const s = fileSearch(string(), changeExtension(file_name, ".lyx"), "lyx");
				// Either change buffer or load the file
				if (theBufferList().exists(s))
					buf = theBufferList().getBuffer(s);
				else if (s.exists()) {
					buf = lyx_view_->loadDocument(s);
					loaded = true;
				} else
					lyx_view_->message(bformat(
						_("File does not exist: %1$s"),
						makeDisplayPath(file_name)));
			}

			if (!buf) {
				updateFlags = Update::None;
				break;
			}

			buf->updateLabels();
			lyx_view_->setBuffer(buf);
			lyx_view_->documentBufferView()->setCursorFromRow(row);
			if (loaded)
				buf->errors("Parse");
			updateFlags = Update::FitCursor;
			break;
		}


		case LFUN_DIALOG_SHOW_NEW_INSET: {
			LASSERT(lyx_view_, /**/);
			string const name = cmd.getArg(0);
			InsetCode code = insetCode(name);
			string data = trim(to_utf8(cmd.argument()).substr(name.size()));
			bool insetCodeOK = true;
			switch (code) {
			case BIBITEM_CODE:
			case BIBTEX_CODE:
			case INDEX_CODE:
			case LABEL_CODE:
			case NOMENCL_CODE:
			case NOMENCL_PRINT_CODE:
			case REF_CODE:
			case TOC_CODE:
			case HYPERLINK_CODE: {
				InsetCommandParams p(code);
				data = InsetCommand::params2string(name, p);
				break;
			}
			case INCLUDE_CODE: {
				// data is the include type: one of "include",
				// "input", "verbatiminput" or "verbatiminput*"
				if (data.empty())
					// default type is requested
					data = "include";
				InsetCommandParams p(INCLUDE_CODE, data);
				data = InsetCommand::params2string("include", p);
				break;
			}
			case BOX_CODE: {
				// \c data == "Boxed" || "Frameless" etc
				InsetBoxParams p(data);
				data = InsetBox::params2string(p);
				break;
			}
			case BRANCH_CODE: {
				InsetBranchParams p;
				data = InsetBranch::params2string(p);
				break;
			}
			case CITE_CODE: {
				InsetCommandParams p(CITE_CODE);
				data = InsetCommand::params2string(name, p);
				break;
			}
			case ERT_CODE: {
				data = InsetERT::params2string(InsetCollapsable::Open);
				break;
			}
			case EXTERNAL_CODE: {
				InsetExternalParams p;
				data = InsetExternal::params2string(p, *buffer);
				break;
			}
			case FLOAT_CODE:  {
				InsetFloatParams p;
				data = InsetFloat::params2string(p);
				break;
			}
			case LISTINGS_CODE: {
				InsetListingsParams p;
				data = InsetListings::params2string(p);
				break;
			}
			case GRAPHICS_CODE: {
				InsetGraphicsParams p;
				data = InsetGraphics::params2string(p, *buffer);
				break;
			}
			case NOTE_CODE: {
				InsetNoteParams p;
				data = InsetNote::params2string(p);
				break;
			}
			case PHANTOM_CODE: {
				InsetPhantomParams p;
				data = InsetPhantom::params2string(p);
				break;
			}
			case SPACE_CODE: {
				InsetSpaceParams p;
				data = InsetSpace::params2string(p);
				break;
			}
			case VSPACE_CODE: {
				VSpace space;
				data = InsetVSpace::params2string(space);
				break;
			}
			case WRAP_CODE: {
				InsetWrapParams p;
				data = InsetWrap::params2string(p);
				break;
			}
			default:
				lyxerr << "Inset type '" << name << 
					"' not recognized in LFUN_DIALOG_SHOW_NEW_INSET" <<  endl;
				insetCodeOK = false;
				break;
			} // end switch(code)
			if (insetCodeOK)
				dispatch(FuncRequest(LFUN_DIALOG_SHOW, name + " " + data));
			break;
		}

		case LFUN_CITATION_INSERT: {
			LASSERT(lyx_view_, /**/);
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
			LASSERT(lyx_view_, /**/);
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

		case LFUN_LAYOUT_MODULES_CLEAR: {
			LASSERT(lyx_view_ && lyx_view_->documentBufferView(), /**/);
			DocumentClass const * const oldClass = buffer->params().documentClassPtr();
			lyx_view_->documentBufferView()->cursor().recordUndoFullDocument();
			buffer->params().clearLayoutModules();
			buffer->params().makeDocumentClass();
			updateLayout(oldClass, buffer);
			updateFlags = Update::Force | Update::FitCursor;
			break;
		}
		
		case LFUN_LAYOUT_MODULE_ADD: {
			LASSERT(lyx_view_ && lyx_view_->documentBufferView(), /**/);
			BufferParams const & params = buffer->params();
			if (!params.moduleCanBeAdded(argument)) {
				LYXERR0("Module `" << argument << 
						"' cannot be added due to failed requirements or "
						"conflicts with installed modules.");
				break;
			}
			DocumentClass const * const oldClass = params.documentClassPtr();
			lyx_view_->documentBufferView()->cursor().recordUndoFullDocument();
			buffer->params().addLayoutModule(argument);
			buffer->params().makeDocumentClass();
			updateLayout(oldClass, buffer);
			updateFlags = Update::Force | Update::FitCursor;
			break;
		}

		case LFUN_TEXTCLASS_APPLY: {
			LASSERT(lyx_view_ && lyx_view_->documentBufferView(), /**/);

			if (!loadLayoutFile(argument, buffer->temppath()) &&
				!loadLayoutFile(argument, buffer->filePath()))
				break;

			LayoutFile const * old_layout = buffer->params().baseClass();
			LayoutFile const * new_layout = &(LayoutFileList::get()[argument]);

			if (old_layout == new_layout)
				// nothing to do
				break;

			//Save the old, possibly modular, layout for use in conversion.
			DocumentClass const * const oldDocClass =
				buffer->params().documentClassPtr();
			lyx_view_->documentBufferView()->cursor().recordUndoFullDocument();
			buffer->params().setBaseClass(argument);
			buffer->params().makeDocumentClass();
			updateLayout(oldDocClass, buffer);
			updateFlags = Update::Force | Update::FitCursor;
			break;
		}
		
		case LFUN_LAYOUT_RELOAD: {
			LASSERT(lyx_view_, /**/);
			DocumentClass const * const oldClass = buffer->params().documentClassPtr();
			LayoutFileIndex bc = buffer->params().baseClassID();
			LayoutFileList::get().reset(bc);
			buffer->params().setBaseClass(bc);
			buffer->params().makeDocumentClass();
			updateLayout(oldClass, buffer);
			updateFlags = Update::Force | Update::FitCursor;
			break;
		}

		case LFUN_TEXTCLASS_LOAD:
			loadLayoutFile(argument, buffer->temppath()) ||
			loadLayoutFile(argument, buffer->filePath());
			break;

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

			// Everything below is only for active lyx_view_
			if (lyx_view_ == 0)
				break;

			// Start an undo group. This may be needed for
			// some stuff like inset-apply on labels.
			if (theBufferList().isLoaded(buffer))
				buffer->undo().beginUndoGroup();
				
			// Let the current LyXView dispatch its own actions.
			if (lyx_view_->dispatch(cmd)) {
				if (lyx_view_->currentBufferView()) {
					updateFlags = lyx_view_->currentBufferView()->cursor().result().update();
					if (theBufferList().isLoaded(buffer))
						buffer->undo().endUndoGroup();
				}
				break;
			}

			LASSERT(lyx_view_->currentBufferView(), /**/);

			// Let the current BufferView dispatch its own actions.
			if (lyx_view_->currentBufferView()->dispatch(cmd)) {
				// The BufferView took care of its own updates if needed.
				updateFlags = Update::None;
				if (theBufferList().isLoaded(buffer))
					buffer->undo().endUndoGroup();
				break;
			}

			// OK, so try the Buffer itself
			DispatchResult dr;
			BufferView * bv = lyx_view_->currentBufferView();
			bv->buffer().dispatch(cmd, dr);
			if (dr.dispatched()) {
				updateFlags = dr.update();
				break;
			}
			// OK, so try with the document Buffer.
			BufferView * doc_bv = lyx_view_->documentBufferView();
			if (doc_bv) {
				buffer = &(doc_bv->buffer());
				buffer->dispatch(cmd, dr);
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
					lyx_view_->updateCompletion(bv->cursor(), true, true);
				else if (cmd.action == LFUN_CHAR_DELETE_BACKWARD)
					lyx_view_->updateCompletion(bv->cursor(), false, true);
				else
					lyx_view_->updateCompletion(bv->cursor(), false, false);
			}

			updateFlags = bv->cursor().result().update();
		}

		// if we executed a mutating lfun, mark the buffer as dirty
		if (theBufferList().isLoaded(buffer) && flag.enabled()
		    && !lyxaction.funcHasFlag(action, LyXAction::NoBuffer)
		    && !lyxaction.funcHasFlag(action, LyXAction::ReadOnly))
			buffer->markDirty();			

		if (lyx_view_ && lyx_view_->currentBufferView()) {
			// BufferView::update() updates the ViewMetricsInfo and
			// also initializes the position cache for all insets in
			// (at least partially) visible top-level paragraphs.
			// We will redraw the screen only if needed.
			lyx_view_->currentBufferView()->processUpdateFlags(updateFlags);

			// Do we have a selection?
			theSelection().haveSelection(
				lyx_view_->currentBufferView()->cursor().selection());
			
			// update gui
			lyx_view_->restartCursor();
		}
	}
	if (lyx_view_) {
		// Some messages may already be translated, so we cannot use _()
		sendDispatchMessage(translateIfPossible(getMessage()), cmd);
	}
}


void LyXFunc::sendDispatchMessage(docstring const & msg, FuncRequest const & cmd)
{
	const bool verbose = (cmd.origin == FuncRequest::MENU
			      || cmd.origin == FuncRequest::TOOLBAR
			      || cmd.origin == FuncRequest::COMMANDBUFFER);

	if (cmd.action == LFUN_SELF_INSERT || !verbose) {
		LYXERR(Debug::ACTION, "dispatch msg is " << to_utf8(msg));
		if (!msg.empty())
			lyx_view_->message(msg);
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
		lyx_view_->message(dispatch_msg);
}


// Each "lyx_view_" should have it's own message method. lyxview and
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

	LASSERT(lyx_view_, /**/);
	if (!lyx_view_->currentBufferView())
		return _("Welcome to LyX!");

	return lyx_view_->currentBufferView()->cursor().currentState();
}


bool LyXFunc::wasMetaKey() const
{
	return (meta_fake_bit != NoModifier);
}


void LyXFunc::updateLayout(DocumentClass const * const oldlayout, Buffer * buf)
{
	lyx_view_->message(_("Converting document to new document class..."));
	
	StableDocIterator backcur(lyx_view_->currentBufferView()->cursor());
	ErrorList & el = buf->errorList("Class Switch");
	cap::switchBetweenClasses(
			oldlayout, buf->params().documentClassPtr(),
			static_cast<InsetText &>(buf->inset()), el);

	lyx_view_->currentBufferView()->setCursor(backcur.asDocIterator(buf));

	buf->errors("Class Switch");
	buf->updateLabels();
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
