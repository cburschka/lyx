/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#include "lyxfunc.h"
#include "version.h"
#include "kbmap.h"
#include "lyxrow.h"
#include "bufferlist.h"
#include "BufferView.h"
#include "lyxserver.h"
#include "intl.h"
#include "lyx_main.h"
#include "lyx_cb.h"
#include "LyXAction.h"
#include "debug.h"
#include "lyxrc.h"
#include "lyxtext.h"
#include "gettext.h"
#include "Lsstream.h"
#include "trans_mgr.h"
#include "encoding.h"
#include "layout.h"
#include "bufferview_funcs.h"
#include "frontends/LyXView.h"
#include "frontends/lyx_gui.h"
#include "vspace.h"
#include "FloatList.h"
#include "format.h"
#include "exporter.h"
#include "importer.h"
#include "TextCache.h"
#include "lyxfind.h"
#include "undo_funcs.h"
#include "ParagraphParameters.h"

#include "insets/insetcommand.h"
#include "insets/insettabular.h"

#include "mathed/formulamacro.h"
#include "mathed/math_cursor.h"
#include "mathed/math_inset.h"

#include "frontends/FileDialog.h"
#include "frontends/Dialogs.h"
#include "frontends/Toolbar.h"
#include "frontends/Menubar.h"
#include "frontends/Alert.h"

#include "graphics/GraphicsCache.h"

#include "support/lyxalgo.h"
#include "support/LAssert.h"
#include "support/filetools.h"
#include "support/FileInfo.h"
#include "support/forkedcontr.h"
#include "support/lstrings.h"
#include "support/path.h"
#include "support/lyxfunctional.h"

#include "support/BoostFormat.h"

#include <ctime>
#include <clocale>
#include <cstdlib>
#include <cctype>

#include <utility>
#include <algorithm>

using std::pair;
using std::make_pair;
using std::endl;
using std::find_if;
using std::vector;
using std::transform;
using std::back_inserter;
using namespace bv_funcs;

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


inline
LyXText * LyXFunc::TEXT(bool flag = true) const
{
	if (flag)
		return view()->text;
	return view()->getLyXText();
}


inline
void LyXFunc::moveCursorUpdate(bool flag, bool selecting)
{
	if (selecting || TEXT(flag)->selection.mark()) {
		TEXT(flag)->setSelection();
		if (!TEXT(flag)->isInInset())
		    view()->toggleToggle();
	}
	view()->update(TEXT(flag), BufferView::SELECT);
	view()->showCursor();

	view()->switchKeyMap();
}


