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
#include "debug.h"
#include "dispatchresult.h"
#include "encoding.h"
#include "exporter.h"
#include "format.h"
#include "funcrequest.h"
#include "gettext.h"
#include "importer.h"
#include "intl.h"
#include "iterators.h"
#include "kbmap.h"
#include "LColor.h"
#include "lyx_cb.h"
#include "LyXAction.h"
#include "lyxfind.h"
#include "lyxrc.h"
#include "lyxrow.h"
#include "lyxserver.h"
#include "lyxvc.h"
#include "paragraph.h"
#include "ParagraphParameters.h"
#include "undo.h"

#include "insets/insetbox.h"
#include "insets/insetbranch.h"
#include "insets/insetcommand.h"
#include "insets/insetert.h"
#include "insets/insetexternal.h"
#include "insets/insetfloat.h"
#include "insets/insetgraphics.h"
#include "insets/insetnote.h"
#include "insets/insettabular.h"
#include "insets/insetvspace.h"
#include "insets/insetwrap.h"

#include "frontends/Alert.h"
#include "frontends/Dialogs.h"
#include "frontends/FileDialog.h"
#include "frontends/lyx_gui.h"
#include "frontends/LyXKeySym.h"
#include "frontends/LyXView.h"
#include "frontends/Menubar.h"
#include "frontends/Toolbar.h"

#include "support/FileInfo.h"
#include "support/filetools.h"
#include "support/forkedcontr.h"
#include "support/globbing.h"
#include "support/lstrings.h"
#include "support/path.h"
#include "support/path_defines.h"
#include "support/systemcall.h"
#include "support/tostr.h"
#include "support/std_sstream.h"
#include "support/os.h"

using bv_funcs::freefont2string;

using lyx::support::AddName;
using lyx::support::AddPath;
using lyx::support::bformat;
using lyx::support::ChangeExtension;
using lyx::support::contains;
using lyx::support::FileFilterList;
using lyx::support::FileInfo;
using lyx::support::FileSearch;
using lyx::support::ForkedcallsController;
using lyx::support::i18nLibFileSearch;
using lyx::support::IsDirWriteable;
using lyx::support::IsFileReadable;
using lyx::support::isStrInt;
using lyx::support::MakeAbsPath;
using lyx::support::MakeDisplayPath;
using lyx::support::Path;
using lyx::support::QuoteName;
using lyx::support::rtrim;
using lyx::support::split;
using lyx::support::strToInt;
using lyx::support::strToUnsignedInt;
using lyx::support::subst;
using lyx::support::system_lyxdir;
using lyx::support::Systemcall;
using lyx::support::token;
using lyx::support::trim;
using lyx::support::user_lyxdir;
using lyx::support::prefixIs;
using lyx::support::os::getTmpDir;

using std::endl;
using std::make_pair;
using std::pair;
using std::string;
using std::istringstream;


extern BufferList bufferlist;
extern LyXServer * lyxserver;
extern bool selection_possible;

extern boost::scoped_ptr<kb_keymap> toplevel_keymap;

// (alkis)
extern tex_accent_struct get_accent(kb_action action);


LyXFunc::LyXFunc(LyXView * lv)
	: owner(lv),
	encoded_last_key(0),
	keyseq(toplevel_keymap.get(), toplevel_keymap.get()),
	cancel_meta_seq(toplevel_keymap.get(), toplevel_keymap.get()),
	meta_fake_bit(key_modifier::none)
{
}


void LyXFunc::handleKeyFunc(kb_action action)
{
	char c = encoded_last_key;

	if (keyseq.length()) {
		c = 0;
	}

	owner->getIntl().getTransManager()
		.deadkey(c, get_accent(action).accent, view()->getLyXText());
	// Need to clear, in case the minibuffer calls these
	// actions
	keyseq.clear();
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

	Encoding const * encoding = view()->cursor().getEncoding();

	encoded_last_key = keysym->getISOEncoded(encoding ? encoding->Name() : "");

	// Do a one-deep top-level lookup for
	// cancel and meta-fake keys. RVDK_PATCH_5
	cancel_meta_seq.reset();

	FuncRequest func = cancel_meta_seq.addkey(keysym, state);
	lyxerr[Debug::KEY] << "action first set to [" << func.action << ']' << endl;

	// When not cancel or meta-fake, do the normal lookup.
	// Note how the meta_fake Mod1 bit is OR-ed in and reset afterwards.
	// Mostly, meta_fake_bit = key_modifier::none. RVDK_PATCH_5.
	if ((func.action != LFUN_CANCEL) && (func.action != LFUN_META_FAKE)) {
		// remove Caps Lock and Mod2 as a modifiers
		func = keyseq.addkey(keysym, (state | meta_fake_bit));
		lyxerr[Debug::KEY] << "action now set to ["
			<< func.action << ']' << endl;
	}

	// Dont remove this unless you know what you are doing.
	meta_fake_bit = key_modifier::none;

	// can this happen now ?
	if (func.action == LFUN_NOACTION) {
		func = FuncRequest(LFUN_PREFIX);
	}

	if (lyxerr.debugging(Debug::KEY)) {
		lyxerr << "Key [action="
		       << func.action << "]["
		       << keyseq.print() << ']'
		       << endl;
	}

	// already here we know if it any point in going further
	// why not return already here if action == -1 and
	// num_bytes == 0? (Lgb)

	if (keyseq.length() > 1) {
		owner->message(keyseq.print());
	}


	// Maybe user can only reach the key via holding down shift.
	// Let's see. But only if shift is the only modifier
	if (func.action == LFUN_UNKNOWN_ACTION &&
	    state == key_modifier::shift) {
		lyxerr[Debug::KEY] << "Trying without shift" << endl;
		func = keyseq.addkey(keysym, key_modifier::none);
		lyxerr[Debug::KEY] << "Action now " << func.action << endl;
	}

	if (func.action == LFUN_UNKNOWN_ACTION) {
		// Hmm, we didn't match any of the keysequences. See
		// if it's normal insertable text not already covered
		// by a binding
		if (keysym->isText() && keyseq.length() == 1) {
			lyxerr[Debug::KEY] << "isText() is true, inserting." << endl;
			func = FuncRequest(LFUN_SELFINSERT);
		} else {
			lyxerr[Debug::KEY] << "Unknown, !isText() - giving up" << endl;
			owner->message(_("Unknown function."));
			return;
		}
	}

	if (func.action == LFUN_SELFINSERT) {
		if (encoded_last_key != 0) {
			string arg(1, encoded_last_key);
			dispatch(FuncRequest(LFUN_SELFINSERT, arg));
			lyxerr[Debug::KEY]
				<< "SelfInsert arg[`" << arg << "']" << endl;
		}
	} else {
		dispatch(func);
	}
}


