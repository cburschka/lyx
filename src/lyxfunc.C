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

#ifdef __GNUG__
#pragma implementation
#endif

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
#include "layout.h"
#include "bufferview_funcs.h"
#include "frontends/LyXView.h"
#include "frontends/lyx_gui.h"
#include "vspace.h"
#include "FloatList.h"
#include "converter.h"
#include "exporter.h"
#include "importer.h"
#include "TextCache.h"
#include "lyxfind.h"
#include "undo_funcs.h"
#include "ParagraphParameters.h"

#include "insets/inseturl.h"
#include "insets/insetlatexaccent.h"
#include "insets/insettoc.h"
#include "insets/insetref.h"
#include "insets/insetparent.h"
#include "insets/insetindex.h"
#include "insets/insetinclude.h"
#include "insets/insetbib.h"
#include "insets/insetcite.h"
#include "insets/insettext.h"
#include "insets/insetert.h"
#include "insets/insetexternal.h"
#include "insets/insetgraphics.h"
#include "insets/insetfoot.h"
#include "insets/insetmarginal.h"
#include "insets/insetminipage.h"
#include "insets/insetfloat.h"
#if 0
#include "insets/insetlist.h"
#include "insets/insettheorem.h"
#endif
#include "insets/insettabular.h"
#include "insets/insetcaption.h"

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

#include <boost/tuple/tuple.hpp>

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

extern BufferList bufferlist;
extern LyXServer * lyxserver;
extern bool selection_possible;

extern boost::scoped_ptr<kb_keymap> toplevel_keymap;

// (alkis)
extern tex_accent_struct get_accent(kb_action action);

extern void ShowLatexLog();


/* === globals =========================================================== */


LyXFunc::LyXFunc(LyXView * o)
	: owner(o),
	keyseq(toplevel_keymap.get(), toplevel_keymap.get()),
	cancel_meta_seq(toplevel_keymap.get(), toplevel_keymap.get())
{
	meta_fake_bit = key_modifier::none;
	lyx_dead_action = LFUN_NOACTION;
	lyx_calling_dead_action = LFUN_NOACTION;
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
		TEXT(flag)->setSelection(view());
		if (TEXT(flag)->bv_owner)
		    view()->toggleToggle();
	}
	view()->update(TEXT(flag), BufferView::SELECT|BufferView::FITCUR);
	view()->showCursor();

	view()->switchKeyMap();
}


void LyXFunc::handleKeyFunc(kb_action action)
{
	char c = keyseq.getLastKeyEncoded();

	if (keyseq.length() > 1) {
		c = 0;
	}

	owner->getIntl().getTransManager()
		.deadkey(c, get_accent(action).accent, TEXT(false));
	// Need to clear, in case the minibuffer calls these
	// actions
	keyseq.clear();
	// copied verbatim from do_accent_char
	view()->update(TEXT(false),
	       BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
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
		return;
	}

	// Do a one-deep top-level lookup for
	// cancel and meta-fake keys. RVDK_PATCH_5
	cancel_meta_seq.reset();

	int action = cancel_meta_seq.addkey(keysym, state);
	if (lyxerr.debugging(Debug::KEY)) {
		lyxerr << "action first set to [" << action << "]" << endl;
	}

	// When not cancel or meta-fake, do the normal lookup.
	// Note how the meta_fake Mod1 bit is OR-ed in and reset afterwards.
	// Mostly, meta_fake_bit = key_modifier::none. RVDK_PATCH_5.
	if ((action != LFUN_CANCEL) && (action != LFUN_META_FAKE)) {
		// remove Caps Lock and Mod2 as a modifiers
		action = keyseq.addkey(keysym, (state | meta_fake_bit));
		lyxerr[Debug::KEY] << "action now set to ["
			<< action << "]" << endl;
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
		       << keyseq.print() << "]"
		       << endl;
	}

	// already here we know if it any point in going further
	// why not return already here if action == -1 and
	// num_bytes == 0? (Lgb)

	if (keyseq.length() > 1) {
		owner->message(keyseq.print());
	}

	if (action == LFUN_UNKNOWN_ACTION) {
		// It is unknown, but what if we remove all
		// the modifiers? (Lgb)
		action = keyseq.addkey(keysym, key_modifier::none);

		lyxerr[Debug::KEY] << "Removing modifiers...\n"
			<< "Action now set to ["
			<< action << "]" << endl;

		if (action == LFUN_UNKNOWN_ACTION) {
			owner->message(_("Unknown function."));
			return;
		}
	}

	if (action == LFUN_SELFINSERT) {
		char c = keysym->getISOEncoded();
		string argument;

		if (c != 0)
			argument = c;

		dispatch(FuncRequest(view(), LFUN_SELFINSERT, argument));
		lyxerr[Debug::KEY] << "SelfInsert arg[`"
				   << argument << "']" << endl;
	} else {
		dispatch(action);
	}
}


