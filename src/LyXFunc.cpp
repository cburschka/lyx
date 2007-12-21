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
#include <vector>

#include "LyXFunc.h"

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
#include "support/debug.h"
#include "DispatchResult.h"
#include "Encoding.h"
#include "ErrorList.h"
#include "Format.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "support/gettext.h"
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
#include "TextClassList.h"
#include "ToolbarBackend.h"

#include "insets/InsetBox.h"
#include "insets/InsetBranch.h"
#include "insets/InsetCommand.h"
#include "insets/InsetERT.h"
#include "insets/InsetExternal.h"
#include "insets/InsetFloat.h"
#include "insets/InsetListings.h"
#include "insets/InsetGraphics.h"
#include "insets/InsetInclude.h"
#include "insets/InsetNote.h"
#include "insets/InsetTabular.h"
#include "insets/InsetVSpace.h"
#include "insets/InsetWrap.h"

#include "frontends/alert.h"
#include "frontends/Application.h"
#include "frontends/FileDialog.h"
#include "frontends/FontLoader.h"
#include "frontends/KeySymbol.h"
#include "frontends/LyXView.h"
#include "frontends/Selection.h"

#include "support/environment.h"
#include "support/FileFilterList.h"
#include "support/FileName.h"
#include "support/filetools.h"
#include "support/lstrings.h"
#include "support/Path.h"
#include "support/Package.h"
#include "support/Systemcall.h"
#include "support/convert.h"
#include "support/os.h"

#include <sstream>

using namespace std;
using namespace lyx::support;

namespace lyx {

using frontend::LyXView;

namespace Alert = frontend::Alert;

extern bool quitting;
extern bool use_gui;

namespace {


bool import(LyXView * lv, FileName const & filename,
		      string const & format, ErrorList & errorList)
{
	docstring const displaypath = makeDisplayPath(filename.absFilename());
	lv->message(bformat(_("Importing %1$s..."), displaypath));

	FileName const lyxfile(changeExtension(filename.absFilename(), ".lyx"));

	string loader_format;
	vector<string> loaders = theConverters().loaders();
	if (find(loaders.begin(), loaders.end(), format) == loaders.end()) {
		for (vector<string>::const_iterator it = loaders.begin();
		     it != loaders.end(); ++it) {
			if (theConverters().isReachable(format, *it)) {
				string const tofile =
					changeExtension(filename.absFilename(),
						formats.extension(*it));
				if (!theConverters().convert(0, filename, FileName(tofile),
							filename, format, *it, errorList))
					return false;
				loader_format = *it;
				break;
			}
		}
		if (loader_format.empty()) {
			frontend::Alert::error(_("Couldn't import file"),
				     bformat(_("No information for importing the format %1$s."),
					 formats.prettyName(format)));
			return false;
		}
	} else {
		loader_format = format;
	}


	if (loader_format == "lyx") {
		Buffer * buf = theLyXFunc().loadAndViewFile(lyxfile);
		if (!buf) {
			// we are done
			lv->message(_("file not imported!"));
			return false;
		}
		updateLabels(*buf);
		lv->setBuffer(buf);
		buf->errors("Parse");
	} else {
		Buffer * const b = newFile(lyxfile.absFilename(), string(), true);
		if (b)
			lv->setBuffer(b);
		else
			return false;
		bool as_paragraphs = loader_format == "textparagraph";
		string filename2 = (loader_format == format) ? filename.absFilename()
			: changeExtension(filename.absFilename(),
					  formats.extension(loader_format));
		lv->view()->insertPlaintextFile(FileName(filename2), as_paragraphs);
		theLyXFunc().setLyXView(lv);
		lyx::dispatch(FuncRequest(LFUN_MARK_OFF));
	}

	// we are done
	lv->message(_("imported."));
	return true;
}



// This function runs "configure" and then rereads lyx.defaults to
// reconfigure the automatic settings.
void reconfigure(LyXView & lv, string const & option)
{
	// emit message signal.
	lv.message(_("Running configure..."));

	// Run configure in user lyx directory
	PathChanger p(package().user_support());
	string configure_command = package().configure_command();
	configure_command += option;
	Systemcall one;
	int ret = one.startscript(Systemcall::Wait, configure_command);
	p.pop();
	// emit message signal.
	lv.message(_("Reloading configuration..."));
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
		BOOST_ASSERT(cursor.idx() <= cursor.lastidx());
		BOOST_ASSERT(cursor.pit() <= cursor.lastpit());
		BOOST_ASSERT(cursor.pos() <= cursor.lastpos());

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
	if (!quitting && lyx_view_ && lyx_view_->view() && lyx_view_ != lv)
		// save current selection to the selection buffer to allow
		// middle-button paste in another window
		cap::saveSelection(lyx_view_->view()->cursor());
	lyx_view_ = lv;
}


void LyXFunc::handleKeyFunc(kb_action action)
{
	char_type c = encoded_last_key;

	if (keyseq.length())
		c = 0;

	BOOST_ASSERT(lyx_view_ && lyx_view_->view());
	lyx_view_->view()->getIntl().getTransManager().deadkey(
		c, get_accent(action).accent, view()->cursor().innerText(), view()->cursor());
	// Need to clear, in case the minibuffer calls these
	// actions
	keyseq.clear();
	// copied verbatim from do_accent_char
	view()->cursor().resetAnchor();
	view()->processUpdateFlags(Update::FitCursor);
}


void LyXFunc::gotoBookmark(unsigned int idx, bool openFile, bool switchToBuffer)
{
	BOOST_ASSERT(lyx_view_);
	if (!LyX::ref().session().bookmarks().isValid(idx))
		return;
	BookmarksSection::Bookmark const & bm = LyX::ref().session().bookmarks().bookmark(idx);
	BOOST_ASSERT(!bm.filename.empty());
	string const file = bm.filename.absFilename();
	// if the file is not opened, open it.
	if (!theBufferList().exists(file)) {
		if (openFile)
			dispatch(FuncRequest(LFUN_FILE_OPEN, file));
		else
			return;
	}
	// open may fail, so we need to test it again
	if (!theBufferList().exists(file))
		return;

	// if the current buffer is not that one, switch to it.
	if (lyx_view_->buffer()->absFileName() != file) {
		if (!switchToBuffer)
			return;
		dispatch(FuncRequest(LFUN_BUFFER_SWITCH, file));
	}
	// moveToPosition try paragraph id first and then paragraph (pit, pos).
	if (!view()->moveToPosition(bm.bottom_pit, bm.bottom_pos,
		bm.top_id, bm.top_pos))
		return;

	// Cursor jump succeeded!
	Cursor const & cur = view()->cursor();
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
		lyx_view_->restartCursor();
		return;
	}

	//Encoding const * encoding = view()->cursor().getEncoding();
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
	}

	lyx_view_->restartCursor();
}