FuncStatus LyXFunc::getStatus(FuncRequest const & cmd) const
{
	//lyxerr << "LyXFunc::getStatus: cmd: " << cmd << endl;
	FuncStatus flag;
	Buffer * buf = owner->buffer();
	LCursor & cur = view()->cursor();

	if (cmd.action == LFUN_NOACTION) {
		setStatusMessage(N_("Nothing to do"));
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
		flag |= lyx_gui::getStatus(cmd);
	}

	if (flag.unknown()) {
		setStatusMessage(N_("Unknown action"));
		return flag;
	}

	// the default error message if we disable the command
	setStatusMessage(N_("Command disabled"));

	// Check whether we need a buffer
	if (!lyxaction.funcHasFlag(cmd.action, LyXAction::NoBuffer) && !buf) {
		// no, exit directly
		setStatusMessage(N_("Command not allowed with"
				    "out any document open"));
		flag.enabled(false);
		return flag;
	}

	// I would really like to avoid having this switch and rather try to
	// encode this in the function itself.
	// -- And I'd rather let an inset decide which LFUNs it is willing
	// to handle (Andre')
	bool enable = true;
	switch (cmd.action) {
	case LFUN_TOOLTIPS_TOGGLE:
		flag.setOnOff(owner->getDialogs().tooltipsEnabled());
		break;

	case LFUN_READ_ONLY_TOGGLE:
		flag.setOnOff(buf->isReadonly());
		break;

	case LFUN_SWITCHBUFFER:
		// toggle on the current buffer, but do not toggle off
		// the other ones (is that a good idea?)
		if (cmd.argument == buf->fileName())
			flag.setOnOff(true);
		break;

	case LFUN_TRACK_CHANGES:
		flag.setOnOff(buf->params().tracking_changes);
		break;

	case LFUN_EXPORT:
		enable = cmd.argument == "custom"
			|| Exporter::IsExportable(*buf, cmd.argument);
		break;
	case LFUN_UNDO:
		enable = !buf->undostack().empty();
		break;
	case LFUN_REDO:
		enable = !buf->redostack().empty();
		break;
	case LFUN_CUT:
	case LFUN_COPY:
		enable = cur.selection();
		break;

	case LFUN_RUNCHKTEX:
		enable = buf->isLatex() && lyxrc.chktex_command != "none";
		break;

	case LFUN_BUILDPROG:
		enable = Exporter::IsExportable(*buf, "program");
		break;

	case LFUN_LAYOUT_TABULAR:
		enable = cur.innerInsetOfType(InsetBase::TABULAR_CODE);
		break;

	case LFUN_LAYOUT:
	case LFUN_LAYOUT_PARAGRAPH:
		enable = !cur.inset().forceDefaultParagraphs(&cur.inset());
		break;

	case LFUN_VC_REGISTER:
		enable = !buf->lyxvc().inUse();
		break;
	case LFUN_VC_CHECKIN:
		enable = buf->lyxvc().inUse() && !buf->isReadonly();
		break;
	case LFUN_VC_CHECKOUT:
		enable = buf->lyxvc().inUse() && buf->isReadonly();
		break;
	case LFUN_VC_REVERT:
	case LFUN_VC_UNDO:
		enable = buf->lyxvc().inUse();
		break;
	case LFUN_MENURELOAD:
		enable = !buf->isUnnamed() && !buf->isClean();
		break;
	case LFUN_BOOKMARK_GOTO:
		enable = view()->isSavedPosition(strToUnsignedInt(cmd.argument));
		break;

	case LFUN_MERGE_CHANGES:
	case LFUN_ACCEPT_CHANGE:
	case LFUN_REJECT_CHANGE:
	case LFUN_ACCEPT_ALL_CHANGES:
	case LFUN_REJECT_ALL_CHANGES:
		enable = buf && buf->params().tracking_changes;
		break;

	case LFUN_INSET_SETTINGS: {
		enable = false;
		if (!cur.size())
			break;
		UpdatableInset * inset = cur.inset().asUpdatableInset();
		lyxerr << "inset: " << inset << endl;
		if (!inset)
			break;

		InsetOld::Code code = inset->lyxCode();
		switch (code) {
			case InsetOld::TABULAR_CODE:
				enable = cmd.argument == "tabular";
				break;
			case InsetOld::ERT_CODE:
				enable = cmd.argument == "ert";
				break;
			case InsetOld::FLOAT_CODE:
				enable = cmd.argument == "float";
				break;
			case InsetOld::WRAP_CODE:
				enable = cmd.argument == "wrap";
				break;
			case InsetOld::NOTE_CODE:
				enable = cmd.argument == "note";
				break;
			case InsetOld::BRANCH_CODE:
				enable = cmd.argument == "branch";
				break;
			case InsetOld::BOX_CODE:
				enable = cmd.argument == "box";
				break;
			default:
				break;
		}
		break;
	}

	case LFUN_DIALOG_SHOW: {
		string const name = cmd.getArg(0);
		if (!buf)
			enable = name == "aboutlyx"
				|| name == "file"
				|| name == "forks"
				|| name == "preferences"
				|| name == "texinfo";
		else if (name == "print")
			enable = Exporter::IsExportable(*buf, "dvi")
				&& lyxrc.print_command != "none";
		else if (name == "character")
			enable = cur.inset().lyxCode() != InsetOld::ERT_CODE;
		else if (name == "vclog")
			enable = buf->lyxvc().inUse();
		else if (name == "latexlog")
			enable = IsFileReadable(buf->getLogName().second);
		break;
	}

	case LFUN_MENUNEW:
	case LFUN_MENUNEWTMPLT:
	case LFUN_WORDFINDFORWARD:
	case LFUN_WORDFINDBACKWARD:
	case LFUN_PREFIX:
	case LFUN_EXEC_COMMAND:
	case LFUN_CANCEL:
	case LFUN_META_FAKE:
	case LFUN_CLOSEBUFFER:
	case LFUN_MENUWRITE:
	case LFUN_WRITEAS:
	case LFUN_UPDATE:
	case LFUN_PREVIEW:
	case LFUN_IMPORT:
	case LFUN_QUIT:
	case LFUN_TOCVIEW:
	case LFUN_AUTOSAVE:
	case LFUN_RECONFIGURE:
	case LFUN_HELP_OPEN:
	case LFUN_FILE_NEW:
	case LFUN_FILE_OPEN:
	case LFUN_DROP_LAYOUTS_CHOICE:
	case LFUN_MENU_OPEN_BY_NAME:
	case LFUN_GETNAME:
	case LFUN_NOTIFY:
	case LFUN_GOTOFILEROW:
	case LFUN_GOTO_PARAGRAPH:
	case LFUN_DIALOG_SHOW_NEW_INSET:
	case LFUN_DIALOG_SHOW_NEXT_INSET:
	case LFUN_DIALOG_UPDATE:
	case LFUN_DIALOG_HIDE:
	case LFUN_DIALOG_DISCONNECT_INSET:
	case LFUN_CHILDOPEN:
	case LFUN_TOGGLECURSORFOLLOW:
	case LFUN_KMAP_OFF:
	case LFUN_KMAP_PRIM:
	case LFUN_KMAP_SEC:
	case LFUN_KMAP_TOGGLE:
	case LFUN_REPEAT:
	case LFUN_EXPORT_CUSTOM:
	case LFUN_PRINT:
	case LFUN_SEQUENCE:
	case LFUN_SAVEPREFERENCES:
	case LFUN_SCREEN_FONT_UPDATE:
	case LFUN_SET_COLOR:
	case LFUN_MESSAGE:
	case LFUN_EXTERNAL_EDIT:
	case LFUN_FILE_INSERT:
	case LFUN_FILE_INSERT_ASCII:
	case LFUN_FILE_INSERT_ASCII_PARA:
		// these are handled in our dispatch()
		break;

	default:
		cur.getStatus(cmd, flag);
	}

	if (!enable)
		flag.enabled(false);

	// Can we use a readonly buffer?
	if (buf && buf->isReadonly() 
	    && !lyxaction.funcHasFlag(cmd.action, LyXAction::ReadOnly)
	    && !lyxaction.funcHasFlag(cmd.action, LyXAction::NoBuffer)) {
		setStatusMessage(N_("Document is read-only"));
		flag.enabled(false);
	}

	return flag;
}


