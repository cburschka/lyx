/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "bufferview_funcs.h"
#include "LyXView.h"
#include "BufferView.h"
#include "paragraph.h"
#include "lyxfont.h"
#include "lyx_gui_misc.h"
#include "lyxtext.h"
#include "buffer.h"
#include "support/lstrings.h"
#include "lyx_cb.h"
#include "language.h"
#include "gettext.h"
#include "ParagraphParameters.h"

void Emph(BufferView * bv)
{
	LyXFont font(LyXFont::ALL_IGNORE);
	font.setEmph(LyXFont::TOGGLE);
	ToggleAndShow(bv, font);
}


void Bold(BufferView * bv)
{
	LyXFont font(LyXFont::ALL_IGNORE);
	font.setSeries(LyXFont::BOLD_SERIES);
	ToggleAndShow(bv, font);
}


void Noun(BufferView * bv)
{
	LyXFont font(LyXFont::ALL_IGNORE);
	font.setNoun(LyXFont::TOGGLE);
	ToggleAndShow(bv, font);
}


void Number(BufferView * bv)
{
	LyXFont font(LyXFont::ALL_IGNORE);
	font.setNumber(LyXFont::TOGGLE);
	ToggleAndShow(bv, font);
}

void Lang(BufferView * bv, string const & l)
{
	LyXFont font(LyXFont::ALL_IGNORE);
	Language const * lang = languages.getLanguage(l);
	if (lang) {
		font.setLanguage(lang);
		ToggleAndShow(bv, font);
	} else
		WriteAlert(_("Error! unknown language"),l);
}


void Tex(BufferView * bv)
{
	LyXFont font(LyXFont::ALL_IGNORE);
	font.setLatex (LyXFont::TOGGLE);
	ToggleAndShow(bv, font);
}


// Change environment depth.
// if decInc >= 0, increment depth
// if decInc <  0, decrement depth
void changeDepth(BufferView * bv, LyXText * text, int decInc)
{
	if (!bv->available() || !text)
	    return;
	
	bv->hideCursor();
	bv->update(bv->text, BufferView::SELECT|BufferView::FITCUR);
	if (decInc >= 0)
		text->incDepth(bv);
	else
		text->decDepth(bv);
	if (text->inset_owner)
	    bv->updateInset((Inset *)text->inset_owner, true);
	bv->update(bv->text, BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
	bv->owner()->message(_("Changed environment depth "
			       "(in possible range, maybe not)"));
}


// How should this actually work? Should it prohibit input in all BufferViews,
// or just in the current one? If "just the current one", then it should be
// placed in BufferView. If "all BufferViews" then LyXGUI (I think) should
// run "ProhibitInput" on all LyXViews which will run prohibitInput on all
// BufferViews. Or is it perhaps just the (input in) BufferViews in the
// current LyxView that should be prohibited (Lgb) (This applies to
// "AllowInput" as well.)
void ProhibitInput(BufferView * bv)
{
	bv->hideCursor();

	static Cursor cursor;
	static bool cursor_undefined = true;
   
	if (cursor_undefined){
		cursor = XCreateFontCursor(fl_get_display(), XC_watch);
		XFlush(fl_get_display());
		cursor_undefined = false;
	}
   
	/* set the cursor to the watch for all forms and the canvas */ 
	XDefineCursor(fl_get_display(), bv->owner()->getForm()->window, 
		      cursor);

	XFlush(fl_get_display());
	fl_deactivate_all_forms();
}


void AllowInput(BufferView * bv)
{
	/* reset the cursor from the watch for all forms and the canvas */
   
	XUndefineCursor(fl_get_display(), bv->owner()->getForm()->window);

	XFlush(fl_get_display());
	fl_activate_all_forms();
}


void Code(BufferView * bv)
{
	LyXFont font(LyXFont::ALL_IGNORE);
	font.setFamily(LyXFont::TYPEWRITER_FAMILY); // no good
	ToggleAndShow(bv, font);
}


void Sans(BufferView * bv)
{
	LyXFont font(LyXFont::ALL_IGNORE);
	font.setFamily(LyXFont::SANS_FAMILY);
	ToggleAndShow(bv, font);
}


void Roman(BufferView * bv)
{
	LyXFont font(LyXFont::ALL_IGNORE);
	font.setFamily(LyXFont::ROMAN_FAMILY);
	ToggleAndShow(bv, font);
}


void StyleReset(BufferView * bv)
{
	LyXFont font(LyXFont::ALL_INHERIT, ignore_language);
	ToggleAndShow(bv, font);
}


void Underline(BufferView * bv)
{
	LyXFont font(LyXFont::ALL_IGNORE);
	font.setUnderbar(LyXFont::TOGGLE);
	ToggleAndShow(bv, font);
}


void FontSize(BufferView * bv, string const & size)
{
	LyXFont font(LyXFont::ALL_IGNORE);
	font.setLyXSize(size);
	ToggleAndShow(bv, font);
}


// Returns the current font and depth as a message. 
string const CurrentState(BufferView * bv)
{
	ostringstream state;

	if (bv->available()) { 
		// I think we should only show changes from the default
		// font. (Asger)
		LyXText * text = bv->getLyXText();
		Buffer * buffer = bv->buffer();
		LyXFont font = text->real_current_font;
		LyXFont const & defaultfont =
			textclasslist
			.TextClass(buffer->params.textclass)
			.defaultfont();
		font.reduce(defaultfont);

		state << _("Font:") << ' '
		      << font.stateText(&buffer->params);
		
		// The paragraph depth
		int depth = text->getDepth();
		if (depth > 0)
			state << _(", Depth: ") << depth;
		
		// The paragraph spacing, but only if different from
		// buffer spacing.
		if (!text->cursor.par()->params().spacing().isDefault()) {
			Spacing::Space cur_space =
				text->cursor.par()->params().spacing().getSpace();
			state << _(", Spacing: ");

			switch (cur_space) {
			case Spacing::Single:
				state << _("Single");
				
				break;
			case Spacing::Onehalf:
				state << _("Onehalf");
				break;
			case Spacing::Double:
				state << _("Double");
				break;
			case Spacing::Other:
				state << _("Other (")
				      << text->cursor.par()->params().spacing().getValue()
				      << ")";
				break;
			case Spacing::Default:
				// should never happen, do nothing
				break;
			}
		}
	}
	return state.str().c_str();
}


/* -------> Does the actual toggle job of the XxxCB() calls above.
 * Also shows the current font state.
 */
void ToggleAndShow(BufferView * bv, LyXFont const & font, bool toggleall)
{
	if (bv->available()) { 
		if (bv->theLockingInset()) {
			bv->theLockingInset()->SetFont(bv, font, toggleall);
			return;
		}
		LyXText * text = bv->getLyXText();
		if (!text)
			return;

		bv->hideCursor();
		bv->update(text, BufferView::SELECT|BufferView::FITCUR);
		text->toggleFree(bv, font, toggleall);
		bv->update(text, BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);

		if (font.language() != ignore_language ||
		    font.latex() != LyXFont::IGNORE ||
		    font.number() != LyXFont::IGNORE)
		{
			LyXCursor & cursor = text->cursor;
			text->computeBidiTables(bv->buffer(), cursor.row());
			if (cursor.boundary() != 
			    text->isBoundary(bv->buffer(), cursor.par(), cursor.pos(),
					     text->real_current_font) )
				text->setCursor(bv, cursor.par(), cursor.pos(),
						false, !cursor.boundary());
		}
	}
}
