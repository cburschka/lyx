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
#include "frontends/LyXView.h"
#include "BufferView.h"
#include "paragraph.h"
#include "lyxfont.h"
#include "lyxtext.h"
#include "buffer.h"
#include "lyx_cb.h"
#include "language.h"
#include "gettext.h"
#include "ParagraphParameters.h"
#include "lyxtextclasslist.h"

#include "frontends/Alert.h"

#include "support/lstrings.h"

void emph(BufferView * bv)
{
	LyXFont font(LyXFont::ALL_IGNORE);
	font.setEmph(LyXFont::TOGGLE);
	toggleAndShow(bv, font);
}


void bold(BufferView * bv)
{
	LyXFont font(LyXFont::ALL_IGNORE);
	font.setSeries(LyXFont::BOLD_SERIES);
	toggleAndShow(bv, font);
}


void noun(BufferView * bv)
{
	LyXFont font(LyXFont::ALL_IGNORE);
	font.setNoun(LyXFont::TOGGLE);
	toggleAndShow(bv, font);
}


void number(BufferView * bv)
{
	LyXFont font(LyXFont::ALL_IGNORE);
	font.setNumber(LyXFont::TOGGLE);
	toggleAndShow(bv, font);
}

void lang(BufferView * bv, string const & l)
{
	LyXFont font(LyXFont::ALL_IGNORE);
	Language const * lang = languages.getLanguage(l);
	if (lang) {
		font.setLanguage(lang);
		toggleAndShow(bv, font);
	} else
		Alert::alert(_("Error! unknown language"),l);
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


void code(BufferView * bv)
{
	LyXFont font(LyXFont::ALL_IGNORE);
	font.setFamily(LyXFont::TYPEWRITER_FAMILY); // no good
	toggleAndShow(bv, font);
}


void sans(BufferView * bv)
{
	LyXFont font(LyXFont::ALL_IGNORE);
	font.setFamily(LyXFont::SANS_FAMILY);
	toggleAndShow(bv, font);
}


void roman(BufferView * bv)
{
	LyXFont font(LyXFont::ALL_IGNORE);
	font.setFamily(LyXFont::ROMAN_FAMILY);
	toggleAndShow(bv, font);
}


void styleReset(BufferView * bv)
{
#ifndef INHERIT_LANG
	LyXFont font(LyXFont::ALL_INHERIT, ignore_language);
#else
	LyXFont font(LyXFont::ALL_INHERIT);
#endif
	toggleAndShow(bv, font);
}


void underline(BufferView * bv)
{
	LyXFont font(LyXFont::ALL_IGNORE);
	font.setUnderbar(LyXFont::TOGGLE);
	toggleAndShow(bv, font);
}


void fontSize(BufferView * bv, string const & size)
{
	LyXFont font(LyXFont::ALL_IGNORE);
	font.setLyXSize(size);
	toggleAndShow(bv, font);
}


// Returns the current font and depth as a message.
string const currentState(BufferView * bv)
{
	ostringstream state;

	if (bv->available()) {
		// I think we should only show changes from the default
		// font. (Asger)
		LyXText * text = bv->getLyXText();
		Buffer * buffer = bv->buffer();
		LyXFont font = text->real_current_font;
		LyXFont const & defaultfont =
			textclasslist[buffer->params.textclass].defaultfont();
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
#ifdef DEVEL_VERSION
		state << _(", Paragraph: ") << text->cursor.par()->id();
#endif
	}
	return state.str().c_str();
}


/* Does the actual toggle job of the calls above.
 * Also shows the current font state.
 */
void toggleAndShow(BufferView * bv, LyXFont const & font, bool toggleall)
{
	if (bv->available()) {
		if (bv->theLockingInset()) {
			bv->theLockingInset()->setFont(bv, font, toggleall);
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
		    font.number() != LyXFont::IGNORE) {
			LyXCursor & cursor = text->cursor;
			text->computeBidiTables(bv->buffer(), cursor.row());
			if (cursor.boundary() !=
			    text->isBoundary(bv->buffer(), cursor.par(), cursor.pos(),
					     text->real_current_font))
				text->setCursor(bv, cursor.par(), cursor.pos(),
						false, !cursor.boundary());
		}
	}
}
