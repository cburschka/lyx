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

#include "insets/insetcommand.h"
#include "insets/insetexternal.h"
#include "insets/insettabular.h"

#include "mathed/math_cursor.h"

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
#include "support/path.h"
#include "support/path_defines.h"
#include "support/tostr.h"
#include "support/std_sstream.h"
#include "support/os.h"

using bv_funcs::apply_freefont;
using bv_funcs::changeDepth;
using bv_funcs::currentState;
using bv_funcs::DEC_DEPTH;
using bv_funcs::freefont2string;
using bv_funcs::INC_DEPTH;
using bv_funcs::update_and_apply_freefont;

using lyx::support::AddName;
using lyx::support::AddPath;
using lyx::support::bformat;
using lyx::support::ChangeExtension;
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
using lyx::support::rtrim;
using lyx::support::split;
using lyx::support::strToInt;
using lyx::support::strToUnsignedInt;
using lyx::support::system_lyxdir;
using lyx::support::token;
using lyx::support::trim;
using lyx::support::user_lyxdir;
using lyx::support::prefixIs;
using lyx::support::os::getTmpDir;

using std::endl;
using std::make_pair;
using std::string;
using std::istringstream;


extern BufferList bufferlist;
extern LyXServer * lyxserver;
extern bool selection_possible;

extern boost::scoped_ptr<kb_keymap> toplevel_keymap;

// (alkis)
extern tex_accent_struct get_accent(kb_action action);

extern void ShowLatexLog();


LyXFunc::LyXFunc(LyXView * o)
	: owner(o),
	encoded_last_key(0),
	keyseq(toplevel_keymap.get(), toplevel_keymap.get()),
	cancel_meta_seq(toplevel_keymap.get(), toplevel_keymap.get()),
	meta_fake_bit(key_modifier::none)
{
}


void LyXFunc::moveCursorUpdate()
{
	LyXText * lt = view()->text;
	if (lt->selection.mark())
		lt->setSelection();
	view()->update();
	view()->switchKeyMap();
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
	view()->update();
	view()->getLyXText()->selection.cursor = view()->getLyXText()->cursor;
}


void LyXFunc::processKeySym(LyXKeySymPtr keysym,
			    key_modifier::state state)
{
	string argument;

	if (lyxerr.debugging(Debug::KEY)) {
		lyxerr << "KeySym is "
		       << keysym->getSymbolName()
		       << endl;
	}

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

	Encoding const * encoding = view()->getEncoding();

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
			string arg;
			arg += encoded_last_key;

			dispatch(FuncRequest(view(), LFUN_SELFINSERT, arg));

			lyxerr[Debug::KEY] << "SelfInsert arg[`"
				   << argument << "']" << endl;
		}
	} else {
		dispatch(func);
	}
}