void LyXFunc::handleKeyFunc(kb_action action)
{
	char c = encoded_last_key;

	if (keyseq.length()) {
		c = 0;
	}

	owner->getIntl().getTransManager()
		.deadkey(c, get_accent(action).accent, TEXT(false));
	// Need to clear, in case the minibuffer calls these
	// actions
	keyseq.clear();
	// copied verbatim from do_accent_char
	view()->update(TEXT(false), BufferView::SELECT);
	TEXT(false)->selection.cursor = TEXT(false)->cursor;
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

	int action = cancel_meta_seq.addkey(keysym, state);
	lyxerr[Debug::KEY] << "action first set to [" << action << ']' << endl;

	// When not cancel or meta-fake, do the normal lookup.
	// Note how the meta_fake Mod1 bit is OR-ed in and reset afterwards.
	// Mostly, meta_fake_bit = key_modifier::none. RVDK_PATCH_5.
	if ((action != LFUN_CANCEL) && (action != LFUN_META_FAKE)) {
		// remove Caps Lock and Mod2 as a modifiers
		action = keyseq.addkey(keysym, (state | meta_fake_bit));
		lyxerr[Debug::KEY] << "action now set to ["
			<< action << ']' << endl;
	}

	// Dont remove this unless you know what you are doing.
	meta_fake_bit = key_modifier::none;

	// can this happen now ?
	if (action == LFUN_NOACTION) {
		action = LFUN_PREFIX;
	}

	if (lyxerr.debugging(Debug::KEY)) {
		lyxerr << "Key [action="
		       << action << "]["
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
	if (action == LFUN_UNKNOWN_ACTION && state == key_modifier::shift) {
		lyxerr[Debug::KEY] << "Trying without shift" << endl;
		action = keyseq.addkey(keysym, key_modifier::none);
		lyxerr[Debug::KEY] << "Action now " << action << endl;
	}

	if (action == LFUN_UNKNOWN_ACTION) {
		// Hmm, we didn't match any of the keysequences. See
		// if it's normal insertable text not already covered
		// by a binding
		if (keysym->isText() && keyseq.length() == 1) {
			lyxerr[Debug::KEY] << "isText() is true, inserting." << endl;
			action = LFUN_SELFINSERT;
		} else {
			lyxerr[Debug::KEY] << "Unknown, !isText() - giving up" << endl;
			owner->message(_("Unknown function."));
			return;
		}
	}

	if (action == LFUN_SELFINSERT) {
		if (encoded_last_key != 0) {
			string arg;
			arg += encoded_last_key;

			dispatch(FuncRequest(view(), LFUN_SELFINSERT, arg));

			lyxerr[Debug::KEY] << "SelfInsert arg[`"
				   << argument << "']" << endl;
		}
	} else {
		dispatch(action);
	}
}


FuncStatus LyXFunc::getStatus(int ac) const
{
	return getStatus(lyxaction.retrieveActionArg(ac));
}


FuncStatus LyXFunc::getStatus(FuncRequest const & ev) const
{
	FuncStatus flag;
	Buffer * buf = owner->buffer();

	if (ev.action == LFUN_NOACTION) {
		setStatusMessage(N_("Nothing to do"));
		return flag.disabled(true);
	}

	switch (ev.action) {
	case LFUN_UNKNOWN_ACTION:
#ifndef HAVE_LIBAIKSAURUS
	case LFUN_THESAURUS_ENTRY:
#endif
		flag.unknown(true);
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
			return flag.disabled(true);
		}
	}

	UpdatableInset * tli = view()->theLockingInset();

	// I would really like to avoid having this switch and rather try to
	// encode this in the function itself.
	bool disable = false;
	switch (ev.action) {
	case LFUN_MENUPRINT:
		disable = !Exporter::IsExportable(buf, "dvi")
			|| lyxrc.print_command == "none";
		break;
	case LFUN_EXPORT:
		disable = ev.argument != "custom"
			&& !Exporter::IsExportable(buf, ev.argument);
		break;
	case LFUN_UNDO:
		disable = buf->undostack.empty();
		break;
	case LFUN_REDO:
		disable = buf->redostack.empty();
		break;
	case LFUN_CUT:
	case LFUN_COPY:
		if (tli) {
			UpdatableInset * in = tli;
			if (in->lyxCode() != Inset::TABULAR_CODE) {
				in = tli->getFirstLockingInsetOfType(Inset::TABULAR_CODE);
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
		disable = !Exporter::IsExportable(buf, "program");
		break;

	case LFUN_LAYOUT_CHARACTER:
		disable = tli && tli->lyxCode() == Inset::ERT_CODE;
		break;

	case LFUN_LAYOUT_TABULAR:
		disable = !tli
			|| (tli->lyxCode() != Inset::TABULAR_CODE
			    && !tli->getFirstLockingInsetOfType(Inset::TABULAR_CODE));
		break;

	case LFUN_DEPTH_MIN:
		disable = !changeDepth(view(), TEXT(false), DEC_DEPTH, true);
		break;

	case LFUN_DEPTH_PLUS:
		disable = !changeDepth(view(), TEXT(false), INC_DEPTH, true);
		break;

	case LFUN_LAYOUT:
	case LFUN_LAYOUT_PARAGRAPH: {
		Inset * inset = TEXT(false)->cursor.par()->inInset();
		disable = inset && inset->forceDefaultParagraphs(inset);
		break;
	}

	case LFUN_INSET_OPTARG:
		disable = (TEXT(false)->cursor.par()->layout()->optionalargs == 0);
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
		} else
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
			if (tli->lyxCode() == Inset::TABULAR_CODE) {
				ret = static_cast<InsetTabular *>(tli)
					->getStatus(ev.argument);
				flag |= ret;
				disable = false;
			} else if (tli->getFirstLockingInsetOfType(Inset::TABULAR_CODE)) {
				ret = static_cast<InsetTabular *>
					(tli->getFirstLockingInsetOfType(Inset::TABULAR_CODE))
					->getStatus(ev.argument);
				flag |= ret;
				disable = false;
			} else {
				disable = true;
			}
		} else {
			static InsetTabular inset(*owner->buffer(), 1, 1);
			FuncStatus ret;

			disable = true;
			ret = inset.getStatus(ev.argument);
			if (ret.onoff(true) || ret.onoff(false))
				flag.setOnOff(false);
		}
		break;

	case LFUN_VC_REGISTER:
		disable = buf->lyxvc.inUse();
		break;
	case LFUN_VC_CHECKIN:
		disable = !buf->lyxvc.inUse() || buf->isReadonly();
		break;
	case LFUN_VC_CHECKOUT:
		disable = !buf->lyxvc.inUse() || !buf->isReadonly();
		break;
	case LFUN_VC_REVERT:
	case LFUN_VC_UNDO:
	case LFUN_VC_HISTORY:
		disable = !buf->lyxvc.inUse();
		break;
	case LFUN_MENURELOAD:
		disable = buf->isUnnamed() || buf->isClean();
		break;
	case LFUN_BOOKMARK_GOTO:
		disable =  !view()->
			isSavedPosition(strToUnsignedInt(ev.argument));
		break;
	case LFUN_MERGE_CHANGES:
	case LFUN_ACCEPT_CHANGE:
	case LFUN_REJECT_CHANGE:
	case LFUN_ACCEPT_ALL_CHANGES:
	case LFUN_REJECT_ALL_CHANGES:
		disable = !buf->params.tracking_changes;
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
		if (inset->lyxCode() == Inset::TEXT_CODE)
			inset = static_cast<UpdatableInset*>(inset->owner());

		Inset::Code code = inset->lyxCode();
		switch (code) {
			case Inset::TABULAR_CODE:
				disable = ev.argument != "tabular";
				break;
			case Inset::ERT_CODE:
				disable = ev.argument != "ert";
				break;
			case Inset::FLOAT_CODE:
				disable = ev.argument != "float";
				break;
			case Inset::MINIPAGE_CODE:
				disable = ev.argument != "minipage";
				break;
			case Inset::WRAP_CODE:
				disable = ev.argument != "wrap";
				break;
			default:
				break;
		}
		break;
	}

	case LFUN_LATEX_LOG:
		disable = !IsFileReadable(buf->getLogName().second);
		break;

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

	default:
		break;
	}

	// the functions which insert insets
	Inset::Code code = Inset::NO_CODE;
	switch (ev.action) {
	case LFUN_DIALOG_SHOW_NEW_INSET:
		if (ev.argument == "bibitem")
			code = Inset::BIBITEM_CODE;
		else if (ev.argument == "bibtex")
			code = Inset::BIBTEX_CODE;
		else if (ev.argument == "citation")
			code = Inset::CITE_CODE;
		else if (ev.argument == "ert")
			code = Inset::ERT_CODE;
		else if (ev.argument == "external")
			code = Inset::EXTERNAL_CODE;
		else if (ev.argument == "float")
			code = Inset::FLOAT_CODE;
		else if (ev.argument == "graphics")
			code = Inset::GRAPHICS_CODE;
		else if (ev.argument == "include")
			code = Inset::INCLUDE_CODE;
		else if (ev.argument == "index")
			code = Inset::INDEX_CODE;
		else if (ev.argument == "label")
			code = Inset::LABEL_CODE;
		else if (ev.argument == "minipage")
			code = Inset::MINIPAGE_CODE;
		else if (ev.argument == "ref")
			code = Inset::REF_CODE;
		else if (ev.argument == "toc")
			code = Inset::TOC_CODE;
		else if (ev.argument == "url")
			code = Inset::URL_CODE;
		else if (ev.argument == "wrap")
			code = Inset::WRAP_CODE;
		break;

	case LFUN_INSET_ERT:
		code = Inset::ERT_CODE;
		break;
	case LFUN_INSET_FOOTNOTE:
		code = Inset::FOOT_CODE;
		break;
	case LFUN_TABULAR_INSERT:
		code = Inset::TABULAR_CODE;
		break;
	case LFUN_INSET_MARGINAL:
		code = Inset::MARGIN_CODE;
		break;
	case LFUN_INSET_MINIPAGE:
		code = Inset::MINIPAGE_CODE;
		break;
	case LFUN_INSET_FLOAT:
	case LFUN_INSET_WIDE_FLOAT:
		code = Inset::FLOAT_CODE;
		break;
	case LFUN_INSET_WRAP:
		code = Inset::WRAP_CODE;
		break;
	case LFUN_FLOAT_LIST:
		code = Inset::FLOAT_LIST_CODE;
		break;
#if 0
	case LFUN_INSET_LIST:
		code = Inset::LIST_CODE;
		break;
	case LFUN_INSET_THEOREM:
		code = Inset::THEOREM_CODE;
		break;
#endif
	case LFUN_INSET_CAPTION:
		code = Inset::CAPTION_CODE;
		break;
	case LFUN_INSERT_NOTE:
		code = Inset::NOTE_CODE;
		break;
	case LFUN_INSERT_LABEL:
		code = Inset::LABEL_CODE;
		break;
	case LFUN_INSET_OPTARG:
		code = Inset::OPTARG_CODE;
		break;
	case LFUN_ENVIRONMENT_INSERT:
		code = Inset::MINIPAGE_CODE;
		break;
	case LFUN_INDEX_INSERT:
		code = Inset::INDEX_CODE;
		break;
	case LFUN_INDEX_PRINT:
		code = Inset::INDEX_PRINT_CODE;
		break;
	case LFUN_TOC_INSERT:
		code = Inset::TOC_CODE;
		break;
	case LFUN_PARENTINSERT:
		code = Inset::PARENT_CODE;
		break;
	case LFUN_HTMLURL:
	case LFUN_URL:
		code = Inset::URL_CODE;
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
		code = Inset::SPECIALCHAR_CODE;
		break;
	case LFUN_PROTECTEDSPACE:
		// slight hack: we know this is allowed in math mode
		if (!mathcursor)
			code = Inset::SPECIALCHAR_CODE;
		break;
	default:
		break;
	}
	if (code != Inset::NO_CODE && tli && !tli->insetAllowed(code))
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
		flag.setOnOff(TEXT(false)->cursor.par()->params().startOfAppendix());
		break;
	case LFUN_SWITCHBUFFER:
		// toggle on the current buffer, but do not toggle off
		// the other ones (is that a good idea?)
		if (ev.argument == buf->fileName())
			flag.setOnOff(true);
		break;
	case LFUN_TRACK_CHANGES:
		flag.setOnOff(buf->params.tracking_changes);
		break;
	default:
		break;
	}

	// the font related toggles
	if (!mathcursor) {
		LyXFont const & font = TEXT(false)->real_current_font;
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
		flag = getStatus(lyxaction.LookupFunc(token(ev.argument, ';', 0)));
	}

	return flag;
}