namespace {

bool ensureBufferClean(BufferView * bv)
{
	Buffer & buf = *bv->buffer();
	if (buf.isClean())
		return true;

	string const file = MakeDisplayPath(buf.fileName(), 30);
	string text = bformat(_("The document %1$s has unsaved "
				"changes.\n\nDo you want to save "
				"the document?"), file);
	int const ret = Alert::prompt(_("Save changed document?"),
				      text, 0, 1, _("&Save"),
				      _("&Cancel"));

	if (ret == 0)
		bv->owner()->dispatch(FuncRequest(LFUN_MENUWRITE));

	return buf.isClean();
}

void showPrintError(string const & name)
{
		string str = bformat(_("Could not print the document %1$s.\n"
			"Check that your printer is set up correctly."),
			MakeDisplayPath(name, 50));
		Alert::error(_("Print document failed"), str);
}

} //namespace anon


void LyXFunc::dispatch(FuncRequest const & cmd, bool verbose)
{
	string const argument = cmd.argument;
	kb_action const action = cmd.action;

	lyxerr[Debug::ACTION] << "LyXFunc::dispatch: cmd: " << cmd << endl;
	//lyxerr << "LyXFunc::dispatch: cmd: " << cmd << endl;

	// we have not done anything wrong yet.
	errorstat = false;
	dispatch_buffer.erase();
	selection_possible = false;

	// We cannot use this function here
	if (!getStatus(cmd).enabled()) {
		lyxerr[Debug::ACTION] << "LyXFunc::dispatch: "
		       << lyxaction.getActionName(action)
		       << " [" << action << "] is disabled at this location"
		       << endl;
		setErrorMessage(getStatusMessage());

	} else {

		if (view()->available())
			view()->hideCursor();

		switch (action) {

		case LFUN_WORDFINDFORWARD:
		case LFUN_WORDFINDBACKWARD: {
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

			bool const fw = action == LFUN_WORDFINDFORWARD;
			string const data =
				lyx::find::find2string(searched_string, true, false, fw);
			lyx::find::find(view(), FuncRequest(LFUN_WORD_FIND, data));
			break;
		}

		case LFUN_PREFIX:
			owner->message(keyseq.printOptions());
			break;

		case LFUN_EXEC_COMMAND:
			owner->focus_command_buffer();
			break;

		case LFUN_CANCEL:
			keyseq.reset();
			meta_fake_bit = key_modifier::none;
			if (view()->available())
				// cancel any selection
				dispatch(FuncRequest(LFUN_MARK_OFF));
			setMessage(N_("Cancel"));
			break;

		case LFUN_META_FAKE:
			meta_fake_bit = key_modifier::alt;
			setMessage(keyseq.print());
			break;

		case LFUN_READ_ONLY_TOGGLE:
			if (owner->buffer()->lyxvc().inUse())
				owner->buffer()->lyxvc().toggleReadOnly();
			else
				owner->buffer()->setReadonly(
					!owner->buffer()->isReadonly());
			break;

		// --- Menus -----------------------------------------------
		case LFUN_MENUNEW:
			menuNew(argument, false);
			break;

		case LFUN_MENUNEWTMPLT:
			menuNew(argument, true);
			break;

		case LFUN_CLOSEBUFFER:
			closeBuffer();
			break;

		case LFUN_MENUWRITE:
			if (!owner->buffer()->isUnnamed()) {
				string const str = bformat(_("Saving document %1$s..."),
					 MakeDisplayPath(owner->buffer()->fileName()));
				owner->message(str);
				MenuWrite(owner->buffer());
				owner->message(str + _(" done."));
			} else
				WriteAs(owner->buffer());
			break;

		case LFUN_WRITEAS:
			WriteAs(owner->buffer(), argument);
			break;

		case LFUN_MENURELOAD: {
			string const file = MakeDisplayPath(view()->buffer()->fileName(), 20);
			string text = bformat(_("Any changes will be lost. Are you sure "
				"you want to revert to the saved version of the document %1$s?"), file);
			int const ret = Alert::prompt(_("Revert to saved document?"),
				text, 0, 1, _("&Revert"), _("&Cancel"));

			if (ret == 0)
				view()->reload();
			break;
		}

		case LFUN_UPDATE:
			Exporter::Export(owner->buffer(), argument, true);
			view()->showErrorList(BufferFormat(*owner->buffer()));
			break;

		case LFUN_PREVIEW:
			Exporter::Preview(owner->buffer(), argument);
			view()->showErrorList(BufferFormat(*owner->buffer()));
			break;

		case LFUN_BUILDPROG:
			Exporter::Export(owner->buffer(), "program", true);
			view()->showErrorList(_("Build"));
			break;

		case LFUN_RUNCHKTEX:
			owner->buffer()->runChktex();
			view()->showErrorList(_("ChkTeX"));
			break;

		case LFUN_EXPORT:
			if (argument == "custom")
				owner->getDialogs().show("sendto");
			else {
				Exporter::Export(owner->buffer(), argument, false);
				view()->showErrorList(BufferFormat(*owner->buffer()));
			}
			break;

		case LFUN_EXPORT_CUSTOM: {
			string format_name;
			string command = split(argument, format_name, ' ');
			Format const * format = formats.getFormat(format_name);
			if (!format) {
				lyxerr << "Format \"" << format_name
				       << "\" not recognized!"
				       << std::endl;
				break;
			}

			Buffer * buffer = owner->buffer();

			// The name of the file created by the conversion process
			string filename;

			// Output to filename
			if (format->name() == "lyx") {
				string const latexname =
					buffer->getLatexName(false);
				filename = ChangeExtension(latexname,
							   format->extension());
				filename = AddName(buffer->temppath(), filename);

				if (!buffer->writeFile(filename))
					break;

			} else {
				Exporter::Export(buffer, format_name, true,
						 filename);
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

		case LFUN_PRINT: {
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

			Buffer * buffer = owner->buffer();

			if (!Exporter::Export(buffer, "dvi", true)) {
				showPrintError(buffer->fileName());
				break;
			}

			// Push directory path.
			string const path = buffer->temppath();
			Path p(path);

			// there are three cases here:
			// 1. we print to a file
			// 2. we print directly to a printer
			// 3. we print using a spool command (print to file first)
			Systemcall one;
			int res = 0;
			string const dviname =
				ChangeExtension(buffer->getLatexName(true),
						"dvi");

			if (target == "printer") {
				if (!lyxrc.print_spool_command.empty()) {
					// case 3: print using a spool
					string const psname =
						ChangeExtension(dviname,".ps");
					command += lyxrc.print_to_file
						+ QuoteName(psname)
						+ ' '
						+ QuoteName(dviname);

					string command2 =
						lyxrc.print_spool_command +' ';
					if (target_name != "default") {
						command2 += lyxrc.print_spool_printerprefix
							+ target_name
							+ ' ';
					}
					command2 += QuoteName(psname);
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
						command + QuoteName(dviname));
				}
		
			} else {
				// case 1: print to a file
				command += lyxrc.print_to_file
					+ QuoteName(MakeAbsPath(target_name,
								path))
					+ ' '
					+ QuoteName(dviname);
				res = one.startscript(Systemcall::DontWait,
						      command);
			}

			if (res != 0)
				showPrintError(buffer->fileName());
			break;
		}

		case LFUN_IMPORT:
			doImport(argument);
			break;

		case LFUN_QUIT:
			QuitLyX();
			break;

		case LFUN_TOCVIEW: {
			InsetCommandParams p("tableofcontents");
			string const data = InsetCommandMailer::params2string("toc", p);
			owner->getDialogs().show("toc", data, 0);
			break;
		}

		case LFUN_AUTOSAVE:
			AutoSave(view());
			break;

		case LFUN_RECONFIGURE:
			Reconfigure(view());
			break;

		case LFUN_HELP_OPEN: {
			string const arg = argument;
			if (arg.empty()) {
				setErrorMessage(N_("Missing argument"));
				break;
			}
			string const fname = i18nLibFileSearch("doc", arg, "lyx");
			if (fname.empty()) {
				lyxerr << "LyX: unable to find documentation file `"
							 << arg << "'. Bad installation?" << endl;
				break;
			}
			owner->message(bformat(_("Opening help file %1$s..."),
				MakeDisplayPath(fname)));
			view()->loadLyXFile(fname, false);
			break;
		}

		// --- version control -------------------------------
		case LFUN_VC_REGISTER:
			if (!ensureBufferClean(view()))
				break;
			if (!owner->buffer()->lyxvc().inUse()) {
				owner->buffer()->lyxvc().registrer();
				view()->reload();
			}
			break;

		case LFUN_VC_CHECKIN:
			if (!ensureBufferClean(view()))
				break;
			if (owner->buffer()->lyxvc().inUse()
					&& !owner->buffer()->isReadonly()) {
				owner->buffer()->lyxvc().checkIn();
				view()->reload();
			}
			break;

		case LFUN_VC_CHECKOUT:
			if (!ensureBufferClean(view()))
				break;
			if (owner->buffer()->lyxvc().inUse()
					&& owner->buffer()->isReadonly()) {
				owner->buffer()->lyxvc().checkOut();
				view()->reload();
			}
			break;

		case LFUN_VC_REVERT:
			owner->buffer()->lyxvc().revert();
			view()->reload();
			break;

		case LFUN_VC_UNDO:
			owner->buffer()->lyxvc().undoLast();
			view()->reload();
			break;

		// --- buffers ----------------------------------------
		case LFUN_SWITCHBUFFER:
			view()->setBuffer(bufferlist.getBuffer(argument));
			break;

		case LFUN_FILE_NEW:
			NewFile(view(), argument);
			break;

		case LFUN_FILE_OPEN:
			open(argument);
			break;

		case LFUN_DROP_LAYOUTS_CHOICE:
			owner->getToolbar().openLayoutList();
			break;

		case LFUN_MENU_OPEN_BY_NAME:
			owner->getMenubar().openByName(argument);
			break;

		// --- lyxserver commands ----------------------------
		case LFUN_GETNAME:
			setMessage(owner->buffer()->fileName());
			lyxerr[Debug::INFO] << "FNAME["
							 << owner->buffer()->fileName()
							 << "] " << endl;
			break;

		case LFUN_NOTIFY:
			dispatch_buffer = keyseq.print();
			lyxserver->notifyClient(dispatch_buffer);
			break;

		case LFUN_GOTOFILEROW: {
			string file_name;
			int row;
			istringstream is(argument);
			is >> file_name >> row;
			if (prefixIs(file_name, getTmpDir())) {
				// Needed by inverse dvi search. If it is a file
				// in tmpdir, call the apropriated function
				view()->setBuffer(bufferlist.getBufferFromTmp(file_name));
			} else {
				// Must replace extension of the file to be .lyx
				// and get full path
				string const s = ChangeExtension(file_name, ".lyx");
				// Either change buffer or load the file
				if (bufferlist.exists(s)) {
					view()->setBuffer(bufferlist.getBuffer(s));
				} else {
					view()->loadLyXFile(s);
				}
			}

			view()->setCursorFromRow(row);

			view()->center();
			// see BufferView_pimpl::center()
			view()->updateScrollbar();
			break;
		}

		case LFUN_GOTO_PARAGRAPH: {
			istringstream is(argument);
			int id;
			is >> id;
			ParIterator par = owner->buffer()->getParFromID(id);
			if (par == owner->buffer()->par_iterator_end()) {
				lyxerr[Debug::INFO] << "No matching paragraph found! ["
								<< id << ']' << endl;
				break;
			} else {
				lyxerr[Debug::INFO] << "Paragraph " << par->id()
								<< " found." << endl;
			}

			// Set the cursor
			view()->setCursor(par, 0);

			view()->switchKeyMap();
			owner->view_state_changed();

			view()->center();
			// see BufferView_pimpl::center()
			view()->updateScrollbar();
			break;
		}

		case LFUN_DIALOG_SHOW: {
			string const name = cmd.getArg(0);
			string data = trim(cmd.argument.substr(name.size()));

			if (name == "character") {
				data = freefont2string();
				if (!data.empty())
					owner->getDialogs().show("character", data);
			}
			else if (name == "document")
				owner->getDialogs().showDocument();
			else if (name == "preamble")
				owner->getDialogs().showPreamble();
			else if (name == "preferences")
				owner->getDialogs().showPreferences();
			else if (name == "spellchecker")
				owner->getDialogs().showSpellchecker();

			else if (name == "latexlog") {
				pair<Buffer::LogType, string> const logfile =
					owner->buffer()->getLogName();
				switch (logfile.first) {
				case Buffer::latexlog:
					data = "latex ";
					break;
				case Buffer::buildlog:
					data = "literate ";
					break;
				}
				data += logfile.second;
				owner->getDialogs().show("log", data);
			}
			else if (name == "vclog") {
				string const data = "vc " +
					owner->buffer()->lyxvc().getLogFile();
				owner->getDialogs().show("log", data);
			}
			else
				owner->getDialogs().show(name, data);
			break;
		}

		case LFUN_DIALOG_SHOW_NEW_INSET: {
			string const name = cmd.getArg(0);
			string data = trim(cmd.argument.substr(name.size()));
			if (name == "bibitem" ||
			    name == "bibtex" ||
			    name == "include" ||
			    name == "index" ||
			    name == "label" ||
			    name == "ref" ||
			    name == "toc" ||
			    name == "url") {
				InsetCommandParams p(name);
				data = InsetCommandMailer::params2string(name, p);
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
				Buffer const & buffer = *owner->buffer();
				data = InsetExternalMailer::params2string(p, buffer);
			} else if (name == "float") {
				InsetFloatParams p;
				data = InsetFloatMailer::params2string(p);
			} else if (name == "graphics") {
				InsetGraphicsParams p;
				Buffer const & buffer = *owner->buffer();
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
			owner->getDialogs().show(name, data, 0);
			break;
		}

		case LFUN_DIALOG_SHOW_NEXT_INSET:
			break;

		case LFUN_DIALOG_UPDATE: {
			string const & name = argument;
			// Can only update a dialog connected to an existing inset
			InsetBase * inset = owner->getDialogs().getOpenInset(name);
			if (inset) {
				FuncRequest fr(LFUN_INSET_DIALOG_UPDATE, cmd.argument);
				inset->dispatch(view()->cursor(), fr);
			} else if (name == "paragraph") {
				dispatch(FuncRequest(LFUN_PARAGRAPH_UPDATE));
			}
			break;
		}

		case LFUN_DIALOG_HIDE:
			Dialogs::hide(argument, 0);
			break;

		case LFUN_DIALOG_DISCONNECT_INSET:
			owner->getDialogs().disconnect(argument);
			break;

		case LFUN_CHILDOPEN: {
			string const filename =
				MakeAbsPath(argument, owner->buffer()->filePath());
			setMessage(N_("Opening child document ") +
					 MakeDisplayPath(filename) + "...");
			view()->savePosition(0);
			string const parentfilename = owner->buffer()->fileName();
			if (bufferlist.exists(filename))
				view()->setBuffer(bufferlist.getBuffer(filename));
			else
				view()->loadLyXFile(filename);
			// Set the parent name of the child document.
			// This makes insertion of citations and references in the child work,
			// when the target is in the parent or another child document.
			owner->buffer()->setParentName(parentfilename);
			break;
		}

		case LFUN_TOGGLECURSORFOLLOW:
			lyxrc.cursor_follows_scrollbar = !lyxrc.cursor_follows_scrollbar;
			break;

		case LFUN_KMAP_OFF:
			owner->getIntl().KeyMapOn(false);
			break;

		case LFUN_KMAP_PRIM:
			owner->getIntl().KeyMapPrim();
			break;

		case LFUN_KMAP_SEC:
			owner->getIntl().KeyMapSec();
			break;

		case LFUN_KMAP_TOGGLE:
			owner->getIntl().ToggleKeyMap();
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

		case LFUN_SEQUENCE:
			// argument contains ';'-terminated commands
			while (!argument.empty()) {
				string first;
				string rest = split(argument, first, ';');
				dispatch(lyxaction.lookupFunc(rest));
			}
			break;

		case LFUN_SAVEPREFERENCES: {
			Path p(user_lyxdir());
			lyxrc.write("preferences");
			break;
		}

		case LFUN_SCREEN_FONT_UPDATE:
			// handle the screen font changes.
			lyxrc.set_font_norm_type();
			lyx_gui::update_fonts();
			// All visible buffers will need resize
			view()->resize();
			break;

		case LFUN_SET_COLOR: {
			string lyx_name;
			string const x11_name = split(argument, lyx_name, ' ');
			if (lyx_name.empty() || x11_name.empty()) {
				setErrorMessage(N_("Syntax: set-color <lyx_name>"
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
								"may not be redefined"), lyx_name));
				break;
			}

			lyx_gui::update_color(lcolor.getFromLyXName(lyx_name));

			if (graphicsbg_changed) {
#ifdef WITH_WARNINGS
#warning FIXME!! The graphics cache no longer has a changeDisplay method.
#endif
#if 0
				lyx::graphics::GCache::get().changeDisplay(true);
#endif
			}
			break;
		}

		case LFUN_MESSAGE:
			owner->message(argument);
			break;

		case LFUN_TOOLTIPS_TOGGLE:
			owner->getDialogs().toggleTooltips();
			break;

		case LFUN_EXTERNAL_EDIT: {
			FuncRequest fr(action, argument);
			InsetExternal().dispatch(view()->cursor(), fr);
			break;
		}

		case LFUN_BREAKLINE: {
#warning swallow 'Return' if the minibuffer is focused. But how?
		}

		default: {
			DispatchResult res = view()->cursor().dispatch(cmd);
			if (!res.dispatched());
				view()->dispatch(cmd);
			break;
		}
		}

		if (view()->available()) {
			view()->fitCursor();
			view()->update();
			view()->cursor().updatePos();
			// if we executed a mutating lfun, mark the buffer as dirty
			if (getStatus(cmd).enabled()
					&& !lyxaction.funcHasFlag(cmd.action, LyXAction::NoBuffer)
					&& !lyxaction.funcHasFlag(cmd.action, LyXAction::ReadOnly))
				view()->buffer()->markDirty();
		}

		if (view()->cursor().inTexted()) {
			view()->owner()->updateLayoutChoice();
			sendDispatchMessage(getMessage(), cmd, verbose);
		}
	}
}


void LyXFunc::sendDispatchMessage(string const & msg,
				  FuncRequest const & cmd, bool verbose)
{
	owner->updateMenubar();
	owner->updateToolbar();

	if (cmd.action == LFUN_SELFINSERT || !verbose) {
		lyxerr[Debug::ACTION] << "dispatch msg is " << msg << endl;
		if (!msg.empty())
			owner->message(msg);
		return;
	}

	string dispatch_msg = msg;
	if (!dispatch_msg.empty())
		dispatch_msg += ' ';

	string comname = lyxaction.getActionName(cmd.action);

	bool argsadded = false;

	if (!cmd.argument.empty()) {
		if (cmd.action != LFUN_UNKNOWN_ACTION) {
			comname += ' ' + cmd.argument;
			argsadded = true;
		}
	}

	string const shortcuts = toplevel_keymap->findbinding(cmd);

	if (!shortcuts.empty()) {
		comname += ": " + shortcuts;
	} else if (!argsadded && !cmd.argument.empty()) {
		comname += ' ' + cmd.argument;
	}

	if (!comname.empty()) {
		comname = rtrim(comname);
		dispatch_msg += '(' + comname + ')';
	}

	lyxerr[Debug::ACTION] << "verbose dispatch msg " << dispatch_msg << endl;
	if (!dispatch_msg.empty())
		owner->message(dispatch_msg);
}


void LyXFunc::setupLocalKeymap()
{
	keyseq.stdmap = toplevel_keymap.get();
	keyseq.curmap = toplevel_keymap.get();
	cancel_meta_seq.stdmap = toplevel_keymap.get();
	cancel_meta_seq.curmap = toplevel_keymap.get();
}


void LyXFunc::menuNew(string const & name, bool fromTemplate)
{
	string initpath = lyxrc.document_path;
	string filename(name);

	if (view()->available()) {
		string const trypath = owner->buffer()->filePath();
		// If directory is writeable, use this as default.
		if (IsDirWriteable(trypath))
			initpath = trypath;
	}

	static int newfile_number;

	if (filename.empty()) {
		filename = AddName(lyxrc.document_path,
			    "newfile" + tostr(++newfile_number) + ".lyx");
		FileInfo fi(filename);
		while (bufferlist.exists(filename) || fi.readable()) {
			++newfile_number;
			filename = AddName(lyxrc.document_path,
				    "newfile" +	tostr(newfile_number) +
				    ".lyx");
			fi.newFile(filename);
		}
	}

	// The template stuff
	string templname;
	if (fromTemplate) {
		FileDialog fileDlg(_("Select template file"),
			LFUN_SELECT_FILE_SYNC,
			make_pair(string(_("Documents|#o#O")),
				  string(lyxrc.document_path)),
			make_pair(string(_("Templates|#T#t")),
				  string(lyxrc.template_path)));

		FileDialog::Result result =
			fileDlg.open(lyxrc.template_path,
				     FileFilterList(_("LyX Documents (*.lyx)")),
				     string());

		if (result.first == FileDialog::Later)
			return;
		if (result.second.empty())
			return;
		templname = result.second;
	}

	view()->newFile(filename, templname, !name.empty());
}


void LyXFunc::open(string const & fname)
{
	string initpath = lyxrc.document_path;

	if (view()->available()) {
		string const trypath = owner->buffer()->filePath();
		// If directory is writeable, use this as default.
		if (IsDirWriteable(trypath))
			initpath = trypath;
	}

	string filename;

	if (fname.empty()) {
		FileDialog fileDlg(_("Select document to open"),
			LFUN_FILE_OPEN,
			make_pair(string(_("Documents|#o#O")),
				  string(lyxrc.document_path)),
			make_pair(string(_("Examples|#E#e")),
				  string(AddPath(system_lyxdir(), "examples"))));

		FileDialog::Result result =
			fileDlg.open(initpath,
				     FileFilterList(_("LyX Documents (*.lyx)")),
				     string());

		if (result.first == FileDialog::Later)
			return;

		filename = result.second;

		// check selected filename
		if (filename.empty()) {
			owner->message(_("Canceled."));
			return;
		}
	} else
		filename = fname;

	// get absolute path of file and add ".lyx" to the filename if
	// necessary
	string const fullpath = FileSearch(string(), filename, "lyx");
	if (!fullpath.empty()) {
		filename = fullpath;
	}

	string const disp_fn(MakeDisplayPath(filename));

	// if the file doesn't exist, let the user create one
	FileInfo const f(filename, true);
	if (!f.exist()) {
		// the user specifically chose this name. Believe them.
		view()->newFile(filename, "", true);
		return;
	}

	owner->message(bformat(_("Opening document %1$s..."), disp_fn));

	string str2;
	if (view()->loadLyXFile(filename)) {
		str2 = bformat(_("Document %1$s opened."), disp_fn);
	} else {
		str2 = bformat(_("Could not open document %1$s"), disp_fn);
	}
	owner->message(str2);
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

		if (view()->available()) {
			string const trypath = owner->buffer()->filePath();
			// If directory is writeable, use this as default.
			if (IsDirWriteable(trypath))
				initpath = trypath;
		}

		string const text = bformat(_("Select %1$s file to import"),
			formats.prettyName(format));

		FileDialog fileDlg(text,
			LFUN_IMPORT,
			make_pair(string(_("Documents|#o#O")),
				  string(lyxrc.document_path)),
			make_pair(string(_("Examples|#E#e")),
				  string(AddPath(system_lyxdir(), "examples"))));

		string const filter = formats.prettyName(format)
			+ " (*." + formats.extension(format) + ')';

		FileDialog::Result result =
			fileDlg.open(initpath,
				     FileFilterList(filter),
				     string());

		if (result.first == FileDialog::Later)
			return;

		filename = result.second;

		// check selected filename
		if (filename.empty())
			owner->message(_("Canceled."));
	}

	if (filename.empty())
		return;

	// get absolute path of file
	filename = MakeAbsPath(filename);

	string const lyxfile = ChangeExtension(filename, ".lyx");

	// Check if the document already is open
	if (lyx_gui::use_gui && bufferlist.exists(lyxfile)) {
		if (!bufferlist.close(bufferlist.getBuffer(lyxfile), true)) {
			owner->message(_("Canceled."));
			return;
		}
	}

	// if the file exists already, and we didn't do
	// -i lyx thefile.lyx, warn
	if (FileInfo(lyxfile, true).exist() && filename != lyxfile) {
		string const file = MakeDisplayPath(lyxfile, 30);

		string text = bformat(_("The document %1$s already exists.\n\n"
			"Do you want to over-write that document?"), file);
		int const ret = Alert::prompt(_("Over-write document?"),
			text, 0, 1, _("&Over-write"), _("&Cancel"));

		if (ret == 1) {
			owner->message(_("Canceled."));
			return;
		}
	}

	Importer::Import(owner, filename, format);
}