FuncStatus LyXFunc::getStatus(int ac) const
{
	kb_action action;
	string arg;
	boost::tie(action, arg) = lyxaction.retrieveActionArg(ac);
	return getStatus(FuncRequest(view(), action, arg));
}


FuncStatus LyXFunc::getStatus(FuncRequest const & ev) const
{
	FuncStatus flag;
	Buffer * buf = owner->buffer();

	if (ev.action == LFUN_NOACTION) {
		setStatusMessage(N_("Nothing to do"));
		return flag.disabled(true);
	}

	if (ev.action == LFUN_UNKNOWN_ACTION) {
		setStatusMessage(N_("Unknown action"));
		return flag.unknown(true);
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
		disable = ev.argument == "fax" &&
			!Exporter::IsExportable(buf, ev.argument);
		break;
	case LFUN_UNDO:
		disable = buf->undostack.empty();
		break;
	case LFUN_REDO:
		disable = buf->redostack.empty();
		break;
#ifndef HAVE_LIBAIKSAURUS
	case LFUN_THESAURUS_ENTRY:
		disable = true;
		break;
#endif
	case LFUN_RUNCHKTEX:
		disable = lyxrc.chktex_command == "none";
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

	case LFUN_LAYOUT:
	case LFUN_LAYOUT_PARAGRAPH: {
		Inset * inset = TEXT(false)->cursor.par()->inInset();
		disable = inset && inset->forceDefaultParagraphs(inset);
		break;
	}

	case LFUN_TABULAR_FEATURE:
		disable = true;
		if (tli) {
			FuncStatus ret;
			//ret.disabled(true);
			if (tli->lyxCode() == Inset::TABULAR_CODE) {
				ret = static_cast<InsetTabular *>(tli)
					->getStatus(ev.argument);
			} else if (tli->getFirstLockingInsetOfType(Inset::TABULAR_CODE)) {
				ret = static_cast<InsetTabular *>
					(tli->getFirstLockingInsetOfType(Inset::TABULAR_CODE))
					->getStatus(ev.argument);
			}
			flag |= ret;
			disable = false;
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
	case LFUN_BOOKMARK_GOTO:
		disable =  !view()->
			isSavedPosition(strToUnsignedInt(ev.argument));
		break;
	case LFUN_INSET_TOGGLE: {
		LyXText * lt = view()->getLyXText();
		disable = !(isEditableInset(lt->getInset())
			    || (lt->inset_owner
				&& lt->inset_owner->owner()
				&& lt->inset_owner->owner()->isOpen()));
		break;
	}
	case LFUN_LATEX_LOG:
		disable = !IsFileReadable(buf->getLogName().second);
		break;
	case LFUN_MATH_VALIGN:
		if (mathcursor && mathcursor->formula()->hullType() != "simple") {
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
		break;

	case LFUN_MATH_HALIGN:
		if (mathcursor && mathcursor->formula()->hullType() != "simple") {
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
		} else
			disable = true;
		break;

	case LFUN_MATH_MUTATE:
		if (tli && (tli->lyxCode() == Inset::MATH_CODE))
			flag.setOnOff(mathcursor->formula()->hullType() == ev.argument);
		else
			disable = true;
		break;

	// we just need to be in math mode to enable that
	case LFUN_MATH_SIZE:
	case LFUN_MATH_SPACE:
	case LFUN_MATH_LIMITS:
	case LFUN_MATH_NONUMBER:
	case LFUN_MATH_NUMBER:
		disable = !mathcursor;
		break;

	// we need to be math mode and a math array for that
	// Hack: halign produces non-zero result iff we are in a math array
	case LFUN_MATH_ROW_INSERT:
	case LFUN_MATH_ROW_DELETE:
	case LFUN_MATH_COLUMN_INSERT:
	case LFUN_MATH_COLUMN_DELETE:
		disable = !mathcursor || !mathcursor->halign() ||
			mathcursor->formula()->hullType() == "simple";
		break;

	default:
		break;
	}

	// the functions which insert insets
	Inset::Code code = Inset::NO_CODE;
	switch (ev.action) {
	case LFUN_INSET_ERT:
		code = Inset::ERT_CODE;
		break;
	case LFUN_INSET_GRAPHICS:
		code = Inset::GRAPHICS_CODE;
		break;
	case LFUN_INSET_FOOTNOTE:
		code = Inset::FOOT_CODE;
		break;
	case LFUN_TABULAR_INSERT:
		code = Inset::TABULAR_CODE;
		break;
	case LFUN_INSET_EXTERNAL:
		code = Inset::EXTERNAL_CODE;
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
	case LFUN_REF_INSERT:
		code = Inset::REF_CODE;
		break;
	case LFUN_CITATION_CREATE:
	case LFUN_CITATION_INSERT:
		code = Inset::CITE_CODE;
		break;
	case LFUN_INSERT_BIBTEX:
		code = Inset::BIBTEX_CODE;
		break;
	case LFUN_INDEX_INSERT:
		code = Inset::INDEX_CODE;
		break;
	case LFUN_INDEX_PRINT:
		code = Inset::INDEX_PRINT_CODE;
		break;
	case LFUN_CHILD_INSERT:
		code = Inset::INCLUDE_CODE;
		break;
	case LFUN_TOC_INSERT:
		code = Inset::TOC_CODE;
		break;
	case LFUN_PARENTINSERT:
		code = Inset::PARENT_CODE;
		break;
	case LFUN_HTMLURL:
	case LFUN_URL:
	case LFUN_INSERT_URL:
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
	if (code != Inset::NO_CODE && tli && !tli->insetAllowed(code)) {
		disable = true;
	}

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
		string const msg = string(_("Unknown function ("))
			+ s + ")";
		owner->message(msg);
		return;
	}

	dispatch(action, verbose);
}


void LyXFunc::dispatch(int ac, bool verbose)
{
	kb_action action;
	string arg;
	boost::tie(action, arg) = lyxaction.retrieveActionArg(ac);
	dispatch(FuncRequest(view(), action, arg), verbose);
}



void LyXFunc::dispatch(FuncRequest const & ev, bool verbose)
{
	lyxerr[Debug::ACTION] << "LyXFunc::dispatch: action[" << ev.action
			      <<"] arg[" << ev.argument << "]" << endl;

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
		UpdatableInset::RESULT result;
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
				argument = keyseq.getLastKeyEncoded();
			}
			// Undo/Redo is a bit tricky for insets.
			if (action == LFUN_UNDO) {
				view()->menuUndo();
				goto exit_with_message;
			} else if (action == LFUN_REDO) {
				view()->menuRedo();
				goto exit_with_message;
			} else if (((result=inset->
				     // Hand-over to inset's own dispatch:
				     localDispatch(FuncRequest(view(), action, argument))) ==
				    UpdatableInset::DISPATCHED) ||
				   (result == UpdatableInset::DISPATCHED_NOUPDATE))
				goto exit_with_message;
					// If UNDISPATCHED, just soldier on
			else if (result == UpdatableInset::FINISHED) {
				goto exit_with_message;
				// We do not need special RTL handling here:
				// FINISHED means that the cursor should be
				// one position after the inset.
			} else if (result == UpdatableInset::FINISHED_RIGHT) {
				TEXT()->cursorRight(view());
				moveCursorUpdate(true, false);
				owner->view_state_changed();
				goto exit_with_message;
			} else if (result == UpdatableInset::FINISHED_UP) {
				if (TEXT()->cursor.irow()->previous()) {
#if 1
					TEXT()->setCursorFromCoordinates(
						view(), TEXT()->cursor.ix() + inset_x,
						TEXT()->cursor.iy() -
						TEXT()->cursor.irow()->baseline() - 1);
					TEXT()->cursor.x_fix(TEXT()->cursor.x());
#else
					TEXT()->cursorUp(view());
#endif
					moveCursorUpdate(true, false);
					owner->view_state_changed();
				} else {
					view()->update(TEXT(), BufferView::SELECT|BufferView::FITCUR);
				}
				goto exit_with_message;
			} else if (result == UpdatableInset::FINISHED_DOWN) {
				if (TEXT()->cursor.irow()->next()) {
#if 1
					TEXT()->setCursorFromCoordinates(
						view(), TEXT()->cursor.ix() + inset_x,
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
				owner->view_state_changed();
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
					if (TEXT()->cursor.row()->next())
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
			owner->getDialogs().updateParagraph();
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
		bool fw = (action == LFUN_WORDFINDBACKWARD);
		if (!searched_string.empty()) {
			lyxfind::LyXFind(view(), searched_string, fw);
		}
//		view()->showCursor();
	}
	break;

	case LFUN_PREFIX:
	{
		if (view()->available() && !view()->theLockingInset()) {
			view()->update(TEXT(),
					      BufferView::SELECT|BufferView::FITCUR);
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
			s1 << _("Saving document") << ' '
			   << MakeDisplayPath(owner->buffer()->fileName() + "...");
			owner->message(s1.str().c_str());
			MenuWrite(view(), owner->buffer());
			s1 << _(" done.");
			owner->message(s1.str().c_str());
		} else
			WriteAs(view(), owner->buffer());
		break;

	case LFUN_WRITEAS:
		WriteAs(view(), owner->buffer(), argument);
		break;

	case LFUN_MENURELOAD:
		reloadBuffer();
		break;

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
		MenuRunChktex(owner->buffer());
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
#if 0
	case LFUN_LOFVIEW:
	case LFUN_LOTVIEW:
	case LFUN_LOAVIEW:
#endif
	{
		InsetCommandParams p;

#if 0
		if (action == LFUN_TOCVIEW)
#endif
			p.setCmdName("tableofcontents");
#if 0
		else if (action == LFUN_LOAVIEW)
			p.setCmdName("listof{algorithm}{List of Algorithms}");
		else if (action == LFUN_LOFVIEW)
			p.setCmdName("listoffigures");
		else
			p.setCmdName("listoftables");
#endif
		owner->getDialogs().createTOC(p.getAsString());
		break;
	}

	case LFUN_AUTOSAVE:
		AutoSave(view());
		break;

	case LFUN_UNDO:
		view()->menuUndo();
		break;

	case LFUN_REDO:
		view()->menuRedo();
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
		changeDepth(view(), TEXT(false), -1);
		break;

	case LFUN_DEPTH_PLUS:
		changeDepth(view(), TEXT(false), 1);
		break;

	case LFUN_FREE:
		owner->getDialogs().setUserFreeFont();
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
		owner->getDialogs().showAboutlyx();
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
		owner->prohibitInput();
		string const fname = i18nLibFileSearch("doc", arg, "lyx");
		if (fname.empty()) {
			lyxerr << "LyX: unable to find documentation file `"
			       << arg << "'. Bad installation?" << endl;
			owner->allowInput();
			break;
		}
		ostringstream str;
		str << _("Opening help file") << ' '
		    << MakeDisplayPath(fname) << "...";
		owner->message(str.str().c_str());
		view()->buffer(bufferlist.loadLyXFile(fname, false));
		owner->allowInput();
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
		owner->getDialogs().showVCLogFile();
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
		owner->getDialogs().showLogFile();
		break;

	case LFUN_LAYOUT_DOCUMENT:
		owner->getDialogs().showDocument();
		break;

	case LFUN_LAYOUT_PARAGRAPH:
		owner->getDialogs().showParagraph();
		break;

	case LFUN_LAYOUT_CHARACTER:
		owner->getDialogs().showCharacter();
		break;

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
					    << id << "]" << endl;
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
		view()->getLyXText()->setCursor(view(), par, 0);
		view()->switchKeyMap();
		owner->view_state_changed();

		view()->center();
		// see BufferView_pimpl::center()
		view()->updateScrollbar();
	}
	break;

	case LFUN_APROPOS:
	case LFUN_GETTIP:
	{
		int const qa = lyxaction.LookupFunc(argument);
		setMessage(lyxaction.helpText(static_cast<kb_action>(qa)));
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

	case LFUN_CITATION_CREATE:
	{
		InsetCommandParams p("cite");

		if (!argument.empty()) {
			// This should be set at source, ie when typing
			// "citation-insert foo" in the minibuffer.
			// Question: would pybibliographer also need to be
			// changed. Suspect so. Leave as-is therefore.
			if (contains(argument, "|")) {
				p.setContents(token(argument, '|', 0));
				p.setOptions(token(argument, '|', 1));
			} else {
				p.setContents(argument);
			}
			dispatch(FuncRequest(view(), LFUN_CITATION_INSERT, p.getAsString()));
		} else
			owner->getDialogs().createCitation(p.getAsString());
	}
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

	case LFUN_KMAP_OFF:		// keymap off
		owner->getIntl().KeyMapOn(false);
		break;

	case LFUN_KMAP_PRIM:	// primary keymap
		owner->getIntl().KeyMapPrim();
		break;

	case LFUN_KMAP_SEC:		// secondary keymap
		owner->getIntl().KeyMapSec();
		break;

	case LFUN_KMAP_TOGGLE:	// toggle keymap
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
			static string const err1 (N_("Set-color \""));
			static string const err2 (
				N_("\" failed - color is undefined "
				   "or may not be redefined"));
			setErrorMessage(_(err1) + lyx_name + _(err2));
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
		dispatch_msg += " ";

	string comname = lyxaction.getActionName(ev.action);

	int pseudoaction = ev.action;
	bool argsadded = false;

	if (!ev.argument.empty()) {
		// the pseudoaction is useful for the bindings
		pseudoaction = lyxaction.searchActionArg(ev.action, ev.argument);

		if (pseudoaction == LFUN_UNKNOWN_ACTION) {
			pseudoaction = ev.action;
		} else {
			comname += " " + ev.argument;
			argsadded = true;
		}
	}

	string const shortcuts = toplevel_keymap->findbinding(pseudoaction);

	if (!shortcuts.empty()) {
		comname += ": " + shortcuts;
	} else if (!argsadded && !ev.argument.empty()) {
		comname += " " + ev.argument;
	}

	if (!comname.empty()) {
		comname = rtrim(comname);
		dispatch_msg += "(" + comname + ')';
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
		FileDialog fileDlg(owner, _("Select template file"),
			LFUN_SELECT_FILE_SYNC,
			make_pair(string(_("Documents|#o#O")),
				  string(lyxrc.document_path)),
			make_pair(string(_("Templates|#T#t")),
				  string(lyxrc.template_path)));

		FileDialog::Result result =
			fileDlg.Select(lyxrc.template_path,
				       _("*.lyx|LyX Documents (*.lyx)"));

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
		FileDialog fileDlg(owner, _("Select document to open"),
			LFUN_FILE_OPEN,
			make_pair(string(_("Documents|#o#O")),
				  string(lyxrc.document_path)),
			make_pair(string(_("Examples|#E#e")),
				  string(AddPath(system_lyxdir, "examples"))));

		FileDialog::Result result =
			fileDlg.Select(initpath,
				       "*.lyx|LyX Documents (*.lyx)");

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
		if (!Alert::askQuestion(_("No such file"), disp_fn,
			_("Start a new document with this filename ?"))) {
			owner->message(_("Canceled"));
			return;
		}
		// the user specifically chose this name. Believe them.
		Buffer * buffer =  bufferlist.newFile(filename, "", true);
		view()->buffer(buffer);
		return;
	}

	ostringstream str;
	str << _("Opening document") << ' ' << disp_fn << "...";

	owner->message(str.str().c_str());

	Buffer * openbuf = bufferlist.loadLyXFile(filename);
	if (openbuf) {
		view()->buffer(openbuf);
		ostringstream str;
		str << _("Document") << ' ' << disp_fn << ' ' << _("opened.");
		owner->message(str.str().c_str());
	} else {
		ostringstream str;
		str << _("Could not open document") << ' ' << disp_fn;
		owner->message(str.str().c_str());
	}
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

		string const text = _("Select ") + formats.prettyName(format)
			+ _(" file to import");

		FileDialog fileDlg(owner, text,
			LFUN_IMPORT,
			make_pair(string(_("Documents|#o#O")),
				  string(lyxrc.document_path)),
			make_pair(string(_("Examples|#E#e")),
				  string(AddPath(system_lyxdir, "examples"))));

		string const extension = "*." + formats.extension(format)
			+ "| " + formats.prettyName(format)
			+ " (*." + formats.extension(format) + ")";

		FileDialog::Result result = fileDlg.Select(initpath,
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
	if (lyxrc.use_gui && bufferlist.exists(lyxfile)) {
		switch (Alert::askConfirmation(_("Document is already open:"),
					MakeDisplayPath(lyxfile, 50),
					_("Do you want to close that document now?\n"
					  "('No' will just switch to the open version)")))
			{
			case 1:
				// If close is canceled, we cancel here too.
				if (!bufferlist.close(bufferlist.getBuffer(lyxfile)))
					return;
				break;
			case 2:
				view()->buffer(bufferlist.getBuffer(lyxfile));
				return;
			case 3:
				owner->message(_("Canceled."));
				return;
			}
	}

	// if the file exists already, and we didn't do
	// -i lyx thefile.lyx, warn
	if (FileInfo(lyxfile, true).exist() && filename != lyxfile) {
		if (!Alert::askQuestion(_("A document by the name"),
			MakeDisplayPath(lyxfile), _("already exists. Overwrite?"))) {
			owner->message(_("Canceled"));
			return;
		}
	}

	Importer::Import(owner, filename, format);
}


void LyXFunc::reloadBuffer()
{
	string const fn = owner->buffer()->fileName();
	if (bufferlist.close(owner->buffer()))
		view()->buffer(bufferlist.loadLyXFile(fn));
}


void LyXFunc::closeBuffer()
{
	if (bufferlist.close(owner->buffer()) && !quitting) {
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
	return owner->view().get();
}