void LyXFunc::dispatch(string const & s, bool verbose)
{
	int const action = lyxaction.LookupFunc(s);

	if (action == LFUN_UNKNOWN_ACTION) {
#if USE_BOOST_FORMAT
boost::format fmt(_("Unknown function (%1$s)"));
fmt % s;
owner->message(fmt.str());
#else
		string const msg = string(_("Unknown function ("))
			+ s + ')';
		owner->message(msg);
#endif
		return;
	}

	dispatch(action, verbose);
}


void LyXFunc::dispatch(int ac, bool verbose)
{
	dispatch(lyxaction.retrieveActionArg(ac), verbose);
}



void LyXFunc::dispatch(FuncRequest const & ev, bool verbose)
{
	lyxerr[Debug::ACTION] << "LyXFunc::dispatch: action[" << ev.action
			      <<"] arg[" << ev.argument << ']' << endl;

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

	if (view()->available())
		view()->hideCursor();

	string argument = ev.argument;
	kb_action action = ev.action;

	// We cannot use this function here
	if (getStatus(ev).disabled()) {
		lyxerr[Debug::ACTION] << "LyXFunc::dispatch: "
		       << lyxaction.getActionName(action)
		       << " [" << action << "] is disabled at this location"
		       << endl;
		setErrorMessage(getStatusMessage());
		goto exit_with_message;
	}

	if (view()->available() && view()->theLockingInset()) {
		Inset::RESULT result;
		if ((action > 1) || ((action == LFUN_UNKNOWN_ACTION) &&
				     (!keyseq.deleted())))
		{
			UpdatableInset * inset = view()->theLockingInset();
#if 1
			int inset_x;
			int dummy_y;
			inset->getCursorPos(view(), inset_x, dummy_y);
#endif
			if ((action == LFUN_UNKNOWN_ACTION)
			    && argument.empty()) {
				argument = encoded_last_key;
			}
			// Undo/Redo is a bit tricky for insets.
			if (action == LFUN_UNDO) {
				view()->undo();
				goto exit_with_message;
			} else if (action == LFUN_REDO) {
				view()->redo();
				goto exit_with_message;
			} else if (((result=inset->
				     // Hand-over to inset's own dispatch:
				     localDispatch(FuncRequest(view(), action, argument))) ==
				    DISPATCHED) ||
				   (result == DISPATCHED_NOUPDATE))
				goto exit_with_message;
					// If UNDISPATCHED, just soldier on
			else if (result == FINISHED) {
				owner->clearMessage();
				goto exit_with_message;
				// We do not need special RTL handling here:
				// FINISHED means that the cursor should be
				// one position after the inset.
			} else if (result == FINISHED_RIGHT) {
				TEXT()->cursorRight(view());
				moveCursorUpdate(true, false);
				owner->clearMessage();
				goto exit_with_message;
			} else if (result == FINISHED_UP) {
				if (TEXT()->cursor.irow() != TEXT()->rows().begin()) {
#if 1
					TEXT()->setCursorFromCoordinates(
						TEXT()->cursor.ix() + inset_x,
						TEXT()->cursor.iy() -
						TEXT()->cursor.irow()->baseline() - 1);
					TEXT()->cursor.x_fix(TEXT()->cursor.x());
#else
					TEXT()->cursorUp(view());
#endif
					moveCursorUpdate(true, false);
				} else {
					view()->update(TEXT(), BufferView::SELECT);
				}
				owner->clearMessage();
				goto exit_with_message;
			} else if (result == FINISHED_DOWN) {
				if (boost::next(TEXT()->cursor.irow()) != TEXT()->rows().end()) {
#if 1
					TEXT()->setCursorFromCoordinates(
						TEXT()->cursor.ix() + inset_x,
						TEXT()->cursor.iy() -
						TEXT()->cursor.irow()->baseline() +
						TEXT()->cursor.irow()->height() + 1);
					TEXT()->cursor.x_fix(TEXT()->cursor.x());
#else
					TEXT()->cursorDown(view());
#endif
				} else {
					TEXT()->cursorRight(view());
				}
				moveCursorUpdate(true, false);
				owner->clearMessage();
				goto exit_with_message;
			}
#warning I am not sure this is still right, please have a look! (Jug 20020417)
			else { // result == UNDISPATCHED
				//setMessage(N_("Text mode"));
				switch (action) {
				case LFUN_UNKNOWN_ACTION:
				case LFUN_BREAKPARAGRAPH:
				case LFUN_BREAKLINE:
					TEXT()->cursorRight(view());
					view()->switchKeyMap();
					owner->view_state_changed();
					break;
				case LFUN_RIGHT:
					if (!TEXT()->cursor.par()->isRightToLeftPar(owner->buffer()->params)) {
						TEXT()->cursorRight(view());
						moveCursorUpdate(true, false);
						owner->view_state_changed();
					}
					goto exit_with_message;
				case LFUN_LEFT:
					if (TEXT()->cursor.par()->isRightToLeftPar(owner->buffer()->params)) {
						TEXT()->cursorRight(view());
						moveCursorUpdate(true, false);
						owner->view_state_changed();
					}
					goto exit_with_message;
				case LFUN_DOWN:
					if (boost::next(TEXT()->cursor.row()) != TEXT()->rows().end())
						TEXT()->cursorDown(view());
					else
						TEXT()->cursorRight(view());
					moveCursorUpdate(true, false);
					owner->view_state_changed();
					goto exit_with_message;
				default:
					break;
				}
			}
		}
	}

	switch (action) {

	case LFUN_ESCAPE:
	{
		if (!view()->available()) break;
		// this function should be used always [asierra060396]
		UpdatableInset * tli =
			view()->theLockingInset();
		if (tli) {
			UpdatableInset * lock = tli->getLockingInset();

			if (tli == lock) {
				view()->unlockInset(tli);
				TEXT()->cursorRight(view());
				moveCursorUpdate(true, false);
				owner->view_state_changed();
			} else {
				tli->unlockInsetInInset(view(),
							lock,
							true);
			}
			finishUndo();
			// Tell the paragraph dialog that we changed paragraph
			dispatch(FuncRequest(LFUN_PARAGRAPH_UPDATE));
		}
	}
	break;

		// --- Misc -------------------------------------------
	case LFUN_WORDFINDFORWARD  :
	case LFUN_WORDFINDBACKWARD : {
		static string last_search;
		string searched_string;

		if (!argument.empty()) {
			last_search = argument;
			searched_string = argument;
		} else {
			searched_string = last_search;
		}
		bool fw = (action == LFUN_WORDFINDFORWARD);
		if (!searched_string.empty()) {
			lyxfind::LyXFind(view(), searched_string, fw);
		}
//		view()->showCursor();
	}
	break;

	case LFUN_PREFIX:
	{
		if (view()->available() && !view()->theLockingInset()) {
			view()->update(TEXT(), BufferView::SELECT);
		}
		owner->message(keyseq.printOptions());
	}
	break;

	// --- Misc -------------------------------------------
	case LFUN_EXEC_COMMAND:
		owner->focus_command_buffer();
		break;

	case LFUN_CANCEL:                   // RVDK_PATCH_5
		keyseq.reset();
		meta_fake_bit = key_modifier::none;
		if (view()->available())
			// cancel any selection
			dispatch(LFUN_MARK_OFF);
		setMessage(N_("Cancel"));
		break;

	case LFUN_META_FAKE:                                 // RVDK_PATCH_5
	{
		meta_fake_bit = key_modifier::alt;
		setMessage(keyseq.print());
	}
	break;

	case LFUN_READ_ONLY_TOGGLE:
		if (owner->buffer()->lyxvc.inUse()) {
			owner->buffer()->lyxvc.toggleReadOnly();
		} else {
			owner->buffer()->setReadonly(
				!owner->buffer()->isReadonly());
		}
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
			ostringstream s1;
#if USE_BOOST_FORMAT
			s1 << boost::format(_("Saving document %1$s..."))
			   % MakeDisplayPath(owner->buffer()->fileName());
#else
			s1 << _("Saving document ")
			   << MakeDisplayPath(owner->buffer()->fileName())
			   << _("...");
#endif
			owner->message(STRCONV(s1.str()));
			MenuWrite(view(), owner->buffer());
			s1 << _(" done.");
			owner->message(STRCONV(s1.str()));
		} else
			WriteAs(view(), owner->buffer());
		break;

	case LFUN_WRITEAS:
		WriteAs(view(), owner->buffer(), argument);
		break;

	case LFUN_MENURELOAD: {
		string const file = MakeDisplayPath(view()->buffer()->fileName(), 20);
#if USE_BOOST_FORMAT
		boost::format fmt(_("Any changes will be lost. Are you sure you want to revert to the saved version of the document %1$s?"));
		fmt % file;
		string text = fmt.str();
#else
		string text = _("Any changes will be lost. Are you sure you want to revert to the saved version of the document");
		text += file + _("?");
#endif
		int const ret = Alert::prompt(_("Revert to saved document?"),
			text, 0, 1, _("&Revert"), _("&Cancel"));

		if (ret == 0)
			view()->reload();
		break;
	}

	case LFUN_UPDATE:
		Exporter::Export(owner->buffer(), argument, true);
		break;

	case LFUN_PREVIEW:
		Exporter::Preview(owner->buffer(), argument);
		break;

	case LFUN_BUILDPROG:
		Exporter::Export(owner->buffer(), "program", true);
		break;

	case LFUN_RUNCHKTEX:
		owner->buffer()->runChktex();
		break;

	case LFUN_MENUPRINT:
		owner->getDialogs().showPrint();
		break;

	case LFUN_EXPORT:
		if (argument == "custom")
			owner->getDialogs().showSendto();
		else
			Exporter::Export(owner->buffer(), argument, false);
		break;

	case LFUN_IMPORT:
		doImport(argument);
		break;

	case LFUN_QUIT:
		QuitLyX();
		break;

	case LFUN_TOCVIEW:
	{
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

	case LFUN_MENUSEARCH:
		owner->getDialogs().showSearch();
		break;

	case LFUN_REMOVEERRORS:
		if (view()->removeAutoInsets()) {
#warning repaint() or update() or nothing ?
			view()->repaint();
			view()->fitCursor();
		}
		break;

	case LFUN_DEPTH_MIN:
		changeDepth(view(), TEXT(false), DEC_DEPTH, false);
		owner->view_state_changed();
		break;

	case LFUN_DEPTH_PLUS:
		changeDepth(view(), TEXT(false), INC_DEPTH, false);
		owner->view_state_changed();
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
	case LFUN_HELP_ABOUTLYX:
		owner->getDialogs().show("about");
		break;

	case LFUN_HELP_TEXINFO:
		owner->getDialogs().showTexinfo();
		break;

	case LFUN_HELP_OPEN:
	{
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
		ostringstream str;
#if USE_BOOST_FORMAT
		str << boost::format(_("Opening help file %1$s..."))
		    % MakeDisplayPath(fname);
#else
		str << _("Opening help file ")
		    << MakeDisplayPath(fname) << _("...");
#endif
		owner->message(STRCONV(str.str()));
		view()->buffer(bufferlist.loadLyXFile(fname, false));
		break;
	}

		// --- version control -------------------------------
	case LFUN_VC_REGISTER:
	{
		if (!owner->buffer()->lyxvc.inUse())
			owner->buffer()->lyxvc.registrer();
	}
	break;

	case LFUN_VC_CHECKIN:
	{
		if (owner->buffer()->lyxvc.inUse()
		    && !owner->buffer()->isReadonly())
			owner->buffer()->lyxvc.checkIn();
	}
	break;

	case LFUN_VC_CHECKOUT:
	{
		if (owner->buffer()->lyxvc.inUse()
		    && owner->buffer()->isReadonly())
			owner->buffer()->lyxvc.checkOut();
	}
	break;

	case LFUN_VC_REVERT:
	{
		owner->buffer()->lyxvc.revert();
	}
	break;

	case LFUN_VC_UNDO:
	{
		owner->buffer()->lyxvc.undoLast();
	}
	break;

	case LFUN_VC_HISTORY:
	{
		owner->getDialogs().show("vclog");
		break;
	}

	// --- buffers ----------------------------------------

	case LFUN_SWITCHBUFFER:
		view()->buffer(bufferlist.getBuffer(argument));
		break;

	case LFUN_FILE_NEW:
	{
		// servercmd: argument must be <file>:<template>
		Buffer * tmpbuf = NewFile(argument);
		if (tmpbuf)
			view()->buffer(tmpbuf);
	}
	break;

	case LFUN_FILE_OPEN:
		open(argument);
		break;

	case LFUN_LATEX_LOG:
		owner->getDialogs().show("log");
		break;

	case LFUN_LAYOUT_DOCUMENT:
		owner->getDialogs().showDocument();
		break;

	case LFUN_LAYOUT_CHARACTER: {
		string data = freefont2string();
		if (!data.empty())
			owner->getDialogs().show("character", data);
		break;
	}

	case LFUN_LAYOUT_TABULAR:
	    if (view()->theLockingInset()) {
		if (view()->theLockingInset()->lyxCode()==Inset::TABULAR_CODE) {
		    InsetTabular * inset = static_cast<InsetTabular *>
			(view()->theLockingInset());
		    inset->openLayoutDialog(view());
		} else if (view()->theLockingInset()->
			   getFirstLockingInsetOfType(Inset::TABULAR_CODE)!=0) {
		    InsetTabular * inset = static_cast<InsetTabular *>(
			view()->theLockingInset()->getFirstLockingInsetOfType(Inset::TABULAR_CODE));
		    inset->openLayoutDialog(view());
		}
	    }
	    break;

	case LFUN_LAYOUT_PREAMBLE:
		owner->getDialogs().showPreamble();
		break;

	case LFUN_DROP_LAYOUTS_CHOICE:
		owner->getToolbar().openLayoutList();
		break;

	case LFUN_MENU_OPEN_BY_NAME:
		owner->getMenubar().openByName(argument);
		break; // RVDK_PATCH_5

	case LFUN_SPELLCHECK:
		owner->getDialogs().showSpellchecker();
		break;

	// --- lyxserver commands ----------------------------


	case LFUN_GETNAME:
		setMessage(owner->buffer()->fileName());
		lyxerr[Debug::INFO] << "FNAME["
			       << owner->buffer()->fileName()
			       << "] " << endl;
		break;

	case LFUN_NOTIFY:
	{
		dispatch_buffer = keyseq.print();
		lyxserver->notifyClient(dispatch_buffer);
	}
	break;

	case LFUN_GOTOFILEROW:
	{
		string file_name;
		int row;
		istringstream istr(argument.c_str());
		istr >> file_name >> row;
		// Must replace extension of the file to be .lyx and get full path
		string const s(ChangeExtension(file_name, ".lyx"));

		// Either change buffer or load the file
		if (bufferlist.exists(s)) {
			view()->buffer(bufferlist.getBuffer(s));
		} else {
			view()->buffer(bufferlist.loadLyXFile(s));
		}

		view()->setCursorFromRow(row);

		view()->center();
		// see BufferView_pimpl::center()
		view()->updateScrollbar();
	}
	break;

	case LFUN_GOTO_PARAGRAPH:
	{
		istringstream istr(argument.c_str());

		int id;
		istr >> id;
		Paragraph * par = owner->buffer()->getParFromID(id);
		if (par == 0) {
			lyxerr[Debug::INFO] << "No matching paragraph found! ["
					    << id << ']' << endl;
			break;
		} else {
			lyxerr[Debug::INFO] << "Paragraph " << par->id()
					    << " found." << endl;
		}

		if (view()->theLockingInset())
			view()->unlockInset(view()->theLockingInset());
		if (par->inInset()) {
			par->inInset()->edit(view());
		}
		// Set the cursor
		view()->getLyXText()->setCursor(par, 0);
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
	{
		setErrorMessage(N_("This is only allowed in math mode!"));
	}
	break;

	// passthrough hat and underscore outside mathed:
	case LFUN_SUBSCRIPT:
		dispatch(FuncRequest(view(), LFUN_SELFINSERT, "_"));
		break;
	case LFUN_SUPERSCRIPT:
		dispatch(FuncRequest(view(), LFUN_SELFINSERT, "^"));
		break;

	case LFUN_MATH_PANEL:
		owner->getDialogs().showMathPanel();
		break;

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
	}
	break;

	case LFUN_DIALOG_SHOW_NEXT_INSET: {
	}
	break;

	case LFUN_DIALOG_UPDATE: {
		string const & name = argument;
		// Can only update a dialog connected to an existing inset
		InsetBase * inset = owner->getDialogs().getOpenInset(name);
		if (inset) {
			FuncRequest fr(view(), LFUN_INSET_DIALOG_UPDATE,
				       ev.argument);
			inset->localDispatch(fr);
		} else if (name == "paragraph") {
			dispatch(FuncRequest(LFUN_PARAGRAPH_UPDATE));
		}
	}
	break;

	case LFUN_DIALOG_HIDE:
		Dialogs::hide(argument, 0);
		break;

	case LFUN_DIALOG_DISCONNECT_INSET:
		owner->getDialogs().disconnect(argument);
		break;

	case LFUN_CHILDOPEN:
	{
		string const filename =
			MakeAbsPath(argument,
				    owner->buffer()->filePath());
		setMessage(N_("Opening child document ") +
			   MakeDisplayPath(filename) + "...");
		view()->savePosition(0);
		if (bufferlist.exists(filename))
			view()->buffer(bufferlist.getBuffer(filename));
		else
			view()->buffer(bufferlist.loadLyXFile(filename));
	}
	break;

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

	case LFUN_SEQUENCE:
	{
		// argument contains ';'-terminated commands
		while (!argument.empty()) {
			string first;
			argument = split(argument, first, ';');
			dispatch(first);
		}
	}
	break;

	case LFUN_DIALOG_PREFERENCES:
		owner->getDialogs().showPreferences();
		break;

	case LFUN_SAVEPREFERENCES:
	{
		Path p(user_lyxdir);
		lyxrc.write("preferences");
	}
	break;

	case LFUN_SCREEN_FONT_UPDATE:
	{
		// handle the screen font changes.
		lyxrc.set_font_norm_type();
		lyx_gui::update_fonts();
		// We also need to empty the textcache so that
		// the buffer will be formatted correctly after
		// a zoom change.
		textcache.clear();
		// Of course we should only do the resize and the textcache.clear
		// if values really changed...but not very important right now. (Lgb)
		// All visible buffers will need resize
		view()->resize();
		view()->repaint();
	}
	break;

	case LFUN_SET_COLOR:
	{
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
#if USE_BOOST_FORMAT
			setErrorMessage(
				boost::io::str(
					boost::format(
						_("Set-color \"%1$s\" failed "
						  "- color is undefined or "
						  "may not be redefined"))
					% lyx_name));
#else
			setErrorMessage(_("Set-color ") + lyx_name
					+ _(" failed - color is undefined"
					    " or may not be redefined"));
#endif

			break;
		}

		lyx_gui::update_color(lcolor.getFromLyXName(lyx_name));

		if (graphicsbg_changed) {
#ifdef WITH_WARNINGS
#warning FIXME!! The graphics cache no longer has a changeDisplay method.
#endif
#if 0
			grfx::GCache & gc = grfx::GCache::get();
			gc.changeDisplay(true);
#endif
		}

		view()->repaint();
		break;
	}

	case LFUN_MESSAGE:
		owner->message(argument);
		break;

	case LFUN_FORKS_SHOW:
		owner->getDialogs().showForks();
		break;

	case LFUN_FORKS_KILL:
	{
		if (!isStrInt(argument))
			break;

		pid_t const pid = strToInt(argument);
		ForkedcallsController & fcc = ForkedcallsController::get();
		fcc.kill(pid);
		break;
	}

	case LFUN_TOOLTIPS_TOGGLE:
		owner->getDialogs().toggleTooltips();
		break;

	default:
		// Then if it was none of the above
		// Trying the BufferView::pimpl dispatch:
		if (!view()->dispatch(ev))
			lyxerr << "A truly unknown func ["
			       << lyxaction.getActionName(ev.action) << "]!"
			       << endl;
		break;
	} // end of switch

