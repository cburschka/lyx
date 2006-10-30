/**
 * \file lyxfunc.C
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

#include "lyxfunc.h"

#include "BranchList.h"
#include "buffer.h"
#include "buffer_funcs.h"
#include "bufferlist.h"
#include "bufferparams.h"
#include "BufferView.h"
#include "cursor.h"
#include "CutAndPaste.h"
#include "debug.h"
#include "dispatchresult.h"
#include "encoding.h"
#include "errorlist.h"
#include "exporter.h"
#include "format.h"
#include "funcrequest.h"
#include "FuncStatus.h"
#include "gettext.h"
#include "importer.h"
#include "insetiterator.h"
#include "intl.h"
#include "kbmap.h"
#include "language.h"
#include "LColor.h"
#include "session.h"
#include "lyx_main.h"
#include "lyx_cb.h"
#include "LyXAction.h"
#include "lyxfind.h"
#include "lyxlex.h"
#include "lyxrc.h"
#include "lyxrow.h"
#include "lyxserver.h"
#include "lyxtextclasslist.h"
#include "lyxvc.h"
#include "paragraph.h"
#include "pariterator.h"
#include "ParagraphParameters.h"
#include "undo.h"

#include "insets/insetbox.h"
#include "insets/insetbranch.h"
#include "insets/insetcommand.h"
#include "insets/insetert.h"
#include "insets/insetexternal.h"
#include "insets/insetfloat.h"
#include "insets/insetgraphics.h"
#include "insets/insetinclude.h"
#include "insets/insetnote.h"
#include "insets/insettabular.h"
#include "insets/insetvspace.h"
#include "insets/insetwrap.h"

#include "frontends/Application.h"
#include "frontends/Alert.h"
#include "frontends/Dialogs.h"
#include "frontends/FileDialog.h"
#include "frontends/FontLoader.h"
#include "frontends/Gui.h"
#include "frontends/LyXKeySym.h"
#include "frontends/LyXView.h"
#include "frontends/Menubar.h"
#include "frontends/Toolbars.h"

#include "support/environment.h"
#include "support/filefilterlist.h"
#include "support/filetools.h"
#include "support/forkedcontr.h"
#include "support/fs_extras.h"
#include "support/lstrings.h"
#include "support/path.h"
#include "support/package.h"
#include "support/systemcall.h"
#include "support/convert.h"
#include "support/os.h"

#include <boost/current_function.hpp>
#include <boost/filesystem/operations.hpp>

#include <sstream>


namespace lyx {

using bv_funcs::freefont2string;

using support::absolutePath;
using support::addName;
using support::addPath;
using support::bformat;
using support::changeExtension;
using support::contains;
using support::FileFilterList;
using support::fileSearch;
using support::ForkedcallsController;
using support::i18nLibFileSearch;
using support::isDirWriteable;
using support::isFileReadable;
using support::isStrInt;
using support::makeAbsPath;
using support::makeDisplayPath;
using support::package;
using support::quoteName;
using support::rtrim;
using support::split;
using support::subst;
using support::Systemcall;
using support::token;
using support::trim;
using support::prefixIs;

using std::endl;
using std::make_pair;
using std::pair;
using std::string;
using std::istringstream;
using std::ostringstream;

namespace Alert = frontend::Alert;
namespace fs = boost::filesystem;


// (alkis)
extern tex_accent_struct get_accent(kb_action action);


namespace {

bool getLocalStatus(LCursor cursor,
	       FuncRequest const & cmd, FuncStatus & status)
{
	// Try to fix cursor in case it is broken.
	cursor.fixIfBroken();

	// This is, of course, a mess. Better create a new doc iterator and use
	// this in Inset::getStatus. This might require an additional
	// BufferView * arg, though (which should be avoided)
	//LCursor safe = *this;
	bool res = false;
	for ( ; cursor.depth(); cursor.pop()) {
		//lyxerr << "\nLCursor::getStatus: cmd: " << cmd << endl << *this << endl;
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
	: lyx_view_(0),
	encoded_last_key(0),
	meta_fake_bit(key_modifier::none)
{
}


void LyXFunc::initKeySequences(kb_keymap * kb)
{
	keyseq.reset(new kb_sequence(kb, kb));
	cancel_meta_seq.reset(new kb_sequence(kb, kb));
}


void LyXFunc::setLyXView(LyXView * lv)
{
	lyx_view_ = lv;
}


void LyXFunc::handleKeyFunc(kb_action action)
{
	char_type c = encoded_last_key;

	if (keyseq->length())
		c = 0;

	lyx_view_->view()->getIntl().getTransManager().deadkey(
		c, get_accent(action).accent, view()->getLyXText(), view()->cursor());
	// Need to clear, in case the minibuffer calls these
	// actions
	keyseq->clear();
	// copied verbatim from do_accent_char
	view()->cursor().resetAnchor();
	view()->update();
}


void LyXFunc::processKeySym(LyXKeySymPtr keysym, key_modifier::state state)
{
	lyxerr[Debug::KEY] << "KeySym is " << keysym->getSymbolName() << endl;

	// Do nothing if we have nothing (JMarc)
	if (!keysym->isOK()) {
		lyxerr[Debug::KEY] << "Empty kbd action (probably composing)"
				   << endl;
		return;
	}

	if (keysym->isModifier()) {
		lyxerr[Debug::KEY] << "isModifier true" << endl;
		return;
	}

	//Encoding const * encoding = view()->cursor().getEncoding();
	//encoded_last_key = keysym->getISOEncoded(encoding ? encoding->name() : "");
	size_t encoded_last_key = keysym->getUCSEncoded();

	// Do a one-deep top-level lookup for
	// cancel and meta-fake keys. RVDK_PATCH_5
	cancel_meta_seq->reset();

	FuncRequest func = cancel_meta_seq->addkey(keysym, state);
	lyxerr[Debug::KEY] << BOOST_CURRENT_FUNCTION
			   << " action first set to [" << func.action << ']'
			   << endl;

	// When not cancel or meta-fake, do the normal lookup.
	// Note how the meta_fake Mod1 bit is OR-ed in and reset afterwards.
	// Mostly, meta_fake_bit = key_modifier::none. RVDK_PATCH_5.
	if ((func.action != LFUN_CANCEL) && (func.action != LFUN_META_PREFIX)) {
		// remove Caps Lock and Mod2 as a modifiers
		func = keyseq->addkey(keysym, (state | meta_fake_bit));
		lyxerr[Debug::KEY] << BOOST_CURRENT_FUNCTION
				   << "action now set to ["
				   << func.action << ']' << endl;
	}

	// Dont remove this unless you know what you are doing.
	meta_fake_bit = key_modifier::none;

	// Can this happen now ?
	if (func.action == LFUN_NOACTION) {
		func = FuncRequest(LFUN_COMMAND_PREFIX);
	}

	if (lyxerr.debugging(Debug::KEY)) {
		lyxerr << BOOST_CURRENT_FUNCTION
		       << " Key [action="
		       << func.action << "]["
		       << keyseq->print() << ']'
		       << endl;
	}

	// already here we know if it any point in going further
	// why not return already here if action == -1 and
	// num_bytes == 0? (Lgb)

	if (keyseq->length() > 1) {
		lyx_view_->message(from_utf8(keyseq->print()));
	}


	// Maybe user can only reach the key via holding down shift.
	// Let's see. But only if shift is the only modifier
	if (func.action == LFUN_UNKNOWN_ACTION &&
	    state == key_modifier::shift) {
		lyxerr[Debug::KEY] << "Trying without shift" << endl;
		func = keyseq->addkey(keysym, key_modifier::none);
		lyxerr[Debug::KEY] << "Action now " << func.action << endl;
	}

	if (func.action == LFUN_UNKNOWN_ACTION) {
		// Hmm, we didn't match any of the keysequences. See
		// if it's normal insertable text not already covered
		// by a binding
		if (keysym->isText() && keyseq->length() == 1) {
			lyxerr[Debug::KEY] << "isText() is true, inserting." << endl;
			func = FuncRequest(LFUN_SELF_INSERT,
					   FuncRequest::KEYBOARD);
		} else {
			lyxerr[Debug::KEY] << "Unknown, !isText() - giving up" << endl;
			lyx_view_->message(_("Unknown function."));
			return;
		}
	}

	if (func.action == LFUN_SELF_INSERT) {
		if (encoded_last_key != 0) {
			docstring const arg(1, encoded_last_key);
			dispatch(FuncRequest(LFUN_SELF_INSERT, arg,
					     FuncRequest::KEYBOARD));
			lyxerr[Debug::KEY]
				<< "SelfInsert arg[`" << to_utf8(arg) << "']" << endl;
		}
	} else {
		dispatch(func);
	}
}


FuncStatus LyXFunc::getStatus(FuncRequest const & cmd) const
{
	//lyxerr << "LyXFunc::getStatus: cmd: " << cmd << endl;
	FuncStatus flag;

	if (cmd.action == LFUN_LYX_QUIT) {
		flag.message(from_utf8(N_("Exiting")));
		flag.enabled(true);
		return flag;
	}

	LCursor & cur = view()->cursor();

	/* In LyX/Mac, when a dialog is open, the menus of the
	   application can still be accessed without giving focus to
	   the main window. In this case, we want to disable the menu
	   entries that are buffer-related.

	   Note that this code is not perfect, as bug 1941 attests:
	   http://bugzilla.lyx.org/show_bug.cgi?id=1941#c4
	*/
	Buffer * buf;
	if (cmd.origin == FuncRequest::UI && !lyx_view_->hasFocus())
		buf = 0;
	else
		buf = lyx_view_->buffer();

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
	case LFUN_BUFFER_TOGGLE_READ_ONLY:
		flag.setOnOff(buf->isReadonly());
		break;

	case LFUN_BUFFER_SWITCH:
		// toggle on the current buffer, but do not toggle off
		// the other ones (is that a good idea?)
		if (to_utf8(cmd.argument()) == buf->fileName())
			flag.setOnOff(true);
		break;

	case LFUN_BUFFER_EXPORT:
		enable = cmd.argument() == "custom"
			|| Exporter::isExportable(*buf, to_utf8(cmd.argument()));
		break;

	case LFUN_BUFFER_CHKTEX:
		enable = buf->isLatex() && !lyxrc.chktex_command.empty();
		break;

	case LFUN_BUILD_PROGRAM:
		enable = Exporter::isExportable(*buf, "program");
		break;

	case LFUN_LAYOUT_TABULAR:
		enable = cur.innerInsetOfType(InsetBase::TABULAR_CODE);
		break;

	case LFUN_LAYOUT:
	case LFUN_LAYOUT_PARAGRAPH:
		enable = !cur.inset().forceDefaultParagraphs(cur.idx());
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
		enable = !buf->isUnnamed() && !buf->isClean();
		break;

	case LFUN_INSET_SETTINGS: {
		enable = false;
		if (!cur)
			break;
		InsetBase::Code code = cur.inset().lyxCode();
		switch (code) {
			case InsetBase::TABULAR_CODE:
				enable = cmd.argument() == "tabular";
				break;
			case InsetBase::ERT_CODE:
				enable = cmd.argument() == "ert";
				break;
			case InsetBase::FLOAT_CODE:
				enable = cmd.argument() == "float";
				break;
			case InsetBase::WRAP_CODE:
				enable = cmd.argument() == "wrap";
				break;
			case InsetBase::NOTE_CODE:
				enable = cmd.argument() == "note";
				break;
			case InsetBase::BRANCH_CODE:
				enable = cmd.argument() == "branch";
				break;
			case InsetBase::BOX_CODE:
				enable = cmd.argument() == "box";
				break;
			default:
				break;
		}
		break;
	}

	case LFUN_INSET_APPLY: {
		string const name = cmd.getArg(0);
		InsetBase * inset = lyx_view_->getDialogs().getOpenInset(name);
		if (inset) {
			FuncRequest fr(LFUN_INSET_MODIFY, cmd.argument());
			FuncStatus fs;
			bool const success = inset->getStatus(cur, fr, fs);
			// Every inset is supposed to handle this
			BOOST_ASSERT(success);
			flag |= fs;
		} else {
			FuncRequest fr(LFUN_INSET_INSERT, cmd.argument());
			flag |= getStatus(fr);
		}
		enable = flag.enabled();
		break;
	}

	case LFUN_DIALOG_SHOW: {
		string const name = cmd.getArg(0);
		if (!buf)
			enable = name == "aboutlyx"
				|| name == "file"
				|| name == "forks"
				|| name == "prefs"
				|| name == "texinfo";
		else if (name == "print")
			enable = Exporter::isExportable(*buf, "dvi")
				&& lyxrc.print_command != "none";
		else if (name == "character" || name == "mathpanel")
			enable = cur.inset().lyxCode() != InsetBase::ERT_CODE;
		else if (name == "latexlog")
			enable = isFileReadable(buf->getLogName().second);
#if !defined (USE_ASPELL) && !defined (USE_ISPELL) && !defined (USE_PSPELL)
		else if (name == "spellchecker")
			enable = false;
#endif
		else if (name == "vclog")
			enable = buf->lyxvc().inUse();
		else if (name == "view-source")
			enable = buf;
		break;
	}

	case LFUN_DIALOG_SHOW_NEW_INSET:
		enable = cur.inset().lyxCode() != InsetBase::ERT_CODE;
		break;

	case LFUN_DIALOG_UPDATE: {
		string const name = cmd.getArg(0);
		if (!buf)
			enable = name == "prefs";
		break;
	}

	case LFUN_CITATION_INSERT: {
		FuncRequest fr(LFUN_INSET_INSERT, "citation");
		enable = getStatus(fr).enabled();
		break;
	}

	case LFUN_BUFFER_WRITE: {
		enable = view()->buffer()->isUnnamed()
			|| !view()->buffer()->isClean();
		break;
	}

	// this one is difficult to get right. As a half-baked
	// solution, we consider only the first action of the sequence
	case LFUN_COMMAND_SEQUENCE: {
		// argument contains ';'-terminated commands
		string const firstcmd = token(to_utf8(cmd.argument()), ';', 0);
		FuncRequest func(lyxaction.lookupFunc(firstcmd));
		func.origin = cmd.origin;
		flag = getStatus(func);
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
	case LFUN_BUFFER_WRITE_AS:
	case LFUN_BUFFER_UPDATE:
	case LFUN_BUFFER_VIEW:
	case LFUN_BUFFER_IMPORT:
	case LFUN_TOC_VIEW:
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
	case LFUN_LYXRC_APPLY:
	case LFUN_BUFFER_NEXT:
	case LFUN_BUFFER_PREVIOUS:
	case LFUN_WINDOW_NEW:
	case LFUN_WINDOW_CLOSE:
		// these are handled in our dispatch()
		break;

	default:
		if (!getLocalStatus(cur, cmd, flag))
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
	if (buf && lookupChangeType(cur, true) == Change::DELETED
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
	Buffer & buf = *bv->buffer();
	if (buf.isClean())
		return true;

	docstring const file = makeDisplayPath(buf.fileName(), 30);
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


void loadTextclass(string const & name)
{
	std::pair<bool, textclass_type> const tc_pair =
		textclasslist.numberOfClass(name);

	if (!tc_pair.first) {
		lyxerr << "Document class \"" << name
		       << "\" does not exist."
		       << std::endl;
		return;
	}

	textclass_type const tc = tc_pair.second;

	if (!textclasslist[tc].load()) {
		docstring s = bformat(_("The document could not be converted\n"
						  "into the document class %1$s."),
				   from_utf8(textclasslist[tc].name()));
		Alert::error(_("Could not change class"), s);
	}
}


void actOnUpdatedPrefs(LyXRC const & lyxrc_orig, LyXRC const & lyxrc_new);

} //namespace anon


void LyXFunc::dispatch(FuncRequest const & cmd)
{
	string const argument = to_utf8(cmd.argument());
	kb_action const action = cmd.action;

	lyxerr[Debug::ACTION] << endl << "LyXFunc::dispatch: cmd: " << cmd << endl;
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
		lyxerr[Debug::ACTION] << "LyXFunc::dispatch: "
		       << lyxaction.getActionName(action)
		       << " [" << action << "] is disabled at this location"
		       << endl;
		setErrorMessage(flag.message());
	} else {
		switch (action) {

		case LFUN_WORD_FIND_FORWARD:
		case LFUN_WORD_FIND_BACKWARD: {
			BOOST_ASSERT(lyx_view_ && lyx_view_->view());
			static string last_search;
			string searched_string;

			if (!argument.empty()) {
				last_search = argument;
				searched_string = argument;
			} else {
				searched_string = last_search;
			}

			if (searched_string.empty())
				break;

			bool const fw = action == LFUN_WORD_FIND_FORWARD;
			string const data =
				find2string(searched_string, true, false, fw);
			find(view(), FuncRequest(LFUN_WORD_FIND, data));
			break;
		}

		case LFUN_COMMAND_PREFIX:
			BOOST_ASSERT(lyx_view_);
			lyx_view_->message(from_utf8(keyseq->printOptions()));
			break;

		case LFUN_COMMAND_EXECUTE:
			BOOST_ASSERT(lyx_view_);
			lyx_view_->getToolbars().display("minibuffer", true);
			lyx_view_->focus_command_buffer();
			break;

		case LFUN_CANCEL:
			BOOST_ASSERT(lyx_view_ && lyx_view_->view());
			keyseq->reset();
			meta_fake_bit = key_modifier::none;
			if (view()->buffer())
				// cancel any selection
				dispatch(FuncRequest(LFUN_MARK_OFF));
			setMessage(_("Cancel"));
			break;

		case LFUN_META_PREFIX:
			meta_fake_bit = key_modifier::alt;
			setMessage(from_utf8(keyseq->print()));
			break;

		case LFUN_BUFFER_TOGGLE_READ_ONLY:
			BOOST_ASSERT(lyx_view_ && lyx_view_->view() && lyx_view_->buffer());
			if (lyx_view_->buffer()->lyxvc().inUse())
				lyx_view_->buffer()->lyxvc().toggleReadOnly();
			else
				lyx_view_->buffer()->setReadonly(
					!lyx_view_->buffer()->isReadonly());
			break;

		// --- Menus -----------------------------------------------
		case LFUN_BUFFER_NEW:
			menuNew(argument, false);
			break;

		case LFUN_BUFFER_NEW_TEMPLATE:
			menuNew(argument, true);
			break;

		case LFUN_BUFFER_CLOSE:
			closeBuffer();
			view()->update();
			break;

		case LFUN_BUFFER_WRITE:
			BOOST_ASSERT(lyx_view_ && lyx_view_->buffer());
			if (!lyx_view_->buffer()->isUnnamed()) {
				docstring const str = bformat(_("Saving document %1$s..."),
					 makeDisplayPath(lyx_view_->buffer()->fileName()));
				lyx_view_->message(str);
				menuWrite(lyx_view_->buffer());
				lyx_view_->message(str + _(" done."));
			} else
				writeAs(lyx_view_->buffer());
			updateFlags = Update::None;
			break;

		case LFUN_BUFFER_WRITE_AS:
			BOOST_ASSERT(lyx_view_ && lyx_view_->buffer());
			writeAs(lyx_view_->buffer(), argument);
			updateFlags = Update::None;
			break;

		case LFUN_BUFFER_RELOAD: {
			BOOST_ASSERT(lyx_view_ && lyx_view_->buffer());
			docstring const file = makeDisplayPath(view()->buffer()->fileName(), 20);
			docstring text = bformat(_("Any changes will be lost. Are you sure "
							     "you want to revert to the saved version of the document %1$s?"), file);
			int const ret = Alert::prompt(_("Revert to saved document?"),
				text, 0, 1, _("&Revert"), _("&Cancel"));

			if (ret == 0)
				view()->reload();
			break;
		}

		case LFUN_BUFFER_UPDATE:
			BOOST_ASSERT(lyx_view_ && lyx_view_->buffer());
			Exporter::Export(lyx_view_->buffer(), argument, true);
			break;

		case LFUN_BUFFER_VIEW:
			BOOST_ASSERT(lyx_view_ && lyx_view_->buffer());
			Exporter::preview(lyx_view_->buffer(), argument);
			break;

		case LFUN_BUILD_PROGRAM:
			BOOST_ASSERT(lyx_view_ && lyx_view_->buffer());
			Exporter::Export(lyx_view_->buffer(), "program", true);
			break;

		case LFUN_BUFFER_CHKTEX:
			BOOST_ASSERT(lyx_view_ && lyx_view_->buffer());
			lyx_view_->buffer()->runChktex();
			break;

		case LFUN_BUFFER_EXPORT:
			BOOST_ASSERT(lyx_view_ && lyx_view_->buffer());
			if (argument == "custom")
				lyx_view_->getDialogs().show("sendto");
			else {
				Exporter::Export(lyx_view_->buffer(), argument, false);
			}
			break;

		case LFUN_BUFFER_EXPORT_CUSTOM: {
			BOOST_ASSERT(lyx_view_ && lyx_view_->buffer());
			string format_name;
			string command = split(argument, format_name, ' ');
			Format const * format = formats.getFormat(format_name);
			if (!format) {
				lyxerr << "Format \"" << format_name
				       << "\" not recognized!"
				       << std::endl;
				break;
			}

			Buffer * buffer = lyx_view_->buffer();

			// The name of the file created by the conversion process
			string filename;

			// Output to filename
			if (format->name() == "lyx") {
				string const latexname =
					buffer->getLatexName(false);
				filename = changeExtension(latexname,
							   format->extension());
				filename = addName(buffer->temppath(), filename);

				if (!buffer->writeFile(filename))
					break;

			} else {
				Exporter::Export(buffer, format_name, true, filename);
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
			string target;
			string target_name;
			string command = split(split(argument, target, ' '),
					       target_name, ' ');

			if (target.empty()
			    || target_name.empty()
			    || command.empty()) {
				lyxerr << "Unable to parse \""
				       << argument << '"' << std::endl;
				break;
			}
			if (target != "printer" && target != "file") {
				lyxerr << "Unrecognized target \""
				       << target << '"' << std::endl;
				break;
			}

			Buffer * buffer = lyx_view_->buffer();

			if (!Exporter::Export(buffer, "dvi", true)) {
				showPrintError(buffer->fileName());
				break;
			}

			// Push directory path.
			string const path = buffer->temppath();
			support::Path p(path);

			// there are three cases here:
			// 1. we print to a file
			// 2. we print directly to a printer
			// 3. we print using a spool command (print to file first)
			Systemcall one;
			int res = 0;
			string const dviname =
				changeExtension(buffer->getLatexName(true),
						"dvi");

			if (target == "printer") {
				if (!lyxrc.print_spool_command.empty()) {
					// case 3: print using a spool
					string const psname =
						changeExtension(dviname,".ps");
					command += lyxrc.print_to_file
						+ quoteName(psname)
						+ ' '
						+ quoteName(dviname);

					string command2 =
						lyxrc.print_spool_command +' ';
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
					res = one.startscript(
						Systemcall::DontWait,
						command + quoteName(dviname));
				}

			} else {
				// case 1: print to a file
				command += lyxrc.print_to_file
					+ quoteName(makeAbsPath(target_name,
								path))
					+ ' '
					+ quoteName(dviname);
				res = one.startscript(Systemcall::DontWait,
						      command);
			}

			if (res != 0)
				showPrintError(buffer->fileName());
			break;
		}

		case LFUN_BUFFER_IMPORT:
			doImport(argument);
			break;

		case LFUN_LYX_QUIT:
			if (argument != "force") {
				if (!theApp->gui().closeAll())
					break;
				lyx_view_ = 0;
			}

			// FIXME: this code needs to be transfered somewhere else
			// as lyx_view_ will most certainly be null and a same buffer
			// might be visible in more than one LyXView.
			if (lyx_view_ && lyx_view_->view()->buffer()) {
				// save cursor Position for opened files to .lyx/session
				LyX::ref().session().lastFilePos().save(lyx_view_->buffer()->fileName(),
					boost::tie(view()->cursor().pit(), view()->cursor().pos()) );
				// save bookmarks to .lyx/session
				view()->saveSavedPositions();
			}

			LyX::ref().quit();
			break;

		case LFUN_TOC_VIEW: {
			BOOST_ASSERT(lyx_view_);
			InsetCommandParams p("tableofcontents");
			string const data = InsetCommandMailer::params2string("toc", p);
			lyx_view_->getDialogs().show("toc", data, 0);
			break;
		}

		case LFUN_BUFFER_AUTO_SAVE:
			autoSave(view());
			break;

		case LFUN_RECONFIGURE:
			reconfigure(view());
			break;

		case LFUN_HELP_OPEN: {
			BOOST_ASSERT(lyx_view_);
			string const arg = argument;
			if (arg.empty()) {
				setErrorMessage(_("Missing argument"));
				break;
			}
			string const fname = i18nLibFileSearch("doc", arg, "lyx");
			if (fname.empty()) {
				lyxerr << "LyX: unable to find documentation file `"
							 << arg << "'. Bad installation?" << endl;
				break;
			}
			lyx_view_->message(bformat(_("Opening help file %1$s..."),
				makeDisplayPath(fname)));
			lyx_view_->loadLyXFile(fname, false);
			break;
		}

		// --- version control -------------------------------
		case LFUN_VC_REGISTER:
			BOOST_ASSERT(lyx_view_ && lyx_view_->buffer());
			if (!ensureBufferClean(view()))
				break;
			if (!lyx_view_->buffer()->lyxvc().inUse()) {
				lyx_view_->buffer()->lyxvc().registrer();
				view()->reload();
			}
			break;

		case LFUN_VC_CHECK_IN:
			BOOST_ASSERT(lyx_view_ && lyx_view_->buffer());
			if (!ensureBufferClean(view()))
				break;
			if (lyx_view_->buffer()->lyxvc().inUse()
					&& !lyx_view_->buffer()->isReadonly()) {
				lyx_view_->buffer()->lyxvc().checkIn();
				view()->reload();
			}
			break;

		case LFUN_VC_CHECK_OUT:
			BOOST_ASSERT(lyx_view_ && lyx_view_->buffer());
			if (!ensureBufferClean(view()))
				break;
			if (lyx_view_->buffer()->lyxvc().inUse()
					&& lyx_view_->buffer()->isReadonly()) {
				lyx_view_->buffer()->lyxvc().checkOut();
				view()->reload();
			}
			break;

		case LFUN_VC_REVERT:
			BOOST_ASSERT(lyx_view_ && lyx_view_->buffer());
			lyx_view_->buffer()->lyxvc().revert();
			view()->reload();
			break;

		case LFUN_VC_UNDO_LAST:
			BOOST_ASSERT(lyx_view_ && lyx_view_->buffer());
			lyx_view_->buffer()->lyxvc().undoLast();
			view()->reload();
			break;

		// --- buffers ----------------------------------------
		case LFUN_BUFFER_SWITCH:
			BOOST_ASSERT(lyx_view_);
			lyx_view_->setBuffer(theBufferList().getBuffer(argument));
			break;

		case LFUN_BUFFER_NEXT:
			BOOST_ASSERT(lyx_view_);
			lyx_view_->setBuffer(theBufferList().next(view()->buffer()));
			break;

		case LFUN_BUFFER_PREVIOUS:
			BOOST_ASSERT(lyx_view_);
			lyx_view_->setBuffer(theBufferList().previous(view()->buffer()));
			break;

		case LFUN_FILE_NEW:
			BOOST_ASSERT(lyx_view_);
			newFile(view(), argument);
			break;

		case LFUN_FILE_OPEN:
			BOOST_ASSERT(lyx_view_);
			open(argument);
			break;

		case LFUN_DROP_LAYOUTS_CHOICE:
			BOOST_ASSERT(lyx_view_);
			lyx_view_->getToolbars().openLayoutList();
			break;

		case LFUN_MENU_OPEN:
			BOOST_ASSERT(lyx_view_);
			lyx_view_->getMenubar().openByName(from_utf8(argument));
			break;

		// --- lyxserver commands ----------------------------
		case LFUN_SERVER_GET_NAME:
			BOOST_ASSERT(lyx_view_ && lyx_view_->buffer());
			setMessage(from_utf8(lyx_view_->buffer()->fileName()));
			lyxerr[Debug::INFO] << "FNAME["
							 << lyx_view_->buffer()->fileName()
							 << "] " << endl;
			break;

		case LFUN_SERVER_NOTIFY:
			dispatch_buffer = from_utf8(keyseq->print());
			theLyXServer().notifyClient(to_utf8(dispatch_buffer));
			break;

		case LFUN_SERVER_GOTO_FILE_ROW: {
			BOOST_ASSERT(lyx_view_);
			string file_name;
			int row;
			istringstream is(argument);
			is >> file_name >> row;
			if (prefixIs(file_name, package().temp_dir())) {
				// Needed by inverse dvi search. If it is a file
				// in tmpdir, call the apropriated function
				lyx_view_->setBuffer(theBufferList().getBufferFromTmp(file_name));
			} else {
				// Must replace extension of the file to be .lyx
				// and get full path
				string const s = changeExtension(file_name, ".lyx");
				// Either change buffer or load the file
				if (theBufferList().exists(s)) {
					lyx_view_->setBuffer(theBufferList().getBuffer(s));
				} else {
					lyx_view_->loadLyXFile(s);
				}
			}

			view()->setCursorFromRow(row);

			view()->center();
			// see BufferView::center()
			break;
		}

		case LFUN_DIALOG_SHOW: {
			BOOST_ASSERT(lyx_view_);
			string const name = cmd.getArg(0);
			string data = trim(to_utf8(cmd.argument()).substr(name.size()));

			if (name == "character") {
				data = freefont2string();
				if (!data.empty())
					lyx_view_->getDialogs().show("character", data);
			} else if (name == "latexlog") {
				pair<Buffer::LogType, string> const logfile =
					lyx_view_->buffer()->getLogName();
				switch (logfile.first) {
				case Buffer::latexlog:
					data = "latex ";
					break;
				case Buffer::buildlog:
					data = "literate ";
					break;
				}
				data += LyXLex::quoteString(logfile.second);
				lyx_view_->getDialogs().show("log", data);
			} else if (name == "vclog") {
				string const data = "vc " +
					LyXLex::quoteString(lyx_view_->buffer()->lyxvc().getLogFile());
				lyx_view_->getDialogs().show("log", data);
			} else
				lyx_view_->getDialogs().show(name, data);
			break;
		}

		case LFUN_DIALOG_SHOW_NEW_INSET: {
			BOOST_ASSERT(lyx_view_);
			string const name = cmd.getArg(0);
			string data = trim(to_utf8(cmd.argument()).substr(name.size()));
			if (name == "bibitem" ||
			    name == "bibtex" ||
			    name == "index" ||
			    name == "label" ||
			    name == "ref" ||
			    name == "toc" ||
			    name == "url") {
				InsetCommandParams p(name);
				data = InsetCommandMailer::params2string(name, p);
			} else if (name == "include") {
				InsetCommandParams p(data);
				data = InsetIncludeMailer::params2string(p);
			} else if (name == "box") {
				// \c data == "Boxed" || "Frameless" etc
				InsetBoxParams p(data);
				data = InsetBoxMailer::params2string(p);
			} else if (name == "branch") {
				InsetBranchParams p;
				data = InsetBranchMailer::params2string(p);
			} else if (name == "citation") {
				InsetCommandParams p("cite");
				data = InsetCommandMailer::params2string(name, p);
			} else if (name == "ert") {
				data = InsetERTMailer::params2string(InsetCollapsable::Open);
			} else if (name == "external") {
				InsetExternalParams p;
				Buffer const & buffer = *lyx_view_->buffer();
				data = InsetExternalMailer::params2string(p, buffer);
			} else if (name == "float") {
				InsetFloatParams p;
				data = InsetFloatMailer::params2string(p);
			} else if (name == "graphics") {
				InsetGraphicsParams p;
				Buffer const & buffer = *lyx_view_->buffer();
				data = InsetGraphicsMailer::params2string(p, buffer);
			} else if (name == "note") {
				InsetNoteParams p;
				data = InsetNoteMailer::params2string(p);
			} else if (name == "vspace") {
				VSpace space;
				data = InsetVSpaceMailer::params2string(space);
			} else if (name == "wrap") {
				InsetWrapParams p;
				data = InsetWrapMailer::params2string(p);
			}
			lyx_view_->getDialogs().show(name, data, 0);
			break;
		}

		case LFUN_DIALOG_UPDATE: {
			BOOST_ASSERT(lyx_view_);
			string const & name = argument;
			// Can only update a dialog connected to an existing inset
			InsetBase * inset = lyx_view_->getDialogs().getOpenInset(name);
			if (inset) {
				FuncRequest fr(LFUN_INSET_DIALOG_UPDATE, cmd.argument());
				inset->dispatch(view()->cursor(), fr);
			} else if (name == "paragraph") {
				dispatch(FuncRequest(LFUN_PARAGRAPH_UPDATE));
			} else if (name == "prefs") {
				lyx_view_->getDialogs().update(name, string());
			}
			break;
		}

		case LFUN_DIALOG_HIDE:
			Dialogs::hide(argument, 0);
			break;

		case LFUN_DIALOG_DISCONNECT_INSET:
			BOOST_ASSERT(lyx_view_);
			lyx_view_->getDialogs().disconnect(argument);
			break;


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
					opt1 = '[' + token(argument, '|', 1) + ']';
				}
				std::ostringstream os;
				os << "citation LatexCommand\n"
				   << "\\cite" << opt1 << "{" << arg << "}\n"
				   << "\\end_inset";
				FuncRequest fr(LFUN_INSET_INSERT, os.str());
				dispatch(fr);
			} else
				dispatch(FuncRequest(LFUN_DIALOG_SHOW, "citation"));
			break;
		}

		case LFUN_BUFFER_CHILD_OPEN: {
			BOOST_ASSERT(lyx_view_);
			string const filename =
				makeAbsPath(argument, lyx_view_->buffer()->filePath());
			// FIXME Should use bformat
			setMessage(_("Opening child document ") +
					 makeDisplayPath(filename) + "...");
			view()->savePosition(0);
			string const parentfilename = lyx_view_->buffer()->fileName();
			if (theBufferList().exists(filename))
				lyx_view_->setBuffer(theBufferList().getBuffer(filename));
			else
				lyx_view_->loadLyXFile(filename);
			// Set the parent name of the child document.
			// This makes insertion of citations and references in the child work,
			// when the target is in the parent or another child document.
			lyx_view_->buffer()->setParentName(parentfilename);
			break;
		}

		case LFUN_TOGGLE_CURSOR_FOLLOWS_SCROLLBAR:
			BOOST_ASSERT(lyx_view_);
			lyxrc.cursor_follows_scrollbar = !lyxrc.cursor_follows_scrollbar;
			break;

		case LFUN_KEYMAP_OFF:
			BOOST_ASSERT(lyx_view_);
			lyx_view_->view()->getIntl().keyMapOn(false);
			break;

		case LFUN_KEYMAP_PRIMARY:
			BOOST_ASSERT(lyx_view_);
			lyx_view_->view()->getIntl().keyMapPrim();
			break;

		case LFUN_KEYMAP_SECONDARY:
			BOOST_ASSERT(lyx_view_);
			lyx_view_->view()->getIntl().keyMapSec();
			break;

		case LFUN_KEYMAP_TOGGLE:
			BOOST_ASSERT(lyx_view_);
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

		case LFUN_PREFERENCES_SAVE: {
			support::Path p(package().user_support());
			lyxrc.write("preferences", false);
			break;
		}

		case LFUN_SCREEN_FONT_UPDATE:
			BOOST_ASSERT(lyx_view_);
			// handle the screen font changes.
			lyxrc.set_font_norm_type();
			theFontLoader().update();
			// All visible buffers will need resize
			view()->resize();
			break;

		case LFUN_SET_COLOR: {
			string lyx_name;
			string const x11_name = split(argument, lyx_name, ' ');
			if (lyx_name.empty() || x11_name.empty()) {
				setErrorMessage(_("Syntax: set-color <lyx_name>"
							" <x11_name>"));
				break;
			}

			bool const graphicsbg_changed =
				(lyx_name == lcolor.getLyXName(LColor::graphicsbg) &&
				 x11_name != lcolor.getX11Name(LColor::graphicsbg));

			if (!lcolor.setColor(lyx_name, x11_name)) {
				setErrorMessage(
						bformat(_("Set-color \"%1$s\" failed "
								       "- color is undefined or "
								       "may not be redefined"),
									   from_utf8(lyx_name)));
				break;
			}

			theApp->updateColor(lcolor.getFromLyXName(lyx_name));

			if (graphicsbg_changed) {
#ifdef WITH_WARNINGS
#warning FIXME!! The graphics cache no longer has a changeDisplay method.
#endif
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

		case LFUN_INSET_APPLY: {
			BOOST_ASSERT(lyx_view_);
			string const name = cmd.getArg(0);
			InsetBase * inset = lyx_view_->getDialogs().getOpenInset(name);
			if (inset) {
				FuncRequest fr(LFUN_INSET_MODIFY, argument);
				inset->dispatch(view()->cursor(), fr);
			} else {
				FuncRequest fr(LFUN_INSET_INSERT, argument);
				dispatch(fr);
			}
			// ideally, the update flag should be set by the insets,
			// but this is not possible currently
			updateFlags = Update::Force | Update::FitCursor;
			break;
		}

		case LFUN_ALL_INSETS_TOGGLE: {
			BOOST_ASSERT(lyx_view_);
			string action;
			string const name = split(argument, action, ' ');
			InsetBase::Code const inset_code =
				InsetBase::translate(name);

			LCursor & cur = view()->cursor();
			FuncRequest fr(LFUN_INSET_TOGGLE, action);

			InsetBase & inset = lyx_view_->buffer()->inset();
			InsetIterator it  = inset_iterator_begin(inset);
			InsetIterator const end = inset_iterator_end(inset);
			for (; it != end; ++it) {
				if (inset_code == InsetBase::NO_CODE
				    || inset_code == it->lyxCode()) {
					LCursor tmpcur = cur;
					tmpcur.pushLeft(*it);
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
			else
				buffer.updateDocLang(newL);
			break;
		}

		case LFUN_BUFFER_SAVE_AS_DEFAULT: {
			string const fname =
				addName(addPath(package().user_support(), "templates/"),
					"defaults.lyx");
			Buffer defaults(fname);

			istringstream ss(argument);
			LyXLex lex(0,0);
			lex.setStream(ss);
			int const unknown_tokens = defaults.readHeader(lex);

			if (unknown_tokens != 0) {
				lyxerr << "Warning in LFUN_BUFFER_SAVE_AS_DEFAULT!\n"
				       << unknown_tokens << " unknown token"
				       << (unknown_tokens == 1 ? "" : "s")
				       << endl;
			}

			if (defaults.writeFile(defaults.fileName()))
				// FIXME Should use bformat
				setMessage(_("Document defaults saved in ")
					   + makeDisplayPath(fname));
			else
				setErrorMessage(_("Unable to save document defaults"));
			break;
		}

		case LFUN_BUFFER_PARAMS_APPLY: {
			BOOST_ASSERT(lyx_view_);
			biblio::CiteEngine const engine =
				lyx_view_->buffer()->params().cite_engine;

			istringstream ss(argument);
			LyXLex lex(0,0);
			lex.setStream(ss);
			int const unknown_tokens =
				lyx_view_->buffer()->readHeader(lex);

			if (unknown_tokens != 0) {
				lyxerr << "Warning in LFUN_BUFFER_PARAMS_APPLY!\n"
				       << unknown_tokens << " unknown token"
				       << (unknown_tokens == 1 ? "" : "s")
				       << endl;
			}
			if (engine == lyx_view_->buffer()->params().cite_engine)
				break;

			LCursor & cur = view()->cursor();
			FuncRequest fr(LFUN_INSET_REFRESH);

			InsetBase & inset = lyx_view_->buffer()->inset();
			InsetIterator it  = inset_iterator_begin(inset);
			InsetIterator const end = inset_iterator_end(inset);
			for (; it != end; ++it)
				if (it->lyxCode() == InsetBase::CITE_CODE)
					it->dispatch(cur, fr);
			break;
		}

		case LFUN_TEXTCLASS_APPLY: {
			BOOST_ASSERT(lyx_view_);
			Buffer * buffer = lyx_view_->buffer();

			textclass_type const old_class =
				buffer->params().textclass;

			loadTextclass(argument);

			std::pair<bool, textclass_type> const tc_pair =
				textclasslist.numberOfClass(argument);

			if (!tc_pair.first)
				break;

			textclass_type const new_class = tc_pair.second;
			if (old_class == new_class)
				// nothing to do
				break;

			lyx_view_->message(_("Converting document to new document class..."));
			recordUndoFullDocument(view());
			buffer->params().textclass = new_class;
			StableDocIterator backcur(view()->cursor());
			ErrorList & el = buffer->errorList("Class Switch");
			cap::switchBetweenClasses(
				old_class, new_class,
				static_cast<InsetText &>(buffer->inset()), el);

			view()->setCursor(backcur.asDocIterator(&(buffer->inset())));

			buffer->errors("Class Switch");
			updateLabels(*buffer);
			updateFlags = Update::Force | Update::FitCursor;
			break;
		}

		case LFUN_TEXTCLASS_LOAD:
			loadTextclass(argument);
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
			break;
		}

		case LFUN_WINDOW_NEW:
			LyX::ref().newLyXView();
			break;

		case LFUN_WINDOW_CLOSE:
			BOOST_ASSERT(lyx_view_);
			BOOST_ASSERT(theApp);
			lyx_view_->close();
			// We return here because lyx_view does not exists anymore.
			return;

		default: {
			BOOST_ASSERT(lyx_view_);
			view()->cursor().dispatch(cmd);
			updateFlags = view()->cursor().result().update();
			if (!view()->cursor().result().dispatched())
				if (view()->dispatch(cmd)) 
					updateFlags = Update::Force | Update::FitCursor;
			break;
		}
		}

		if (lyx_view_ && view()->buffer()) {
			// Redraw screen unless explicitly told otherwise.
			// This also initializes the position cache for all insets
			// in (at least partially) visible top-level paragraphs.
			bool needSecondUpdate = false;
			if (updateFlags != Update::None)
				view()->update(updateFlags);
			else
				needSecondUpdate = view()->fitCursor();

			if (needSecondUpdate || updateFlags != Update::None) {
				view()->buffer()->changed();
			}
			lyx_view_->updateStatusBar();

			// if we executed a mutating lfun, mark the buffer as dirty
			if (flag.enabled()
			    && !lyxaction.funcHasFlag(cmd.action, LyXAction::NoBuffer)
			    && !lyxaction.funcHasFlag(cmd.action, LyXAction::ReadOnly))
				view()->buffer()->markDirty();

			if (view()->cursor().inTexted()) {
				lyx_view_->updateLayoutChoice();
			}
		}
	}
	if (!quitting)
		// FIXME UNICODE: _() does not support anything but ascii.
		// Do we need a to_ascii() method?
		sendDispatchMessage(getMessage(), cmd);
}


void LyXFunc::sendDispatchMessage(docstring const & msg, FuncRequest const & cmd)
{
	/* When an action did not originate from the UI/kbd, it makes
	 * sense to avoid updating the GUI. It turns out that this
	 * fixes bug 1941, for reasons that are described here:
	 * http://bugzilla.lyx.org/show_bug.cgi?id=1941#c4
	 */
	if (cmd.origin != FuncRequest::INTERNAL) {
		lyx_view_->updateMenubar();
		lyx_view_->updateToolbars();
	}

	const bool verbose = (cmd.origin == FuncRequest::UI
			      || cmd.origin == FuncRequest::COMMANDBUFFER);

	if (cmd.action == LFUN_SELF_INSERT || !verbose) {
		lyxerr[Debug::ACTION] << "dispatch msg is " << to_utf8(msg) << endl;
		if (!msg.empty())
			lyx_view_->message(msg);
		return;
	}

	docstring dispatch_msg = msg;
	if (!dispatch_msg.empty())
		dispatch_msg += ' ';

	string comname = lyxaction.getActionName(cmd.action);

	bool argsadded = false;

	if (!cmd.argument().empty()) {
		if (cmd.action != LFUN_UNKNOWN_ACTION) {
			comname += ' ' + to_utf8(cmd.argument());
			argsadded = true;
		}
	}

	string const shortcuts = theTopLevelKeymap().printbindings(cmd);

	if (!shortcuts.empty())
		comname += ": " + shortcuts;
	else if (!argsadded && !cmd.argument().empty())
		comname += ' ' + to_utf8(cmd.argument());

	if (!comname.empty()) {
		comname = rtrim(comname);
		dispatch_msg += from_utf8('(' + rtrim(comname) + ')');
	}

	lyxerr[Debug::ACTION] << "verbose dispatch msg "
		<< to_utf8(dispatch_msg) << endl;
	if (!dispatch_msg.empty())
		lyx_view_->message(dispatch_msg);
}


void LyXFunc::menuNew(string const & name, bool fromTemplate)
{
	string initpath = lyxrc.document_path;
	string filename(name);

	if (view()->buffer()) {
		string const trypath = lyx_view_->buffer()->filePath();
		// If directory is writeable, use this as default.
		if (isDirWriteable(trypath))
			initpath = trypath;
	}

	static int newfile_number;

	if (filename.empty()) {
		filename = addName(lyxrc.document_path,
			    "newfile" + convert<string>(++newfile_number) + ".lyx");
		while (theBufferList().exists(filename) || fs::is_readable(filename)) {
			++newfile_number;
			filename = addName(lyxrc.document_path,
					   "newfile" +	convert<string>(newfile_number) +
				    ".lyx");
		}
	}

	// The template stuff
	string templname;
	if (fromTemplate) {
		FileDialog fileDlg(_("Select template file"),
			LFUN_SELECT_FILE_SYNC,
			make_pair(_("Documents|#o#O"), from_utf8(lyxrc.document_path)),
			make_pair(_("Templates|#T#t"), from_utf8(lyxrc.template_path)));

		FileDialog::Result result =
			fileDlg.open(from_utf8(lyxrc.template_path),
				     FileFilterList(_("LyX Documents (*.lyx)")),
				     docstring());

		if (result.first == FileDialog::Later)
			return;
		if (result.second.empty())
			return;
		templname = to_utf8(result.second);
	}

	Buffer * const b = newFile(filename, templname, !name.empty());
	if (b)
		lyx_view_->setBuffer(b);
}


void LyXFunc::open(string const & fname)
{
	string initpath = lyxrc.document_path;

	if (view()->buffer()) {
		string const trypath = lyx_view_->buffer()->filePath();
		// If directory is writeable, use this as default.
		if (isDirWriteable(trypath))
			initpath = trypath;
	}

	string filename;

	if (fname.empty()) {
		FileDialog fileDlg(_("Select document to open"),
			LFUN_FILE_OPEN,
			make_pair(_("Documents|#o#O"), from_utf8(lyxrc.document_path)),
			make_pair(_("Examples|#E#e"), from_utf8(addPath(package().system_support(), "examples"))));

		FileDialog::Result result =
			fileDlg.open(from_utf8(initpath),
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
	string const fullpath = fileSearch(string(), filename, "lyx");
	if (!fullpath.empty()) {
		filename = fullpath;
	}

	docstring const disp_fn = makeDisplayPath(filename);

	// if the file doesn't exist, let the user create one
	if (!fs::exists(filename)) {
		// the user specifically chose this name. Believe him.
		Buffer * const b = newFile(filename, string(), true);
		if (b)
			lyx_view_->setBuffer(b);
		return;
	}

	lyx_view_->message(bformat(_("Opening document %1$s..."), disp_fn));

	docstring str2;
	if (lyx_view_->loadLyXFile(filename)) {
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

	lyxerr[Debug::INFO] << "LyXFunc::doImport: " << format
			    << " file: " << filename << endl;

	// need user interaction
	if (filename.empty()) {
		string initpath = lyxrc.document_path;

		if (view()->buffer()) {
			string const trypath = lyx_view_->buffer()->filePath();
			// If directory is writeable, use this as default.
			if (isDirWriteable(trypath))
				initpath = trypath;
		}

		docstring const text = bformat(_("Select %1$s file to import"),
			formats.prettyName(format));

		FileDialog fileDlg(text,
			LFUN_BUFFER_IMPORT,
			make_pair(_("Documents|#o#O"), from_utf8(lyxrc.document_path)),
			make_pair(_("Examples|#E#e"),
				  from_utf8(addPath(package().system_support(), "examples"))));

		docstring filter = formats.prettyName(format);
		filter += " (*.";
		// FIXME UNICODE
		filter += from_utf8(formats.extension(format));
		filter += ')';

		FileDialog::Result result =
			fileDlg.open(from_utf8(initpath),
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
	filename = makeAbsPath(filename);

	string const lyxfile = changeExtension(filename, ".lyx");

	// Check if the document already is open
	if (use_gui && theBufferList().exists(lyxfile)) {
		if (!theBufferList().close(theBufferList().getBuffer(lyxfile), true)) {
			lyx_view_->message(_("Canceled."));
			return;
		}
	}

	// if the file exists already, and we didn't do
	// -i lyx thefile.lyx, warn
	if (fs::exists(lyxfile) && filename != lyxfile) {
		docstring const file = makeDisplayPath(lyxfile, 30);

		docstring text = bformat(_("The document %1$s already exists.\n\n"
						     "Do you want to over-write that document?"), file);
		int const ret = Alert::prompt(_("Over-write document?"),
			text, 0, 1, _("&Over-write"), _("&Cancel"));

		if (ret == 1) {
			lyx_view_->message(_("Canceled."));
			return;
		}
	}

	ErrorList errorList;
	Importer::Import(lyx_view_, filename, format, errorList);
	// FIXME (Abdel 12/08/06): Is there a need to display the error list here?
}


void LyXFunc::closeBuffer()
{
	// save current cursor position
	LyX::ref().session().lastFilePos().save(lyx_view_->buffer()->fileName(),
		boost::tie(view()->cursor().pit(), view()->cursor().pos()) );
	if (theBufferList().close(lyx_view_->buffer(), true) && !quitting) {
		if (theBufferList().empty()) {
			// need this otherwise SEGV may occur while
			// trying to set variables that don't exist
			// since there's no current buffer
			lyx_view_->getDialogs().hideBufferDependent();
		} else {
			lyx_view_->setBuffer(theBufferList().first());
		}
	}
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


string const LyXFunc::viewStatusMessage()
{
	// When meta-fake key is pressed, show the key sequence so far + "M-".
	if (wasMetaKey())
		return keyseq->print() + "M-";

	// Else, when a non-complete key sequence is pressed,
	// show the available options.
	if (keyseq->length() > 0 && !keyseq->deleted())
		return keyseq->printOptions();

	if (!view()->buffer())
		return to_utf8(_("Welcome to LyX!"));

	return view()->cursor().currentState();
}


BufferView * LyXFunc::view() const
{
	BOOST_ASSERT(lyx_view_);
	return lyx_view_->view();
}


bool LyXFunc::wasMetaKey() const
{
	return (meta_fake_bit != key_modifier::none);
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
	case LyXRC::RC_ASCIIROFF_COMMAND:
	case LyXRC::RC_ASCII_LINELEN:
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
	case LyXRC::RC_COPIER:
	case LyXRC::RC_CURSOR_FOLLOWS_SCROLLBAR:
	case LyXRC::RC_CUSTOM_EXPORT_COMMAND:
	case LyXRC::RC_CUSTOM_EXPORT_FORMAT:
	case LyXRC::RC_DATE_INSERT_FORMAT:
	case LyXRC::RC_DEFAULT_LANGUAGE:
	case LyXRC::RC_DEFAULT_PAPERSIZE:
	case LyXRC::RC_DIALOGS_ICONIFY_WITH_MAIN:
	case LyXRC::RC_DISPLAY_GRAPHICS:
	case LyXRC::RC_DOCUMENTPATH:
		if (lyxrc_orig.document_path != lyxrc_new.document_path) {
			if (fs::exists(lyxrc_new.document_path) &&
			    fs::is_directory(lyxrc_new.document_path)) {
				support::package().document_dir() = lyxrc.document_path;
			}
		}
	case LyXRC::RC_ESC_CHARS:
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
			support::prependEnvPath("PATH", lyxrc.path_prefix);
		}
	case LyXRC::RC_PERS_DICT:
	case LyXRC::RC_POPUP_BOLD_FONT:
	case LyXRC::RC_POPUP_FONT_ENCODING:
	case LyXRC::RC_POPUP_NORMAL_FONT:
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
	case LyXRC::RC_SCREEN_FONT_ENCODING:
	case LyXRC::RC_SCREEN_FONT_ROMAN:
	case LyXRC::RC_SCREEN_FONT_ROMAN_FOUNDRY:
	case LyXRC::RC_SCREEN_FONT_SANS:
	case LyXRC::RC_SCREEN_FONT_SANS_FOUNDRY:
	case LyXRC::RC_SCREEN_FONT_SCALABLE:
	case LyXRC::RC_SCREEN_FONT_SIZES:
	case LyXRC::RC_SCREEN_FONT_TYPEWRITER:
	case LyXRC::RC_SCREEN_FONT_TYPEWRITER_FOUNDRY:
	case LyXRC::RC_SCREEN_GEOMETRY_HEIGHT:
	case LyXRC::RC_SCREEN_GEOMETRY_WIDTH:
	case LyXRC::RC_SCREEN_GEOMETRY_XYSAVED:
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
			support::os::windows_style_tex_paths(lyxrc_new.windows_style_tex_paths);
		}
	case LyXRC::RC_UIFILE:
	case LyXRC::RC_USER_EMAIL:
	case LyXRC::RC_USER_NAME:
	case LyXRC::RC_USETEMPDIR:
	case LyXRC::RC_USE_ALT_LANG:
	case LyXRC::RC_USE_ESC_CHARS:
	case LyXRC::RC_USE_INP_ENC:
	case LyXRC::RC_USE_PERS_DICT:
	case LyXRC::RC_USE_SPELL_LIB:
	case LyXRC::RC_VIEWDVI_PAPEROPTION:
	case LyXRC::RC_VIEWER:
	case LyXRC::RC_LAST:
		break;
	}
}

} // namespace anon


} // namespace lyx