void LyXFunc::closeBuffer()
{
	if (bufferlist.close(owner->buffer(), true) && !quitting) {
		if (bufferlist.empty()) {
			// need this otherwise SEGV may occur while
			// trying to set variables that don't exist
			// since there's no current buffer
			owner->getDialogs().hideBufferDependent();
		} else {
			view()->setBuffer(bufferlist.first());
		}
	}
}


// Each "owner" should have it's own message method. lyxview and
// the minibuffer would use the minibuffer, but lyxserver would
// send an ERROR signal to its client.  Alejandro 970603
// This function is bit problematic when it comes to NLS, to make the
// lyx servers client be language indepenent we must not translate
// strings sent to this func.
void LyXFunc::setErrorMessage(string const & m) const
{
	dispatch_buffer = m;
	errorstat = true;
}


void LyXFunc::setMessage(string const & m) const
{
	dispatch_buffer = m;
}


void LyXFunc::setStatusMessage(string const & m) const
{
	status_buffer = m;
}


string const LyXFunc::viewStatusMessage()
{
	// When meta-fake key is pressed, show the key sequence so far + "M-".
	if (wasMetaKey())
		return keyseq.print() + "M-";

	// Else, when a non-complete key sequence is pressed,
	// show the available options.
	if (keyseq.length() > 0 && !keyseq.deleted())
		return keyseq.printOptions();

	if (!view()->available())
		return _("Welcome to LyX!");

	return view()->cursor().currentState();
}


BufferView * LyXFunc::view() const
{
	BOOST_ASSERT(owner);
	return owner->view().get();
}


bool LyXFunc::wasMetaKey() const
{
	return (meta_fake_bit != key_modifier::none);
}