FuncStatus LyXFunc::getStatus(FuncRequest const & ev) const
{
	FuncStatus flag;
	Buffer * buf = owner->buffer();

	if (ev.action == LFUN_NOACTION) {
		setStatusMessage(N_("Nothing to do"));
		flag.disabled(true);
		return flag;
	}

	switch (ev.action) {
	case LFUN_UNKNOWN_ACTION:
#ifndef HAVE_LIBAIKSAURUS
	case LFUN_THESAURUS_ENTRY:
#endif
		flag.unknown(true);
		flag.disabled(true);
		break;
	default:
		flag |= lyx_gui::getStatus(ev);
	}

	if (flag.unknown()) {
		setStatusMessage(N_("Unknown action"));
		return flag;
	}

	// the default error message if we disable the command
	setStatusMessage(N_("Command disabled"));

	// Check whether we need a buffer
	if (!lyxaction.funcHasFlag(ev.action, LyXAction::NoBuffer)) {
		// Yes we need a buffer, do we have one?
		if (buf) {
			// yes
			// Can we use a readonly buffer?
			if (buf->isReadonly() &&
			    !lyxaction.funcHasFlag(ev.action,
						   LyXAction::ReadOnly)) {
				// no
				setStatusMessage(N_("Document is read-only"));
				flag.disabled(true);
			}
		} else {
			// no
			setStatusMessage(N_("Command not allowed with"
					    "out any document open"));
			flag.disabled(true);
			return flag;
		}
	}

	UpdatableInset * tli = view()->theLockingInset();

	// I would really like to avoid having this switch and rather try to
	// encode this in the function itself.
	bool disable = false;
	switch (ev.action) {
	case LFUN_EXPORT:
		disable = ev.argument != "custom"
			&& !Exporter::IsExportable(*buf, ev.argument);
		break;
	case LFUN_UNDO:
		disable = buf->undostack().empty();
		break;
	case LFUN_REDO:
		disable = buf->redostack().empty();
		break;
	case LFUN_CUT:
	case LFUN_COPY:
		if (tli) {
			UpdatableInset * in = tli;
			if (in->lyxCode() != InsetOld::TABULAR_CODE) {
				in = tli->getFirstLockingInsetOfType(InsetOld::TABULAR_CODE);
			}
			if (in && static_cast<InsetTabular*>(in)->hasSelection()) {
				disable = false;
				break;
			}
		}
		disable = !mathcursor && !view()->getLyXText()->selection.set();
		break;
	case LFUN_RUNCHKTEX:
		disable = !buf->isLatex() || lyxrc.chktex_command == "none";
		break;
	case LFUN_BUILDPROG:
		disable = !Exporter::IsExportable(*buf, "program");
		break;

	case LFUN_LAYOUT_TABULAR:
		disable = !tli
			|| (tli->lyxCode() != InsetOld::TABULAR_CODE
			    && !tli->getFirstLockingInsetOfType(InsetOld::TABULAR_CODE));
		break;

	case LFUN_DEPTH_MIN:
		disable = !changeDepth(view(), view()->getLyXText(), DEC_DEPTH, true);
		break;

	case LFUN_DEPTH_PLUS:
		disable = !changeDepth(view(), view()->getLyXText(), INC_DEPTH, true);
		break;

	case LFUN_LAYOUT:
	case LFUN_LAYOUT_PARAGRAPH: {
		InsetOld * inset = view()->getLyXText()->cursorPar()->inInset();
		disable = inset && inset->forceDefaultParagraphs(inset);
		break;
	}

	case LFUN_INSET_OPTARG:
		disable = (view()->getLyXText()->cursorPar()->layout()->optionalargs == 0);
		break;

	case LFUN_TABULAR_FEATURE:
		if (mathcursor) {
#if 0
			// FIXME: check temporarily disabled
			// valign code
			char align = mathcursor->valign();
			if (align == '\0') {
				disable = true;
				break;
			}
			if (ev.argument.empty()) {
				flag.clear();
				break;
			}
			if (!contains("tcb", ev.argument[0])) {
				disable = true;
				break;
			}
			flag.setOnOff(ev.argument[0] == align);
		} else {
			disable = true;

			char align = mathcursor->halign();
			if (align == '\0') {
				disable = true;
				break;
			}
			if (ev.argument.empty()) {
				flag.clear();
				break;
			}
			if (!contains("lcr", ev.argument[0])) {
				disable = true;
				break;
			}
			flag.setOnOff(ev.argument[0] == align);
#endif

			disable = !mathcursor->halign();
			break;
		}

		if (tli) {
			FuncStatus ret;
			//ret.disabled(true);
			if (tli->lyxCode() == InsetOld::TABULAR_CODE) {
				ret = static_cast<InsetTabular *>(tli)
					->getStatus(ev.argument);
				flag |= ret;
				disable = false;
			} else if (tli->getFirstLockingInsetOfType(InsetOld::TABULAR_CODE)) {
				ret = static_cast<InsetTabular *>
					(tli->getFirstLockingInsetOfType(InsetOld::TABULAR_CODE))
					->getStatus(ev.argument);
				flag |= ret;
				disable = false;
			} else {
				disable = true;
			}
		} else {
			static InsetTabular inset(*buf, 1, 1);
			disable = true;
			FuncStatus ret = inset.getStatus(ev.argument);
			if (ret.onoff(true) || ret.onoff(false))
				flag.setOnOff(false);
		}
		break;

	case LFUN_VC_REGISTER:
		disable = buf->lyxvc().inUse();
		break;
	case LFUN_VC_CHECKIN:
		disable = !buf->lyxvc().inUse() || buf->isReadonly();
		break;
	case LFUN_VC_CHECKOUT:
		disable = !buf->lyxvc().inUse() || !buf->isReadonly();
		break;
	case LFUN_VC_REVERT:
	case LFUN_VC_UNDO:
		disable = !buf->lyxvc().inUse();
		break;
	case LFUN_MENURELOAD:
		disable = buf->isUnnamed() || buf->isClean();
		break;
	case LFUN_BOOKMARK_GOTO:
		disable = !view()->
			isSavedPosition(strToUnsignedInt(ev.argument));
		break;

	case LFUN_MERGE_CHANGES:
	case LFUN_ACCEPT_CHANGE:
	case LFUN_REJECT_CHANGE:
	case LFUN_ACCEPT_ALL_CHANGES:
	case LFUN_REJECT_ALL_CHANGES:
		disable = !buf->params().tracking_changes;
		break;

	case LFUN_INSET_TOGGLE: {
		LyXText * lt = view()->getLyXText();
		disable = !(isEditableInset(lt->getInset())
			    || (lt->inset_owner
				&& lt->inset_owner->owner()
				&& lt->inset_owner->owner()->isOpen()));
		break;
	}

	case LFUN_INSET_SETTINGS: {
		disable = true;
		UpdatableInset * inset = view()->theLockingInset();

		if (!inset)
			break;

		// get the innermost inset
		inset = inset->getLockingInset();

		// jump back to owner if an InsetText, so
		// we get back to the InsetTabular or whatever
		if (inset->lyxCode() == InsetOld::TEXT_CODE)
			inset = inset->owner();

		InsetOld::Code code = inset->lyxCode();
		switch (code) {
			case InsetOld::TABULAR_CODE:
				disable = ev.argument != "tabular";
				break;
			case InsetOld::ERT_CODE:
				disable = ev.argument != "ert";
				break;
			case InsetOld::FLOAT_CODE:
				disable = ev.argument != "float";
				break;
			case InsetOld::MINIPAGE_CODE:
				disable = ev.argument != "minipage";
				break;
			case InsetOld::WRAP_CODE:
				disable = ev.argument != "wrap";
				break;
			case InsetOld::NOTE_CODE:
				disable = ev.argument != "note";
				break;
			case InsetOld::BRANCH_CODE:
				disable = ev.argument != "branch";
				break;
			case InsetOld::BOX_CODE:
				disable = ev.argument != "box";
				break;
			default:
				break;
		}
		break;
	}

	case LFUN_MATH_MUTATE:
		if (mathcursor)
			//flag.setOnOff(mathcursor->formula()->hullType() == ev.argument);
			flag.setOnOff(false);
		else
			disable = true;
		break;

	// we just need to be in math mode to enable that
	case LFUN_MATH_SIZE:
	case LFUN_MATH_SPACE:
	case LFUN_MATH_LIMITS:
	case LFUN_MATH_NONUMBER:
	case LFUN_MATH_NUMBER:
	case LFUN_MATH_EXTERN:
		disable = !mathcursor;
		break;

	case LFUN_DIALOG_SHOW: {
		string const name = ev.getArg(0);
		if (!buf) {
			disable = !(name == "aboutlyx" ||
				    name == "file" ||
				    name == "forks" ||
				    name == "preferences" ||
				    name == "texinfo");
			break;
		}

		if (name == "print") {
			disable = !Exporter::IsExportable(*buf, "dvi") ||
				lyxrc.print_command == "none";
		} else if (name == "character") {
			UpdatableInset * tli = view()->theLockingInset();
			disable = tli && tli->lyxCode() == InsetOld::ERT_CODE;
		} else if (name == "vclog") {
			disable = !buf->lyxvc().inUse();
		} else if (name == "latexlog") {
			disable = !IsFileReadable(buf->getLogName().second);
		}
		break;
	}

	default:
		break;
	}

	// the functions which insert insets
	InsetOld::Code code = InsetOld::NO_CODE;
	switch (ev.action) {
	case LFUN_DIALOG_SHOW_NEW_INSET:
		if (ev.argument == "bibitem")
			code = InsetOld::BIBITEM_CODE;
		else if (ev.argument == "bibtex")
			code = InsetOld::BIBTEX_CODE;
		else if (ev.argument == "citation")
			code = InsetOld::CITE_CODE;
		else if (ev.argument == "ert")
			code = InsetOld::ERT_CODE;
		else if (ev.argument == "external")
			code = InsetOld::EXTERNAL_CODE;
		else if (ev.argument == "float")
			code = InsetOld::FLOAT_CODE;
		else if (ev.argument == "graphics")
			code = InsetOld::GRAPHICS_CODE;
		else if (ev.argument == "include")
			code = InsetOld::INCLUDE_CODE;
		else if (ev.argument == "index")
			code = InsetOld::INDEX_CODE;
		else if (ev.argument == "label")
			code = InsetOld::LABEL_CODE;
		else if (ev.argument == "minipage")
			code = InsetOld::MINIPAGE_CODE;
		else if (ev.argument == "ref")
			code = InsetOld::REF_CODE;
		else if (ev.argument == "toc")
			code = InsetOld::TOC_CODE;
		else if (ev.argument == "url")
			code = InsetOld::URL_CODE;
		else if (ev.argument == "wrap")
			code = InsetOld::WRAP_CODE;
		break;

	case LFUN_INSET_ERT:
		code = InsetOld::ERT_CODE;
		break;
	case LFUN_INSET_FOOTNOTE:
		code = InsetOld::FOOT_CODE;
		break;
	case LFUN_TABULAR_INSERT:
		code = InsetOld::TABULAR_CODE;
		break;
	case LFUN_INSET_MARGINAL:
		code = InsetOld::MARGIN_CODE;
		break;
	case LFUN_INSET_MINIPAGE:
		code = InsetOld::MINIPAGE_CODE;
		break;
	case LFUN_INSET_FLOAT:
	case LFUN_INSET_WIDE_FLOAT:
		code = InsetOld::FLOAT_CODE;
		break;
	case LFUN_INSET_WRAP:
		code = InsetOld::WRAP_CODE;
		break;
	case LFUN_FLOAT_LIST:
		code = InsetOld::FLOAT_LIST_CODE;
		break;
#if 0
	case LFUN_INSET_LIST:
		code = InsetOld::LIST_CODE;
		break;
	case LFUN_INSET_THEOREM:
		code = InsetOld::THEOREM_CODE;
		break;
#endif
	case LFUN_INSET_CAPTION:
		code = InsetOld::CAPTION_CODE;
		break;
	case LFUN_INSERT_NOTE:
		code = InsetOld::NOTE_CODE;
		break;
	case LFUN_INSERT_BOX:
		code = InsetOld::BOX_CODE;
		break;
	case LFUN_INSERT_BRANCH:
		code = InsetOld::BRANCH_CODE;
		if (buf->params().branchlist().empty())
			disable = true;
		break;
	case LFUN_INSERT_LABEL:
		code = InsetOld::LABEL_CODE;
		break;
	case LFUN_INSET_OPTARG:
		code = InsetOld::OPTARG_CODE;
		break;
	case LFUN_ENVIRONMENT_INSERT:
		code = InsetOld::MINIPAGE_CODE;
		break;
	case LFUN_INDEX_INSERT:
		code = InsetOld::INDEX_CODE;
		break;
	case LFUN_INDEX_PRINT:
		code = InsetOld::INDEX_PRINT_CODE;
		break;
	case LFUN_TOC_INSERT:
		code = InsetOld::TOC_CODE;
		break;
	case LFUN_HTMLURL:
	case LFUN_URL:
		code = InsetOld::URL_CODE;
		break;
	case LFUN_QUOTE:
		// always allow this, since we will inset a raw quote
		// if an inset is not allowed.
		break;
	case LFUN_HYPHENATION:
	case LFUN_LIGATURE_BREAK:
	case LFUN_HFILL:
	case LFUN_MENU_SEPARATOR:
	case LFUN_LDOTS:
	case LFUN_END_OF_SENTENCE:
		code = InsetOld::SPECIALCHAR_CODE;
		break;
	case LFUN_SPACE_INSERT:
		// slight hack: we know this is allowed in math mode
		if (!mathcursor)
			code = InsetOld::SPACE_CODE;
		break;
	case LFUN_INSET_DIALOG_SHOW: {
		InsetOld * inset = view()->getLyXText()->getInset();
		disable = !inset;
		if (!disable) {
			code = inset->lyxCode();
			if (!(code == InsetOld::INCLUDE_CODE
				|| code == InsetOld::BIBTEX_CODE
				|| code == InsetOld::FLOAT_LIST_CODE
				|| code == InsetOld::TOC_CODE))
				disable = true;
		}
		break;
	}
	default:
		break;
	}
	if (code != InsetOld::NO_CODE && tli && !tli->insetAllowed(code))
		disable = true;

	if (disable)
		flag.disabled(true);

	// A few general toggles
	switch (ev.action) {
	case LFUN_TOOLTIPS_TOGGLE:
		flag.setOnOff(owner->getDialogs().tooltipsEnabled());
		break;

	case LFUN_READ_ONLY_TOGGLE:
		flag.setOnOff(buf->isReadonly());
		break;
	case LFUN_APPENDIX:
		flag.setOnOff(view()->getLyXText()->cursorPar()->params().startOfAppendix());
		break;
	case LFUN_SWITCHBUFFER:
		// toggle on the current buffer, but do not toggle off
		// the other ones (is that a good idea?)
		if (ev.argument == buf->fileName())
			flag.setOnOff(true);
		break;
	case LFUN_TRACK_CHANGES:
		flag.setOnOff(buf->params().tracking_changes);
		break;
	default:
		break;
	}

	// the font related toggles
	if (!mathcursor) {
		LyXFont const & font = view()->getLyXText()->real_current_font;
		switch (ev.action) {
		case LFUN_EMPH:
			flag.setOnOff(font.emph() == LyXFont::ON);
			break;
		case LFUN_NOUN:
			flag.setOnOff(font.noun() == LyXFont::ON);
			break;
		case LFUN_BOLD:
			flag.setOnOff(font.series() == LyXFont::BOLD_SERIES);
			break;
		case LFUN_SANS:
			flag.setOnOff(font.family() == LyXFont::SANS_FAMILY);
			break;
		case LFUN_ROMAN:
			flag.setOnOff(font.family() == LyXFont::ROMAN_FAMILY);
			break;
		case LFUN_CODE:
			flag.setOnOff(font.family() == LyXFont::TYPEWRITER_FAMILY);
			break;
		default:
			break;
		}
	} else {
		string tc = mathcursor->getLastCode();
		switch (ev.action) {
		case LFUN_BOLD:
			flag.setOnOff(tc == "mathbf");
			break;
		case LFUN_SANS:
			flag.setOnOff(tc == "mathsf");
			break;
		case LFUN_EMPH:
			flag.setOnOff(tc == "mathcal");
			break;
		case LFUN_ROMAN:
			flag.setOnOff(tc == "mathrm");
			break;
		case LFUN_CODE:
			flag.setOnOff(tc == "mathtt");
			break;
		case LFUN_NOUN:
			flag.setOnOff(tc == "mathbb");
			break;
		case LFUN_DEFAULT:
			flag.setOnOff(tc == "mathnormal");
			break;
		default:
			break;
		}
	}

	// this one is difficult to get right. As a half-baked
	// solution, we consider only the first action of the sequence
	if (ev.action == LFUN_SEQUENCE) {
		// argument contains ';'-terminated commands
#warning LyXAction arguments not handled here.
		flag = getStatus(FuncRequest(lyxaction.lookupFunc(token(ev.argument, ';', 0))));
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

} //namespace anon


void LyXFunc::dispatch(FuncRequest const & func, bool verbose)
{
	string argument = func.argument;
	kb_action action = func.action;

	lyxerr[Debug::ACTION] << "LyXFunc::dispatch: action[" << action
			      <<"] arg[" << argument << ']' << endl;

	// we have not done anything wrong yet.
	errorstat = false;
	dispatch_buffer.erase();

#ifdef NEW_DISPATCHER
	// We try do call the most specific dispatcher first:
	//  1. the lockinginset's dispatch
	//  2. the bufferview's dispatch
	//  3. the lyxview's dispatch
#endif

	selection_possible = false;

	// We cannot use this function here
	if (getStatus(func).disabled()) {
		lyxerr[Debug::ACTION] << "LyXFunc::dispatch: "
		       << lyxaction.getActionName(action)
		       << " [" << action << "] is disabled at this location"
		       << endl;
		setErrorMessage(getStatusMessage());
		goto exit_with_message;
	}

	if (view()->available())
		view()->hideCursor();

#if 1
	{
		UpdatableInset * innerinset = view()->theLockingInset();
		for (UpdatableInset * tmp = innerinset; tmp; tmp = tmp->lockingInset())
			innerinset = tmp;

		if (view()->cursor().innerInset() != innerinset) {
			lyxerr << "### CURSOR OUT OF SYNC: tli: "
				<< view()->theLockingInset() << "  inner: "
				<< innerinset
				<< "\ncursor: " << view()->cursor() << endl; 
		}
	}

	if (0) {
		DispatchResult result =
			view()->cursor().dispatch(FuncRequest(func, view()));

		if (result.dispatched()) {
			if (result.update()) {
				view()->update();
			}
			lyxerr << "dispatched by Cursor::dispatch()\n";
			goto exit_with_message;
		}
		lyxerr << "### NOT DispatchResult(true, true) BY Cursor::dispatch() ###\n";
	}
#endif


	if (view()->available() && view()->theLockingInset()) {
		DispatchResult result;
		if (action > 1 || (action == LFUN_UNKNOWN_ACTION &&
				     !keyseq.deleted()))
		{
			UpdatableInset * inset = view()->theLockingInset();
#if 1
			int inset_x;
			int dummy_y;
			inset->getCursorPos(view(), inset_x, dummy_y);
#endif
			if (action == LFUN_UNKNOWN_ACTION && argument.empty())
				argument = encoded_last_key;

			// the insets can't try to handle this,
			// a table cell in the dummy position will
			// lock its insettext, the insettext will
			// pass it the bufferview, and succeed,
			// so it will stay not locked. Not good
			// if we've just done LFUN_ESCAPE (which
			// injects an LFUN_PARAGRAPH_UPDATE)
			if (action == LFUN_PARAGRAPH_UPDATE) {
				view()->dispatch(func);
				goto exit_with_message;
			}

			// Undo/Redo is a bit tricky for insets.
			if (action == LFUN_UNDO) {
				view()->undo();
				goto exit_with_message;
			}

			if (action == LFUN_REDO) {
				view()->redo();
				goto exit_with_message;
			}

			// Hand-over to inset's own dispatch:
			result = inset->dispatch(FuncRequest(view(), action, argument));
			if (result.dispatched()) {
				if (result.update())
					view()->update();

				goto exit_with_message;
			}

			// If DispatchResult(false), just soldier on
			if (result.val() == FINISHED) {
				owner->clearMessage();
				goto exit_with_message;
				// We do not need special RTL handling here:
				// FINISHED means that the cursor should be
				// one position after the inset.
			}

			if (result.val() == FINISHED_RIGHT) {
				view()->text->cursorRight(view());
				moveCursorUpdate();
				owner->clearMessage();
				goto exit_with_message;
			}

			if (result.val() == FINISHED_UP) {
				LyXText * text = view()->text;
				ParagraphList::iterator pit = text->cursorPar();
				Row const & row = *pit->getRow(text->cursor.pos());
				if (text->isFirstRow(pit, row)) {
#if 1
					text->setCursorFromCoordinates(
						text->cursor.x() + inset_x,
						text->cursor.y() -
						row.baseline() - 1);
					view()->x_target(text->cursor.x());
#else
					text->cursorUp(view());
#endif
					moveCursorUpdate();
				} else {
					view()->update();
				}
				owner->clearMessage();
				goto exit_with_message;
			}

			if (result.val() == FINISHED_DOWN) {
				LyXText * text = view()->text;
				ParagraphList::iterator pit = text->cursorPar();
				Row const & row = *pit->getRow(text->cursor.pos());
				if (text->isLastRow(pit, row)) {
#if 1
					text->setCursorFromCoordinates(
						text->cursor.x() + inset_x,
						text->cursor.y() -
						row.baseline() +
						row.height() + 1);
					view()->x_target(text->cursor.x());
#else
					text->cursorDown(view());
#endif
				} else {
					text->cursorRight(view());
				}
				moveCursorUpdate();
				owner->clearMessage();
				goto exit_with_message;
			}

#warning I am not sure this is still right, please have a look! (Jug 20020417)
			// result == DispatchResult()
			//setMessage(N_("Text mode"));
			switch (action) {
			case LFUN_UNKNOWN_ACTION:
			case LFUN_BREAKPARAGRAPH:
			case LFUN_BREAKLINE:
				view()->text->cursorRight(view());
				view()->switchKeyMap();
				owner->view_state_changed();
				break;
			case LFUN_RIGHT:
				if (!view()->text->cursorPar()->isRightToLeftPar(owner->buffer()->params())) {
					view()->text->cursorRight(view());
					moveCursorUpdate();
					owner->view_state_changed();
				}
				goto exit_with_message;
			case LFUN_LEFT:
				if (view()->text->cursorPar()->isRightToLeftPar(owner->buffer()->params())) {
					view()->text->cursorRight(view());
					moveCursorUpdate();
					owner->view_state_changed();
				}
				goto exit_with_message;
			case LFUN_DOWN: {
				LyXText * text = view()->text;
				ParagraphList::iterator pit = text->cursorPar();
				if (text->isLastRow(pit, *pit->getRow(text->cursor.pos())))
					view()->text->cursorDown(view());
				else
					view()->text->cursorRight(view());
				moveCursorUpdate();
				owner->view_state_changed();
				goto exit_with_message;
			}
			default:
				break;
			}
		}
	}

	switch (action) {

	case LFUN_ESCAPE: {
		if (!view()->available())
			break;
		// this function should be used always [asierra060396]
		UpdatableInset * tli = view()->theLockingInset();
		if (tli) {
			UpdatableInset * lock = tli->getLockingInset();

			if (tli == lock) {
				view()->unlockInset(tli);
				view()->text->cursorRight(view());
				moveCursorUpdate();
				owner->view_state_changed();
			} else {
				tli->unlockInsetInInset(view(), lock, true);
			}
			finishUndo();
			// Tell the paragraph dialog that we changed paragraph
			dispatch(FuncRequest(LFUN_PARAGRAPH_UPDATE));
		}
		break;
	}

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
		bool fw = (action == LFUN_WORDFINDFORWARD);
		if (!searched_string.empty())
			lyx::find::find(view(), searched_string,
					true, false, fw);
		break;
	}

	case LFUN_PREFIX:
		if (view()->available() && !view()->theLockingInset())
			view()->update();
		owner->message(keyseq.printOptions());
		break;

	// --- Misc -------------------------------------------
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

	case LFUN_CENTER: // this is center and redraw.
		view()->center();
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
			owner->getDialogs().showSendto();
		else {
			Exporter::Export(owner->buffer(), argument, false);
			view()->showErrorList(BufferFormat(*owner->buffer()));
		}
		break;

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

	case LFUN_UNDO:
		view()->undo();
		break;

	case LFUN_REDO:
		view()->redo();
		break;

	case LFUN_FREEFONT_APPLY:
		apply_freefont(view());
		break;

	case LFUN_FREEFONT_UPDATE:
		update_and_apply_freefont(view(), argument);
		break;

	case LFUN_RECONFIGURE:
		Reconfigure(view());
		break;

#if 0
	case LFUN_FLOATSOPERATE:
		if (argument == "openfoot")
			view()->allFloats(1,0);
		else if (argument == "closefoot")
			view()->allFloats(0,0);
		else if (argument == "openfig")
			view()->allFloats(1,1);
		else if (argument == "closefig")
			view()->allFloats(0,1);
		break;
#else
#ifdef WITH_WARNINGS
//#warning Find another implementation here (or another lyxfunc)!
#endif
#endif

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
		view()->buffer(bufferlist.getBuffer(argument));
		break;

	case LFUN_FILE_NEW:
		NewFile(view(), argument);
		break;

	case LFUN_FILE_OPEN:
		open(argument);
		break;

	case LFUN_LAYOUT_TABULAR: {
		UpdatableInset * tli = view()->theLockingInset();
		if (tli) {
			if (tli->lyxCode() == InsetOld::TABULAR_CODE) {
				static_cast<InsetTabular *>(tli)->openLayoutDialog(view());
			} else if (tli->getFirstLockingInsetOfType(InsetOld::TABULAR_CODE)) {
				static_cast<InsetTabular *>(
					tli->getFirstLockingInsetOfType(InsetOld::TABULAR_CODE))
						->openLayoutDialog(view());
			}
		}
		break;
	}

	case LFUN_DROP_LAYOUTS_CHOICE:
		owner->getToolbar().openLayoutList();
		break;

	case LFUN_MENU_OPEN_BY_NAME:
		owner->getMenubar().openByName(argument);
		break; // RVDK_PATCH_5

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
			view()->buffer(bufferlist.getBufferFromTmp(file_name));
		} else {
			// Must replace extension of the file to be .lyx
			// and get full path
			string const s = ChangeExtension(file_name, ".lyx");
			// Either change buffer or load the file
			if (bufferlist.exists(s)) {
				view()->buffer(bufferlist.getBuffer(s));
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


		par.lockPath(view());
		LyXText * lt = par.text(view());

		// Set the cursor
		lt->setCursor(par.pit(), 0);
		view()->switchKeyMap();
		owner->view_state_changed();

		view()->center();
		// see BufferView_pimpl::center()
		view()->updateScrollbar();
	}
	break;

	// --- insert characters ----------------------------------------

	// ---  Mathed stuff. If we are here, there is no locked inset yet.
	case LFUN_MATH_EXTERN:
	case LFUN_MATH_NUMBER:
	case LFUN_MATH_NONUMBER:
	case LFUN_MATH_LIMITS:
		setErrorMessage(N_("This is only allowed in math mode!"));
		break;

	// passthrough hat and underscore outside mathed:
	case LFUN_SUBSCRIPT:
		dispatch(FuncRequest(view(), LFUN_SELFINSERT, "_"));
		break;

	case LFUN_SUPERSCRIPT:
		dispatch(FuncRequest(view(), LFUN_SELFINSERT, "^"));
		break;

	case LFUN_DIALOG_SHOW: {
		string const name = func.getArg(0);
		string data = trim(func.argument.substr(name.size()));

		if (name == "character") {
			data = freefont2string();
			if (!data.empty())
				owner->getDialogs().show("character", data);
		} else if (name == "document")
			owner->getDialogs().showDocument();
		else if (name == "findreplace")
			owner->getDialogs().showSearch();
		else if (name == "forks")
			owner->getDialogs().showForks();
		else if (name == "preamble")
			owner->getDialogs().showPreamble();
		else if (name == "preferences")
			owner->getDialogs().showPreferences();
		else if (name == "print")
			owner->getDialogs().showPrint();
		else if (name == "spellchecker")
			owner->getDialogs().showSpellchecker();
		else
			owner->getDialogs().show(name, data);
		break;
	}

	case LFUN_DIALOG_SHOW_NEW_INSET: {
		string const & name = argument;
		string data;
		if (name == "bibitem" ||
		    name == "bibtex" ||
		    name == "include" ||
		    name == "index" ||
		    name == "ref" ||
		    name == "toc" ||
		    name == "url") {
			InsetCommandParams p(name);
			data = InsetCommandMailer::params2string(name, p);
		} else if (name == "citation") {
			InsetCommandParams p("cite");
			data = InsetCommandMailer::params2string(name, p);
		}
		owner->getDialogs().show(name, data, 0);
		break;
	}

	case LFUN_DIALOG_SHOW_NEXT_INSET:
		break;

	case LFUN_INSET_DIALOG_SHOW: {
		InsetOld * inset = view()->getLyXText()->getInset();
		if (inset)
			inset->dispatch(FuncRequest(view(), LFUN_INSET_DIALOG_SHOW));
		break;
	}

	case LFUN_DIALOG_UPDATE: {
		string const & name = argument;
		// Can only update a dialog connected to an existing inset
		InsetBase * inset = owner->getDialogs().getOpenInset(name);
		if (inset) {
			FuncRequest fr(view(), LFUN_INSET_DIALOG_UPDATE,
				       func.argument);
			inset->dispatch(fr);
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
		if (bufferlist.exists(filename))
			view()->buffer(bufferlist.getBuffer(filename));
		else
			view()->loadLyXFile(filename);
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
		argument = split(argument, countstr, ' ');
		istringstream is(countstr);
		int count = 0;
		is >> count;
		lyxerr << "repeat: count: " << count << " cmd: " << argument << endl;
		for (int i = 0; i < count; ++i)
			dispatch(lyxaction.lookupFunc(argument));
		break;
	}

	case LFUN_SEQUENCE:
		// argument contains ';'-terminated commands
		while (!argument.empty()) {
			string first;
			argument = split(argument, first, ';');
			dispatch(lyxaction.lookupFunc(first));
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
		view()->update();
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

		view()->update();
		break;
	}

	case LFUN_MESSAGE:
		owner->message(argument);
		break;

	case LFUN_FORKS_KILL:
		if (isStrInt(argument)) {
			pid_t const pid = strToInt(argument);
			ForkedcallsController & fcc = ForkedcallsController::get();
			fcc.kill(pid);
		}
		break;

	case LFUN_TOOLTIPS_TOGGLE:
		owner->getDialogs().toggleTooltips();
		break;

	case LFUN_EXTERNAL_EDIT:
		InsetExternal().dispatch(FuncRequest(view(), action, argument));
		break;

	default:
		// Then if it was none of the above
		// Trying the BufferView::pimpl dispatch:
		if (!view()->dispatch(func))
			lyxerr << "A truly unknown func ["
			       << lyxaction.getActionName(func.action) << "]!"
			       << endl;
		break;
	} // end of switch

exit_with_message:

	view()->owner()->updateLayoutChoice();

	if (view()->available()) {
		if (view()->fitCursor()) {
			//lyxerr << "LyXFunc->fitCursor->update" << endl;
			view()->update();
		}

		// If we executed a mutating lfun, mark the buffer as dirty
		if (!getStatus(func).disabled()
		    && !lyxaction.funcHasFlag(func.action, LyXAction::NoBuffer)
		    && !lyxaction.funcHasFlag(func.action, LyXAction::ReadOnly))
			view()->buffer()->markDirty();
	}

	sendDispatchMessage(getMessage(), func, verbose);
}


void LyXFunc::sendDispatchMessage(string const & msg,
				  FuncRequest const & func, bool verbose)
{
	owner->updateMenubar();
	owner->updateToolbar();

	if (func.action == LFUN_SELFINSERT || !verbose) {
		lyxerr[Debug::ACTION] << "dispatch msg is " << msg << endl;
		if (!msg.empty())
			owner->message(msg);
		return;
	}

	string dispatch_msg = msg;
	if (!dispatch_msg.empty())
		dispatch_msg += ' ';

	string comname = lyxaction.getActionName(func.action);

	bool argsadded = false;

	if (!func.argument.empty()) {
		if (func.action != LFUN_UNKNOWN_ACTION) {
			comname += ' ' + func.argument;
			argsadded = true;
		}
	}

	string const shortcuts = toplevel_keymap->findbinding(func);

	if (!shortcuts.empty()) {
		comname += ": " + shortcuts;
	} else if (!argsadded && !func.argument.empty()) {
		comname += ' ' + func.argument;
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
	keyseq.stdmap = keyseq.curmap = toplevel_keymap.get();
	cancel_meta_seq.stdmap = cancel_meta_seq.curmap = toplevel_keymap.get();
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
				       _("*.lyx| LyX Documents (*.lyx)"));

		if (result.first == FileDialog::Later)
			return;

		string const fname = result.second;

		if (fname.empty())
			return;
		templname = fname;
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
				       _("*.lyx| LyX Documents (*.lyx)"));

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

		string const extension = "*." + formats.extension(format)
			+ "| " + formats.prettyName(format)
			+ " (*." + formats.extension(format) + ')';

		FileDialog::Result result = fileDlg.open(initpath,
							   extension);

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
			view()->buffer(bufferlist.first());
		}
	}
}


// Each "owner" should have it's own message method. lyxview and
// the minibuffer would use the minibuffer, but lyxserver would
// send an ERROR signal to its client.  Alejandro 970603
// This func is bit problematic when it comes to NLS, to make the
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


string const LyXFunc::view_status_message()
{
	// When meta-fake key is pressed, show the key sequence so far + "M-".
	if (wasMetaKey()) {
		return keyseq.print() + "M-";
	}

	// Else, when a non-complete key sequence is pressed,
	// show the available options.
	if (keyseq.length() > 0 && !keyseq.deleted()) {
		return keyseq.printOptions();
	}

	if (!view()->available())
		return _("Welcome to LyX!");

	return currentState(view());
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