exit_with_message:

	view()->owner()->updateLayoutChoice();

	if (view()->available()) {
		view()->fitCursor();

		// If we executed a mutating lfun, mark the buffer as dirty
		if (!lyxaction.funcHasFlag(ev.action, LyXAction::NoBuffer)
		    && !lyxaction.funcHasFlag(ev.action, LyXAction::ReadOnly))
			view()->buffer()->markDirty();
	}

	sendDispatchMessage(getMessage(), ev, verbose);
}


void LyXFunc::sendDispatchMessage(string const & msg, FuncRequest const & ev, bool verbose)
{
	owner->updateMenubar();
	owner->updateToolbar();

	if (ev.action == LFUN_SELFINSERT || !verbose) {
		lyxerr[Debug::ACTION] << "dispatch msg is " << msg << endl;
		if (!msg.empty())
			owner->message(msg);
		return;
	}

	string dispatch_msg = msg;
	if (!dispatch_msg.empty())
		dispatch_msg += ' ';

	string comname = lyxaction.getActionName(ev.action);

	int pseudoaction = ev.action;
	bool argsadded = false;

	if (!ev.argument.empty()) {
		// the pseudoaction is useful for the bindings
		pseudoaction = lyxaction.searchActionArg(ev.action, ev.argument);

		if (pseudoaction == LFUN_UNKNOWN_ACTION) {
			pseudoaction = ev.action;
		} else {
			comname += ' ' + ev.argument;
			argsadded = true;
		}
	}

	string const shortcuts = toplevel_keymap->findbinding(pseudoaction);

	if (!shortcuts.empty()) {
		comname += ": " + shortcuts;
	} else if (!argsadded && !ev.argument.empty()) {
		comname += ' ' + ev.argument;
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

	view()->buffer(bufferlist.newFile(filename, templname, !name.empty()));
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
				  string(AddPath(system_lyxdir, "examples"))));

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
		Buffer * buffer =  bufferlist.newFile(filename, "", true);
		view()->buffer(buffer);
		return;
	}

	ostringstream str;