FuncStatus LyXFunc::getStatus(FuncRequest const & cmd) const
{
	//lyxerr << "LyXFunc::getStatus: cmd: " << cmd << endl;
	FuncStatus flag;

	Buffer * buf = lyx_view_? lyx_view_->buffer() : 0;

	if (cmd.action == LFUN_NOACTION) {
		flag.message(from_utf8(N_("Nothing to do")));
		flag.enabled(false);
		return flag;
	}

	switch (cmd.action) {
	case LFUN_UNKNOWN_ACTION:
#ifndef HAVE_LIBAIKSAURUS
	case LFUN_THESAURUS_ENTRY:
#endif
		flag.unknown(true);
		flag.enabled(false);
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
		flag.enabled(false);
		return flag;
	}

	// I would really like to avoid having this switch and rather try to
	// encode this in the function itself.
	// -- And I'd rather let an inset decide which LFUNs it is willing
	// to handle (Andre')
	bool enable = true;
	switch (cmd.action) {

	// FIXME: these cases should be hidden in GuiApplication::getStatus().
	case LFUN_WINDOW_CLOSE:
		if (theApp())
			return theApp()->getStatus(cmd);
		enable = false;
		break;

	// FIXME: these cases should be hidden in GuiView::getStatus().
	case LFUN_DIALOG_TOGGLE:
	case LFUN_DIALOG_SHOW:
	case LFUN_DIALOG_UPDATE:
	case LFUN_TOOLBAR_TOGGLE:
	case LFUN_INSET_APPLY:
	case LFUN_BUFFER_WRITE:
	case LFUN_BUFFER_WRITE_AS:
		if (lyx_view_)
			return lyx_view_->getStatus(cmd);
		enable = false;
		break;

	case LFUN_BUFFER_TOGGLE_READ_ONLY:
		flag.setOnOff(buf->isReadonly());
		break;

	case LFUN_BUFFER_SWITCH:
		// toggle on the current buffer, but do not toggle off
		// the other ones (is that a good idea?)
		if (buf && to_utf8(cmd.argument()) == buf->absFileName())
			flag.setOnOff(true);
		break;

	case LFUN_BUFFER_EXPORT:
		enable = cmd.argument() == "custom"
			|| buf->isExportable(to_utf8(cmd.argument()));
		break;

	case LFUN_BUFFER_CHKTEX:
		enable = buf->isLatex() && !lyxrc.chktex_command.empty();
		break;

	case LFUN_BUILD_PROGRAM:
		enable = buf->isExportable("program");
		break;

	case LFUN_VC_REGISTER:
		enable = !buf->lyxvc().inUse();
		break;
	case LFUN_VC_CHECK_IN:
		enable = buf->lyxvc().inUse() && !buf->isReadonly();
		break;
	case LFUN_VC_CHECK_OUT:
		enable = buf->lyxvc().inUse() && buf->isReadonly();
		break;
	case LFUN_VC_REVERT:
	case LFUN_VC_UNDO_LAST:
		enable = buf->lyxvc().inUse();
		break;
	case LFUN_BUFFER_RELOAD:
		enable = !buf->isUnnamed() && buf->fileName().exists()
			&& (!buf->isClean() || buf->isExternallyModified(Buffer::timestamp_method));
		break;

	case LFUN_CITATION_INSERT: {
		FuncRequest fr(LFUN_INSET_INSERT, "citation");
		enable = getStatus(fr).enabled();
		break;
	}
	
	// This could be used for the no-GUI version. The GUI version is handled in
	// LyXView::getStatus().
	/*
	case LFUN_BUFFER_WRITE:
	case LFUN_BUFFER_WRITE_AS: {
		Buffer * b = theBufferList().getBuffer(cmd.getArg(0));
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
		enable = LyX::ref().session().bookmarks().isValid(num);
		break;
	}

	case LFUN_BOOKMARK_CLEAR:
		enable = LyX::ref().session().bookmarks().size() > 0;
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

	case LFUN_CALL: {
		FuncRequest func;
		string name = to_utf8(cmd.argument());
		if (LyX::ref().topLevelCmdDef().lock(name, func)) {
			func.origin = cmd.origin;
			flag = getStatus(func);
			LyX::ref().topLevelCmdDef().release(name);
		} else {
			// catch recursion or unknown command definiton
			// all operations until the recursion or unknown command 
			// definiton occures are performed, so set the state to enabled
			enable = true;
		}
		break;
	}

	case LFUN_BUFFER_NEW:
	case LFUN_BUFFER_NEW_TEMPLATE:
	case LFUN_WORD_FIND_FORWARD:
	case LFUN_WORD_FIND_BACKWARD:
	case LFUN_COMMAND_PREFIX:
	case LFUN_COMMAND_EXECUTE:
	case LFUN_CANCEL:
	case LFUN_META_PREFIX:
	case LFUN_BUFFER_CLOSE:
	case LFUN_BUFFER_UPDATE:
	case LFUN_BUFFER_VIEW:
	case LFUN_MASTER_BUFFER_UPDATE:
	case LFUN_MASTER_BUFFER_VIEW:
	case LFUN_BUFFER_IMPORT:
	case LFUN_BUFFER_AUTO_SAVE:
	case LFUN_RECONFIGURE:
	case LFUN_HELP_OPEN:
	case LFUN_FILE_NEW:
	case LFUN_FILE_OPEN:
	case LFUN_DROP_LAYOUTS_CHOICE:
	case LFUN_MENU_OPEN:
	case LFUN_SERVER_GET_NAME:
	case LFUN_SERVER_NOTIFY:
	case LFUN_SERVER_GOTO_FILE_ROW:
	case LFUN_DIALOG_HIDE:
	case LFUN_DIALOG_DISCONNECT_INSET:
	case LFUN_BUFFER_CHILD_OPEN:
	case LFUN_TOGGLE_CURSOR_FOLLOWS_SCROLLBAR:
	case LFUN_KEYMAP_OFF:
	case LFUN_KEYMAP_PRIMARY:
	case LFUN_KEYMAP_SECONDARY:
	case LFUN_KEYMAP_TOGGLE:
	case LFUN_REPEAT:
	case LFUN_BUFFER_EXPORT_CUSTOM:
	case LFUN_BUFFER_PRINT:
	case LFUN_PREFERENCES_SAVE:
	case LFUN_SCREEN_FONT_UPDATE:
	case LFUN_SET_COLOR:
	case LFUN_MESSAGE:
	case LFUN_EXTERNAL_EDIT:
	case LFUN_GRAPHICS_EDIT:
	case LFUN_ALL_INSETS_TOGGLE:
	case LFUN_BUFFER_LANGUAGE:
	case LFUN_TEXTCLASS_APPLY:
	case LFUN_TEXTCLASS_LOAD:
	case LFUN_BUFFER_SAVE_AS_DEFAULT:
	case LFUN_BUFFER_PARAMS_APPLY:
	case LFUN_LAYOUT_MODULES_CLEAR:
	case LFUN_LAYOUT_MODULE_ADD:
	case LFUN_LAYOUT_RELOAD:
	case LFUN_LYXRC_APPLY:
	case LFUN_BUFFER_NEXT:
	case LFUN_BUFFER_PREVIOUS:
	case LFUN_WINDOW_NEW:
	case LFUN_LYX_QUIT:
		// these are handled in our dispatch()
		break;

	default:
		if (!view()) {
			enable = false;
			break;
		}
		if (!getLocalStatus(view()->cursor(), cmd, flag))
			flag = view()->getStatus(cmd);
	}

	if (!enable)
		flag.enabled(false);

	// Can we use a readonly buffer?
	if (buf && buf->isReadonly()
	    && !lyxaction.funcHasFlag(cmd.action, LyXAction::ReadOnly)
	    && !lyxaction.funcHasFlag(cmd.action, LyXAction::NoBuffer)) {
		flag.message(from_utf8(N_("Document is read-only")));
		flag.enabled(false);
	}

	// Are we in a DELETED change-tracking region?
	if (buf && view() 
		&& lookupChangeType(view()->cursor(), true) == Change::DELETED
	    && !lyxaction.funcHasFlag(cmd.action, LyXAction::ReadOnly)
	    && !lyxaction.funcHasFlag(cmd.action, LyXAction::NoBuffer)) {
		flag.message(from_utf8(N_("This portion of the document is deleted.")));
		flag.enabled(false);
	}

	// the default error message if we disable the command
	if (!flag.enabled() && flag.message().empty())
		flag.message(from_utf8(N_("Command disabled")));

	return flag;
}


bool LyXFunc::ensureBufferClean(BufferView * bv)
{
	Buffer & buf = bv->buffer();
	if (buf.isClean())
		return true;

	docstring const file = buf.fileName().displayName(30);
	docstring text = bformat(_("The document %1$s has unsaved "
					     "changes.\n\nDo you want to save "
					     "the document?"), file);
	int const ret = Alert::prompt(_("Save changed document?"),
				      text, 0, 1, _("&Save"),
				      _("&Cancel"));

	if (ret == 0)
		dispatch(FuncRequest(LFUN_BUFFER_WRITE));

	return buf.isClean();
}


namespace {

void showPrintError(string const & name)
{
	docstring str = bformat(_("Could not print the document %1$s.\n"
					    "Check that your printer is set up correctly."),
			     makeDisplayPath(name, 50));
	Alert::error(_("Print document failed"), str);
}


void loadTextClass(string const & name)
{
	pair<bool, textclass_type> const tc_pair =
		textclasslist.numberOfClass(name);

	if (!tc_pair.first) {
		lyxerr << "Document class \"" << name
		       << "\" does not exist."
		       << endl;
		return;
	}

	textclass_type const tc = tc_pair.second;

	if (!textclasslist[tc].load()) {
		docstring s = bformat(_("The document class %1$s."
				   "could not be loaded."),
				   from_utf8(textclasslist[tc].name()));
		Alert::error(_("Could not load class"), s);
	}
}


void actOnUpdatedPrefs(LyXRC const & lyxrc_orig, LyXRC const & lyxrc_new);

} //namespace anon


void LyXFunc::dispatch(FuncRequest const & cmd)
{
	string const argument = to_utf8(cmd.argument());
	kb_action const action = cmd.action;

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
	} else {
		switch (action) {

		case LFUN_WORD_FIND_FORWARD:
		case LFUN_WORD_FIND_BACKWARD: {
			BOOST_ASSERT(lyx_view_ && lyx_view_->view());
			static docstring last_search;
			docstring searched_string;

			if (!cmd.argument().empty()) {
				last_search = cmd.argument();
				searched_string = cmd.argument();
			} else {
				searched_string = last_search;
			}

			if (searched_string.empty())
				break;

			bool const fw = action == LFUN_WORD_FIND_FORWARD;
			docstring const data =
				find2string(searched_string, true, false, fw);
			find(view(), FuncRequest(LFUN_WORD_FIND, data));
			break;
		}

		case LFUN_COMMAND_PREFIX:
			BOOST_ASSERT(lyx_view_);
			lyx_view_->message(keyseq.printOptions(true));
			break;

		case LFUN_CANCEL:
			BOOST_ASSERT(lyx_view_ && lyx_view_->view());
			keyseq.reset();
			meta_fake_bit = NoModifier;
			if (lyx_view_->buffer())
				// cancel any selection
				dispatch(FuncRequest(LFUN_MARK_OFF));
			setMessage(from_ascii(N_("Cancel")));
			break;

		case LFUN_META_PREFIX:
			meta_fake_bit = AltModifier;
			setMessage(keyseq.print(KeySequence::ForGui));
			break;

		case LFUN_BUFFER_TOGGLE_READ_ONLY: {
			BOOST_ASSERT(lyx_view_ && lyx_view_->view() && lyx_view_->buffer());
			Buffer * buf = lyx_view_->buffer();
			if (buf->lyxvc().inUse())
				buf->lyxvc().toggleReadOnly();
			else
				buf->setReadonly(!lyx_view_->buffer()->isReadonly());
			break;
		}

		// --- Menus -----------------------------------------------
		case LFUN_BUFFER_NEW:
			lyx_view_->newDocument(argument, false);
			updateFlags = Update::None;
			break;

		case LFUN_BUFFER_NEW_TEMPLATE:
			lyx_view_->newDocument(argument, true);
			updateFlags = Update::None;
			break;

		case LFUN_BUFFER_CLOSE:
			closeBuffer();
			updateFlags = Update::None;
			break;

		case LFUN_BUFFER_RELOAD: {
			BOOST_ASSERT(lyx_view_ && lyx_view_->buffer());
			docstring const file = makeDisplayPath(lyx_view_->buffer()->absFileName(), 20);
			docstring text = bformat(_("Any changes will be lost. Are you sure "
							     "you want to revert to the saved version of the document %1$s?"), file);
			int const ret = Alert::prompt(_("Revert to saved document?"),
				text, 1, 1, _("&Revert"), _("&Cancel"));

			if (ret == 0)
				reloadBuffer();
			break;
		}

		case LFUN_BUFFER_UPDATE:
			BOOST_ASSERT(lyx_view_ && lyx_view_->buffer());
			lyx_view_->buffer()->doExport(argument, true);
			break;

		case LFUN_BUFFER_VIEW:
			BOOST_ASSERT(lyx_view_ && lyx_view_->buffer());
			lyx_view_->buffer()->preview(argument);
			break;

		case LFUN_MASTER_BUFFER_UPDATE:
			BOOST_ASSERT(lyx_view_ && lyx_view_->buffer() && lyx_view_->buffer()->masterBuffer());
			lyx_view_->buffer()->masterBuffer()->doExport(argument, true);
			break;

		case LFUN_MASTER_BUFFER_VIEW:
			BOOST_ASSERT(lyx_view_ && lyx_view_->buffer() && lyx_view_->buffer()->masterBuffer());
			lyx_view_->buffer()->masterBuffer()->preview(argument);
			break;

		case LFUN_BUILD_PROGRAM:
			BOOST_ASSERT(lyx_view_ && lyx_view_->buffer());
			lyx_view_->buffer()->doExport("program", true);
			break;

		case LFUN_BUFFER_CHKTEX:
			BOOST_ASSERT(lyx_view_ && lyx_view_->buffer());
			lyx_view_->buffer()->runChktex();
			break;

		case LFUN_BUFFER_EXPORT:
			BOOST_ASSERT(lyx_view_ && lyx_view_->buffer());
			if (argument == "custom")
				dispatch(FuncRequest(LFUN_DIALOG_SHOW, "sendto"));
			else
				lyx_view_->buffer()->doExport(argument, false);
			break;

		case LFUN_BUFFER_EXPORT_CUSTOM: {
			BOOST_ASSERT(lyx_view_ && lyx_view_->buffer());
			string format_name;
			string command = split(argument, format_name, ' ');
			Format const * format = formats.getFormat(format_name);
			if (!format) {
				lyxerr << "Format \"" << format_name
				       << "\" not recognized!"
				       << endl;
				break;
			}

			Buffer * buffer = lyx_view_->buffer();

			// The name of the file created by the conversion process
			string filename;

			// Output to filename
			if (format->name() == "lyx") {
				string const latexname = buffer->latexName(false);
				filename = changeExtension(latexname,
							   format->extension());
				filename = addName(buffer->temppath(), filename);

				if (!buffer->writeFile(FileName(filename)))
					break;

			} else {
				buffer->doExport(format_name, true, filename);
			}

			// Substitute $$FName for filename
			if (!contains(command, "$$FName"))
				command = "( " + command + " ) < $$FName";
			command = subst(command, "$$FName", filename);

			// Execute the command in the background
			Systemcall call;
			call.startscript(Systemcall::DontWait, command);
			break;
		}

		case LFUN_BUFFER_PRINT: {
			BOOST_ASSERT(lyx_view_ && lyx_view_->buffer());
			// FIXME: cmd.getArg() might fail if one of the arguments
			// contains double quotes
			string target = cmd.getArg(0);
			string target_name = cmd.getArg(1);
			string command = cmd.getArg(2);

			if (target.empty()
			    || target_name.empty()
			    || command.empty()) {
				lyxerr << "Unable to parse \""
				       << argument << '"' << endl;
				break;
			}
			if (target != "printer" && target != "file") {
				lyxerr << "Unrecognized target \""
				       << target << '"' << endl;
				break;
			}

			Buffer * buffer = lyx_view_->buffer();

			if (!buffer->doExport("dvi", true)) {
				showPrintError(buffer->absFileName());
				break;
			}

			// Push directory path.
			string const path = buffer->temppath();
			// Prevent the compiler from optimizing away p
			FileName pp(path);
			PathChanger p(pp);

			// there are three cases here:
			// 1. we print to a file
			// 2. we print directly to a printer
			// 3. we print using a spool command (print to file first)
			Systemcall one;
			int res = 0;
			string const dviname =
				changeExtension(buffer->latexName(true), "dvi");

			if (target == "printer") {
				if (!lyxrc.print_spool_command.empty()) {
					// case 3: print using a spool
					string const psname =
						changeExtension(dviname,".ps");
					command += ' ' + lyxrc.print_to_file
						+ quoteName(psname)
						+ ' '
						+ quoteName(dviname);

					string command2 =
						lyxrc.print_spool_command + ' ';
					if (target_name != "default") {
						command2 += lyxrc.print_spool_printerprefix
							+ target_name
							+ ' ';
					}
					command2 += quoteName(psname);
					// First run dvips.
					// If successful, then spool command
					res = one.startscript(
						Systemcall::Wait,
						command);

					if (res == 0)
						res = one.startscript(
							Systemcall::DontWait,
							command2);
				} else {
					// case 2: print directly to a printer
					if (target_name != "default")
						command += ' ' + lyxrc.print_to_printer + target_name + ' ';
					res = one.startscript(
						Systemcall::DontWait,
						command + quoteName(dviname));
				}

			} else {
				// case 1: print to a file
				FileName const filename(makeAbsPath(target_name,
							lyx_view_->buffer()->filePath()));
				FileName const dvifile(makeAbsPath(dviname, path));
				if (filename.exists()) {
					docstring text = bformat(
						_("The file %1$s already exists.\n\n"
						  "Do you want to overwrite that file?"),
						makeDisplayPath(filename.absFilename()));
					if (Alert::prompt(_("Overwrite file?"),
					    text, 0, 1, _("&Overwrite"), _("&Cancel")) != 0)
						break;
				}
				command += ' ' + lyxrc.print_to_file
					+ quoteName(filename.toFilesystemEncoding())
					+ ' '
					+ quoteName(dvifile.toFilesystemEncoding());
				res = one.startscript(Systemcall::DontWait,
						      command);
			}

			if (res != 0)
				showPrintError(buffer->absFileName());
			break;
		}

		case LFUN_BUFFER_IMPORT:
			doImport(argument);
			break;

		case LFUN_BUFFER_AUTO_SAVE:
			lyx_view_->buffer()->autoSave();
			break;

		case LFUN_RECONFIGURE:
			BOOST_ASSERT(lyx_view_);
			// argument is any additional parameter to the configure.py command
			reconfigure(*lyx_view_, argument);
			break;

		case LFUN_HELP_OPEN: {
			BOOST_ASSERT(lyx_view_);
			string const arg = argument;
			if (arg.empty()) {
				setErrorMessage(from_ascii(N_("Missing argument")));
				break;
			}
			FileName const fname = i18nLibFileSearch("doc", arg, "lyx");
			if (fname.empty()) {
				lyxerr << "LyX: unable to find documentation file `"
							 << arg << "'. Bad installation?" << endl;
				break;
			}
			lyx_view_->message(bformat(_("Opening help file %1$s..."),
				makeDisplayPath(fname.absFilename())));
			Buffer * buf = loadAndViewFile(fname, false);
			if (buf) {
				updateLabels(*buf);
				lyx_view_->setBuffer(buf);
				buf->errors("Parse");
			}
			updateFlags = Update::None;
			break;
		}

		// --- version control -------------------------------
		case LFUN_VC_REGISTER:
			BOOST_ASSERT(lyx_view_ && lyx_view_->buffer());
			if (!ensureBufferClean(view()))
				break;
			if (!lyx_view_->buffer()->lyxvc().inUse()) {
				lyx_view_->buffer()->lyxvc().registrer();
				reloadBuffer();
			}
			updateFlags = Update::Force;
			break;

		case LFUN_VC_CHECK_IN:
			BOOST_ASSERT(lyx_view_ && lyx_view_->buffer());
			if (!ensureBufferClean(view()))
				break;
			if (lyx_view_->buffer()->lyxvc().inUse()
					&& !lyx_view_->buffer()->isReadonly()) {
				lyx_view_->buffer()->lyxvc().checkIn();
				reloadBuffer();
			}
			break;

		case LFUN_VC_CHECK_OUT:
			BOOST_ASSERT(lyx_view_ && lyx_view_->buffer());
			if (!ensureBufferClean(view()))
				break;
			if (lyx_view_->buffer()->lyxvc().inUse()
					&& lyx_view_->buffer()->isReadonly()) {
				lyx_view_->buffer()->lyxvc().checkOut();
				reloadBuffer();
			}
			break;

		case LFUN_VC_REVERT:
			BOOST_ASSERT(lyx_view_ && lyx_view_->buffer());
			lyx_view_->buffer()->lyxvc().revert();
			reloadBuffer();
			break;

		case LFUN_VC_UNDO_LAST:
			BOOST_ASSERT(lyx_view_ && lyx_view_->buffer());
			lyx_view_->buffer()->lyxvc().undoLast();
			reloadBuffer();
			break;

		// --- buffers ----------------------------------------

		case LFUN_FILE_NEW: {
			BOOST_ASSERT(lyx_view_);
			string name;
			string tmpname = split(argument, name, ':'); // Split filename
			Buffer * const b = newFile(name, tmpname);
			if (b)
				lyx_view_->setBuffer(b);
			updateFlags = Update::None;
			break;
		}

		case LFUN_FILE_OPEN:
			BOOST_ASSERT(lyx_view_);
			open(argument);
			updateFlags = Update::None;
			break;

		// --- lyxserver commands ----------------------------
		case LFUN_SERVER_GET_NAME:
			BOOST_ASSERT(lyx_view_ && lyx_view_->buffer());
			setMessage(from_utf8(lyx_view_->buffer()->absFileName()));
			LYXERR(Debug::INFO, "FNAME["
				<< lyx_view_->buffer()->absFileName() << ']');
			break;

		case LFUN_SERVER_NOTIFY:
			dispatch_buffer = keyseq.print(KeySequence::Portable);
			theServer().notifyClient(to_utf8(dispatch_buffer));
			break;

		case LFUN_SERVER_GOTO_FILE_ROW: {
			BOOST_ASSERT(lyx_view_);
			string file_name;
			int row;
			istringstream is(argument);
			is >> file_name >> row;
			Buffer * buf = 0;
			bool loaded = false;
			if (prefixIs(file_name, package().temp_dir().absFilename()))
				// Needed by inverse dvi search. If it is a file
				// in tmpdir, call the apropriated function
				buf = theBufferList().getBufferFromTmp(file_name);
			else {
				// Must replace extension of the file to be .lyx
				// and get full path
				FileName const s = fileSearch(string(), changeExtension(file_name, ".lyx"), "lyx");
				// Either change buffer or load the file
				if (theBufferList().exists(s.absFilename()))
					buf = theBufferList().getBuffer(s.absFilename());
				else {
					buf = loadAndViewFile(s);
					loaded = true;
				}
			}

			if (!buf) {
				updateFlags = Update::None;
				break;
			}

			updateLabels(*buf);
			lyx_view_->setBuffer(buf);
			view()->setCursorFromRow(row);
			if (loaded)
				buf->errors("Parse");
			updateFlags = Update::FitCursor;
			break;
		}


		case LFUN_DIALOG_SHOW_NEW_INSET: {
			BOOST_ASSERT(lyx_view_);
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
			case REF_CODE:
			case TOC_CODE:
			case HYPERLINK_CODE: {
				InsetCommandParams p(code);
				data = InsetCommandMailer::params2string(name, p);
				break;
			} 
			case INCLUDE_CODE: {
				// data is the include type: one of "include",
				// "input", "verbatiminput" or "verbatiminput*"
				if (data.empty())
					// default type is requested
					data = "include";
				InsetCommandParams p(INCLUDE_CODE, data);
				data = InsetCommandMailer::params2string("include", p);
				break;
			} 
			case BOX_CODE: {
				// \c data == "Boxed" || "Frameless" etc
				InsetBoxParams p(data);
				data = InsetBoxMailer::params2string(p);
				break;
			} 
			case BRANCH_CODE: {
				InsetBranchParams p;
				data = InsetBranchMailer::params2string(p);
				break;
			} 
			case CITE_CODE: {
				InsetCommandParams p(CITE_CODE);
				data = InsetCommandMailer::params2string(name, p);
				break;
			} 
			case ERT_CODE: {
				data = InsetERTMailer::params2string(InsetCollapsable::Open);
				break;
			} 
			case EXTERNAL_CODE: {
				InsetExternalParams p;
				Buffer const & buffer = *lyx_view_->buffer();
				data = InsetExternalMailer::params2string(p, buffer);
				break;
			} 
			case FLOAT_CODE:  {
				InsetFloatParams p;
				data = InsetFloatMailer::params2string(p);
				break;
			} 
			case LISTINGS_CODE: {
				InsetListingsParams p;
				data = InsetListingsMailer::params2string(p);
				break;
			} 
			case GRAPHICS_CODE: {
				InsetGraphicsParams p;
				Buffer const & buffer = *lyx_view_->buffer();
				data = InsetGraphicsMailer::params2string(p, buffer);
				break;
			} 
			case NOTE_CODE: {
				InsetNoteParams p;
				data = InsetNoteMailer::params2string(p);
				break;
			} 
			case VSPACE_CODE: {
				VSpace space;
				data = InsetVSpaceMailer::params2string(space);
				break;
			} 
			case WRAP_CODE: {
				InsetWrapParams p;
				data = InsetWrapMailer::params2string(p);
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
			BOOST_ASSERT(lyx_view_);
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
				string icstr = InsetCommandMailer::params2string("citation", icp);
				FuncRequest fr(LFUN_INSET_INSERT, icstr);
				dispatch(fr);
			} else
				dispatch(FuncRequest(LFUN_DIALOG_SHOW_NEW_INSET, "citation"));
			break;
		}

		case LFUN_BUFFER_CHILD_OPEN: {
			BOOST_ASSERT(lyx_view_ && lyx_view_->buffer());
			Buffer * parent = lyx_view_->buffer();
			FileName filename = makeAbsPath(argument, parent->filePath());
			view()->saveBookmark(false);
			Buffer * child = 0;
			bool parsed = false;
			if (theBufferList().exists(filename.absFilename())) {
				child = theBufferList().getBuffer(filename.absFilename());
			} else {
				setMessage(bformat(_("Opening child document %1$s..."),
					makeDisplayPath(filename.absFilename())));
				child = loadAndViewFile(filename, true);
				parsed = true;
			}
			if (child) {
				// Set the parent name of the child document.
				// This makes insertion of citations and references in the child work,
				// when the target is in the parent or another child document.
				child->setParent(parent);
				updateLabels(*child->masterBuffer());
				lyx_view_->setBuffer(child);
				if (parsed)
					child->errors("Parse");
			}

			// If a screen update is required (in case where auto_open is false), 
			// setBuffer() would have taken care of it already. Otherwise we shall 
			// reset the update flag because it can cause a circular problem.
			// See bug 3970.
			updateFlags = Update::None;
			break;
		}

		case LFUN_TOGGLE_CURSOR_FOLLOWS_SCROLLBAR:
			BOOST_ASSERT(lyx_view_);
			lyxrc.cursor_follows_scrollbar = !lyxrc.cursor_follows_scrollbar;
			break;

		case LFUN_KEYMAP_OFF:
			BOOST_ASSERT(lyx_view_ && lyx_view_->view());
			lyx_view_->view()->getIntl().keyMapOn(false);
			break;

		case LFUN_KEYMAP_PRIMARY:
			BOOST_ASSERT(lyx_view_ && lyx_view_->view());
			lyx_view_->view()->getIntl().keyMapPrim();
			break;

		case LFUN_KEYMAP_SECONDARY:
			BOOST_ASSERT(lyx_view_ && lyx_view_->view());
			lyx_view_->view()->getIntl().keyMapSec();
			break;

		case LFUN_KEYMAP_TOGGLE:
			BOOST_ASSERT(lyx_view_ && lyx_view_->view());
			lyx_view_->view()->getIntl().toggleKeyMap();
			break;

		case LFUN_REPEAT: {
			// repeat command
			string countstr;
			string rest = split(argument, countstr, ' ');
			istringstream is(countstr);
			int count = 0;
			is >> count;
			lyxerr << "repeat: count: " << count << " cmd: " << rest << endl;
			for (int i = 0; i < count; ++i)
				dispatch(lyxaction.lookupFunc(rest));
			break;
		}

		case LFUN_COMMAND_SEQUENCE: {
			// argument contains ';'-terminated commands
			string arg = argument;
			while (!arg.empty()) {
				string first;
				arg = split(arg, first, ';');
				FuncRequest func(lyxaction.lookupFunc(first));
				func.origin = cmd.origin;
				dispatch(func);
			}
			break;
		}

		case LFUN_CALL: {
			FuncRequest func;
			if (LyX::ref().topLevelCmdDef().lock(argument, func)) {
				func.origin = cmd.origin;
				dispatch(func);
				LyX::ref().topLevelCmdDef().release(argument);
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

		case LFUN_SET_COLOR: {
			string lyx_name;
			string const x11_name = split(argument, lyx_name, ' ');
			if (lyx_name.empty() || x11_name.empty()) {
				setErrorMessage(from_ascii(N_(
						"Syntax: set-color <lyx_name>"
						" <x11_name>")));
				break;
			}

			bool const graphicsbg_changed =
				(lyx_name == lcolor.getLyXName(Color_graphicsbg) &&
				 x11_name != lcolor.getX11Name(Color_graphicsbg));

			if (!lcolor.setColor(lyx_name, x11_name)) {
				setErrorMessage(
						bformat(_("Set-color \"%1$s\" failed "
								       "- color is undefined or "
								       "may not be redefined"),
									   from_utf8(lyx_name)));
				break;
			}

			theApp()->updateColor(lcolor.getFromLyXName(lyx_name));

			if (graphicsbg_changed) {
				// FIXME: The graphics cache no longer has a changeDisplay method.
#if 0
				graphics::GCache::get().changeDisplay(true);
#endif
			}
			break;
		}

		case LFUN_MESSAGE:
			BOOST_ASSERT(lyx_view_);
			lyx_view_->message(from_utf8(argument));
			break;

		case LFUN_EXTERNAL_EDIT: {
			BOOST_ASSERT(lyx_view_);
			FuncRequest fr(action, argument);
			InsetExternal().dispatch(view()->cursor(), fr);
			break;
		}

		case LFUN_GRAPHICS_EDIT: {
			FuncRequest fr(action, argument);
			InsetGraphics().dispatch(view()->cursor(), fr);
			break;
		}

		case LFUN_ALL_INSETS_TOGGLE: {
			BOOST_ASSERT(lyx_view_);
			string action;
			string const name = split(argument, action, ' ');
			InsetCode const inset_code = insetCode(name);

			Cursor & cur = view()->cursor();
			FuncRequest fr(LFUN_INSET_TOGGLE, action);

			Inset & inset = lyx_view_->buffer()->inset();
			InsetIterator it  = inset_iterator_begin(inset);
			InsetIterator const end = inset_iterator_end(inset);
			for (; it != end; ++it) {
				if (!it->asInsetMath()
				    && (inset_code == NO_CODE
				    || inset_code == it->lyxCode())) {
					Cursor tmpcur = cur;
					tmpcur.pushBackward(*it);
					it->dispatch(tmpcur, fr);
				}
			}
			updateFlags = Update::Force | Update::FitCursor;
			break;
		}

		case LFUN_BUFFER_LANGUAGE: {
			BOOST_ASSERT(lyx_view_);
			Buffer & buffer = *lyx_view_->buffer();
			Language const * oldL = buffer.params().language;
			Language const * newL = languages.getLanguage(argument);
			if (!newL || oldL == newL)
				break;

			if (oldL->rightToLeft() == newL->rightToLeft()
			    && !buffer.isMultiLingual())
				buffer.changeLanguage(oldL, newL);
			break;
		}

		case LFUN_BUFFER_SAVE_AS_DEFAULT: {
			string const fname =
				addName(addPath(package().user_support().absFilename(), "templates/"),
					"defaults.lyx");
			Buffer defaults(fname);

			istringstream ss(argument);
			Lexer lex(0,0);
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

		case LFUN_BUFFER_PARAMS_APPLY: {
			BOOST_ASSERT(lyx_view_);
			biblio::CiteEngine const oldEngine =
					lyx_view_->buffer()->params().getEngine();
			
			Buffer * buffer = lyx_view_->buffer();

			TextClassPtr oldClass = buffer->params().getTextClassPtr();

			Cursor & cur = view()->cursor();
			cur.recordUndoFullDocument();
			
			istringstream ss(argument);
			Lexer lex(0,0);
			lex.setStream(ss);
			int const unknown_tokens = buffer->readHeader(lex);

			if (unknown_tokens != 0) {
				lyxerr << "Warning in LFUN_BUFFER_PARAMS_APPLY!\n"
						<< unknown_tokens << " unknown token"
						<< (unknown_tokens == 1 ? "" : "s")
						<< endl;
			}
			
			updateLayout(oldClass, buffer);
			
			biblio::CiteEngine const newEngine =
					lyx_view_->buffer()->params().getEngine();
			
			if (oldEngine != newEngine) {
				FuncRequest fr(LFUN_INSET_REFRESH);
	
				Inset & inset = lyx_view_->buffer()->inset();
				InsetIterator it  = inset_iterator_begin(inset);
				InsetIterator const end = inset_iterator_end(inset);
				for (; it != end; ++it)
					if (it->lyxCode() == CITE_CODE)
						it->dispatch(cur, fr);
			}
			
			updateFlags = Update::Force | Update::FitCursor;
			// We are here most certainaly because of a change in the document
			// It is then better to make sure that all dialogs are in sync
			// with current document settings. LyXView::restartCursor() achieve this.
			lyx_view_->restartCursor();
			break;
		}
		
		case LFUN_LAYOUT_MODULES_CLEAR: {
			BOOST_ASSERT(lyx_view_);
			Buffer * buffer = lyx_view_->buffer();
			TextClassPtr oldClass = buffer->params().getTextClassPtr();
			view()->cursor().recordUndoFullDocument();
			buffer->params().clearLayoutModules();
			updateLayout(oldClass, buffer);
			updateFlags = Update::Force | Update::FitCursor;
			break;
		}
		
		case LFUN_LAYOUT_MODULE_ADD: {
			BOOST_ASSERT(lyx_view_);
			Buffer * buffer = lyx_view_->buffer();
			TextClassPtr oldClass = buffer->params().getTextClassPtr();
			view()->cursor().recordUndoFullDocument();
			buffer->params().addLayoutModule(argument);
			updateLayout(oldClass, buffer);
			updateFlags = Update::Force | Update::FitCursor;
			break;
		}

		case LFUN_TEXTCLASS_APPLY: {
			BOOST_ASSERT(lyx_view_);
			Buffer * buffer = lyx_view_->buffer();

			loadTextClass(argument);

			pair<bool, textclass_type> const tc_pair =
				textclasslist.numberOfClass(argument);

			if (!tc_pair.first)
				break;

			textclass_type const old_class = buffer->params().getBaseClass();
			textclass_type const new_class = tc_pair.second;

			if (old_class == new_class)
				// nothing to do
				break;

			//Save the old, possibly modular, layout for use in conversion.
			TextClassPtr oldClass = buffer->params().getTextClassPtr();
			view()->cursor().recordUndoFullDocument();
			buffer->params().setBaseClass(new_class);
			updateLayout(oldClass, buffer);
			updateFlags = Update::Force | Update::FitCursor;
			break;
		}
		
		case LFUN_LAYOUT_RELOAD: {
			BOOST_ASSERT(lyx_view_);
			Buffer * buffer = lyx_view_->buffer();
			TextClassPtr oldClass = buffer->params().getTextClassPtr();
			textclass_type const tc = buffer->params().getBaseClass();
			textclasslist.reset(tc);
			buffer->params().setBaseClass(tc);
			updateLayout(oldClass, buffer);
			updateFlags = Update::Force | Update::FitCursor;
			break;
		}

		case LFUN_TEXTCLASS_LOAD:
			loadTextClass(argument);
			break;

		case LFUN_LYXRC_APPLY: {
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
			break;

		case LFUN_BOOKMARK_CLEAR:
			LyX::ref().session().bookmarks().clear();
			break;

		default:
			BOOST_ASSERT(theApp());
			// Let the frontend dispatch its own actions.
			if (theApp()->dispatch(cmd))
				// Nothing more to do.
				return;

			// Let the current LyXView dispatch its own actions.
			BOOST_ASSERT(lyx_view_);
			if (lyx_view_->dispatch(cmd)) {
				if (lyx_view_->view())
					updateFlags = lyx_view_->view()->cursor().result().update();
				break;
			}

			BOOST_ASSERT(lyx_view_->view());
			// Let the current BufferView dispatch its own actions.
			if (view()->dispatch(cmd)) {
				// The BufferView took care of its own updates if needed.
				updateFlags = Update::None;
				break;
			}

			// Let the current Cursor dispatch its own actions.
			view()->cursor().getPos(cursorPosBeforeDispatchX_,
						cursorPosBeforeDispatchY_);
			view()->cursor().dispatch(cmd);
			updateFlags = view()->cursor().result().update();
			// Verify that the action has been dispatched.
			BOOST_ASSERT(view()->cursor().result().dispatched());
		}

		if (lyx_view_ && lyx_view_->buffer()) {
			// BufferView::update() updates the ViewMetricsInfo and
			// also initializes the position cache for all insets in
			// (at least partially) visible top-level paragraphs.
			// We will redraw the screen only if needed.
			view()->processUpdateFlags(updateFlags);

			// if we executed a mutating lfun, mark the buffer as dirty
			if (flag.enabled()
			    && !lyxaction.funcHasFlag(action, LyXAction::NoBuffer)
			    && !lyxaction.funcHasFlag(action, LyXAction::ReadOnly))
				lyx_view_->buffer()->markDirty();

			//Do we have a selection?
			theSelection().haveSelection(view()->cursor().selection());
		}
	}
	if (!quitting && lyx_view_) {
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

	docstring const shortcuts = theTopLevelKeymap().printBindings(cmd);

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


Buffer * LyXFunc::loadAndViewFile(FileName const & filename, bool tolastfiles)
{
	lyx_view_->setBusy(true);

	Buffer * newBuffer = checkAndLoadLyXFile(filename);

	if (!newBuffer) {
		lyx_view_->message(_("Document not loaded."));
		lyx_view_->setBusy(false);
		return 0;
	}

	lyx_view_->setBuffer(newBuffer);

	// scroll to the position when the file was last closed
	if (lyxrc.use_lastfilepos) {
		LastFilePosSection::FilePos filepos =
			LyX::ref().session().lastFilePos().load(filename);
		lyx_view_->view()->moveToPosition(filepos.pit, filepos.pos, 0, 0);
	}

	if (tolastfiles)
		LyX::ref().session().lastFiles().add(filename);

	lyx_view_->setBusy(false);
	return newBuffer;
}


void LyXFunc::open(string const & fname)
{
	string initpath = lyxrc.document_path;

	if (lyx_view_->buffer()) {
		string const trypath = lyx_view_->buffer()->filePath();
		// If directory is writeable, use this as default.
		if (FileName(trypath).isDirWritable())
			initpath = trypath;
	}

	string filename;

	if (fname.empty()) {
		FileDialog dlg(_("Select document to open"), LFUN_FILE_OPEN);
		dlg.setButton1(_("Documents|#o#O"), from_utf8(lyxrc.document_path));
		dlg.setButton2(_("Examples|#E#e"),
				from_utf8(addPath(package().system_support().absFilename(), "examples")));

		FileDialog::Result result =
			dlg.open(from_utf8(initpath),
				     FileFilterList(_("LyX Documents (*.lyx)")),
				     docstring());

		if (result.first == FileDialog::Later)
			return;

		filename = to_utf8(result.second);

		// check selected filename
		if (filename.empty()) {
			lyx_view_->message(_("Canceled."));
			return;
		}
	} else
		filename = fname;

	// get absolute path of file and add ".lyx" to the filename if
	// necessary
	FileName const fullname = fileSearch(string(), filename, "lyx");
	if (!fullname.empty())
		filename = fullname.absFilename();

	// if the file doesn't exist, let the user create one
	if (!fullname.exists()) {
		// the user specifically chose this name. Believe him.
		Buffer * const b = newFile(filename, string(), true);
		if (b)
			lyx_view_->setBuffer(b);
		return;
	}

	docstring const disp_fn = makeDisplayPath(filename);
	lyx_view_->message(bformat(_("Opening document %1$s..."), disp_fn));

	docstring str2;
	Buffer * buf = loadAndViewFile(fullname);
	if (buf) {
		updateLabels(*buf);
		lyx_view_->setBuffer(buf);
		buf->errors("Parse");
		str2 = bformat(_("Document %1$s opened."), disp_fn);
	} else {
		str2 = bformat(_("Could not open document %1$s"), disp_fn);
	}
	lyx_view_->message(str2);
}


void LyXFunc::doImport(string const & argument)
{
	string format;
	string filename = split(argument, format, ' ');

	LYXERR(Debug::INFO, "LyXFunc::doImport: " << format
			    << " file: " << filename);

	// need user interaction
	if (filename.empty()) {
		string initpath = lyxrc.document_path;

		if (lyx_view_->buffer()) {
			string const trypath = lyx_view_->buffer()->filePath();
			// If directory is writeable, use this as default.
			if (FileName(trypath).isDirWritable())
				initpath = trypath;
		}

		docstring const text = bformat(_("Select %1$s file to import"),
			formats.prettyName(format));

		FileDialog dlg(text, LFUN_BUFFER_IMPORT);
		dlg.setButton1(_("Documents|#o#O"), from_utf8(lyxrc.document_path));
		dlg.setButton2(_("Examples|#E#e"),
			from_utf8(addPath(package().system_support().absFilename(), "examples")));

		docstring filter = formats.prettyName(format);
		filter += " (*.";
		// FIXME UNICODE
		filter += from_utf8(formats.extension(format));
		filter += ')';

		FileDialog::Result result =
			dlg.open(from_utf8(initpath),
				     FileFilterList(filter),
				     docstring());

		if (result.first == FileDialog::Later)
			return;

		filename = to_utf8(result.second);

		// check selected filename
		if (filename.empty())
			lyx_view_->message(_("Canceled."));
	}

	if (filename.empty())
		return;

	// get absolute path of file
	FileName const fullname(makeAbsPath(filename));

	FileName const lyxfile(changeExtension(fullname.absFilename(), ".lyx"));

	// Check if the document already is open
	Buffer * buf = theBufferList().getBuffer(lyxfile.absFilename());
	if (use_gui && buf) {
		lyx_view_->setBuffer(buf);
		if (!lyx_view_->closeBuffer()) {
			lyx_view_->message(_("Canceled."));
			return;
		}
	}

	// if the file exists already, and we didn't do
	// -i lyx thefile.lyx, warn
	if (lyxfile.exists() && fullname != lyxfile) {
		docstring const file = makeDisplayPath(lyxfile.absFilename(), 30);

		docstring text = bformat(_("The document %1$s already exists.\n\n"
						     "Do you want to overwrite that document?"), file);
		int const ret = Alert::prompt(_("Overwrite document?"),
			text, 0, 1, _("&Overwrite"), _("&Cancel"));

		if (ret == 1) {
			lyx_view_->message(_("Canceled."));
			return;
		}
	}

	ErrorList errorList;
	import(lyx_view_, fullname, format, errorList);
	// FIXME (Abdel 12/08/06): Is there a need to display the error list here?
}


void LyXFunc::closeBuffer()
{
	// goto bookmark to update bookmark pit.
	for (size_t i = 0; i < LyX::ref().session().bookmarks().size(); ++i)
		gotoBookmark(i+1, false, false);
	
	lyx_view_->closeBuffer();
}


void LyXFunc::reloadBuffer()
{
	FileName filename = lyx_view_->buffer()->fileName();
	// The user has already confirmed that the changes, if any, should
	// be discarded. So we just release the Buffer and don't call closeBuffer();
	theBufferList().release(lyx_view_->buffer());
	Buffer * buf = loadAndViewFile(filename);
	docstring const disp_fn = makeDisplayPath(filename.absFilename());
	docstring str;
	if (buf) {
		updateLabels(*buf);
		lyx_view_->setBuffer(buf);
		buf->errors("Parse");
		str = bformat(_("Document %1$s reloaded."), disp_fn);
	} else {
		str = bformat(_("Could not reload document %1$s"), disp_fn);
	}
	lyx_view_->message(str);
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


docstring const LyXFunc::viewStatusMessage()
{
	// When meta-fake key is pressed, show the key sequence so far + "M-".
	if (wasMetaKey())
		return keyseq.print(KeySequence::ForGui) + "M-";

	// Else, when a non-complete key sequence is pressed,
	// show the available options.
	if (keyseq.length() > 0 && !keyseq.deleted())
		return keyseq.printOptions(true);

	BOOST_ASSERT(lyx_view_);
	if (!lyx_view_->buffer())
		return _("Welcome to LyX!");

	return view()->cursor().currentState();
}


BufferView * LyXFunc::view() const
{
	BOOST_ASSERT(lyx_view_);
	return lyx_view_->view();
}


bool LyXFunc::wasMetaKey() const
{
	return (meta_fake_bit != NoModifier);
}


void LyXFunc::updateLayout(TextClassPtr const & oldlayout,
                           Buffer * buffer)
{
	lyx_view_->message(_("Converting document to new document class..."));
	
	StableDocIterator backcur(view()->cursor());
	ErrorList & el = buffer->errorList("Class Switch");
	cap::switchBetweenClasses(
			oldlayout, buffer->params().getTextClassPtr(),
			static_cast<InsetText &>(buffer->inset()), el);

	view()->setCursor(backcur.asDocIterator(&(buffer->inset())));

	buffer->errors("Class Switch");
	updateLabels(*buffer);
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
	case LyXRC::RC_PLAINTEXT_ROFF_COMMAND:
	case LyXRC::RC_PLAINTEXT_LINELEN:
	case LyXRC::RC_AUTOREGIONDELETE:
	case LyXRC::RC_AUTORESET_OPTIONS:
	case LyXRC::RC_AUTOSAVE:
	case LyXRC::RC_AUTO_NUMBER:
	case LyXRC::RC_BACKUPDIR_PATH:
	case LyXRC::RC_BIBTEX_COMMAND:
	case LyXRC::RC_BINDFILE:
	case LyXRC::RC_CHECKLASTFILES:
	case LyXRC::RC_USELASTFILEPOS:
	case LyXRC::RC_LOADSESSION:
	case LyXRC::RC_CHKTEX_COMMAND:
	case LyXRC::RC_CONVERTER:
	case LyXRC::RC_CONVERTER_CACHE_MAXAGE:
	case LyXRC::RC_COPIER:
	case LyXRC::RC_CURSOR_FOLLOWS_SCROLLBAR:
	case LyXRC::RC_CUSTOM_EXPORT_COMMAND:
	case LyXRC::RC_CUSTOM_EXPORT_FORMAT:
	case LyXRC::RC_DATE_INSERT_FORMAT:
	case LyXRC::RC_DEFAULT_LANGUAGE:
	case LyXRC::RC_DEFAULT_PAPERSIZE:
	case LyXRC::RC_DEFFILE:
	case LyXRC::RC_DIALOGS_ICONIFY_WITH_MAIN:
	case LyXRC::RC_DISPLAY_GRAPHICS:
	case LyXRC::RC_DOCUMENTPATH:
		if (lyxrc_orig.document_path != lyxrc_new.document_path) {
			FileName path(lyxrc_new.document_path);
			if (path.exists() && path.isDirectory())
				package().document_dir() = FileName(lyxrc.document_path);
		}
	case LyXRC::RC_ESC_CHARS:
	case LyXRC::RC_EXAMPLEPATH:
	case LyXRC::RC_FONT_ENCODING:
	case LyXRC::RC_FORMAT:
	case LyXRC::RC_INDEX_COMMAND:
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
	case LyXRC::RC_MAKE_BACKUP:
	case LyXRC::RC_MARK_FOREIGN_LANGUAGE:
	case LyXRC::RC_NUMLASTFILES:
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
	case LyXRC::RC_SPELL_COMMAND:
	case LyXRC::RC_TEMPDIRPATH:
	case LyXRC::RC_TEMPLATEPATH:
	case LyXRC::RC_TEX_ALLOWS_SPACES:
	case LyXRC::RC_TEX_EXPECTS_WINDOWS_PATHS:
		if (lyxrc_orig.windows_style_tex_paths != lyxrc_new.windows_style_tex_paths) {
			os::windows_style_tex_paths(lyxrc_new.windows_style_tex_paths);
		}
	case LyXRC::RC_UIFILE:
	case LyXRC::RC_USER_EMAIL:
	case LyXRC::RC_USER_NAME:
	case LyXRC::RC_USETEMPDIR:
	case LyXRC::RC_USE_ALT_LANG:
	case LyXRC::RC_USE_CONVERTER_CACHE:
	case LyXRC::RC_USE_ESC_CHARS:
	case LyXRC::RC_USE_INP_ENC:
	case LyXRC::RC_USE_PERS_DICT:
	case LyXRC::RC_USE_PIXMAP_CACHE:
	case LyXRC::RC_USE_SPELL_LIB:
	case LyXRC::RC_VIEWDVI_PAPEROPTION:
	case LyXRC::RC_SORT_LAYOUTS:
	case LyXRC::RC_VIEWER:
	case LyXRC::RC_LAST:
		break;
	}
}

} // namespace anon


} // namespace lyx