#if USE_BOOST_FORMAT
	str << boost::format(_("Opening document %1$s...")) % disp_fn;
#else
	str << _("Opening document ") << disp_fn << _("...");
#endif

	owner->message(STRCONV(str.str()));

	Buffer * openbuf = bufferlist.loadLyXFile(filename);
	ostringstream str2;
	if (openbuf) {
		view()->buffer(openbuf);
#if USE_BOOST_FORMAT
		str2 << boost::format(_("Document %1$s opened.")) % disp_fn;
#else
		str2 << _("Document ") << disp_fn << _(" opened.");
#endif
	} else {
#if USE_BOOST_FORMAT
		str2 << boost::format(_("Could not open document %1$s"))
			% disp_fn;
#else
		str2 << _("Could not open document ") << disp_fn;
#endif
	}
	owner->message(STRCONV(str2.str()));
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

#if USE_BOOST_FORMAT
		boost::format fmt(_("Select %1$s file to import"));
		fmt % formats.prettyName(format);
		string const text = fmt.str();
#else
		string const text = _("Select ") + formats.prettyName(format)
			+ _(" file to import");;
#endif

		FileDialog fileDlg(text,
			LFUN_IMPORT,
			make_pair(string(_("Documents|#o#O")),
				  string(lyxrc.document_path)),
			make_pair(string(_("Examples|#E#e")),
				  string(AddPath(system_lyxdir, "examples"))));

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

#if USE_BOOST_FORMAT
		boost::format fmt(_("The document %1$s already exists.\n\nDo you want to over-write that document?"));
		fmt % file;
		string text = fmt.str();
#else
		string text = _("The document ");
		text += file + _(" already exists.\n\nDo you want to over-write that document?");
#endif
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
			// need this otherwise SEGV may occur while trying to
			// set variables that don't exist
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
	lyx::Assert(owner);
	return owner->view().get();
}
