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
#include "author.h"
#include "changes.h"

#include "frontends/Alert.h"

#include "support/lstrings.h"

#include "BoostFormat.h"

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
	LyXFont font(LyXFont::ALL_INHERIT, ignore_language);
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

	if (!bv->available())
		return string();

	LyXText * text = bv->getLyXText();
	Buffer * buffer = bv->buffer();
	LyXCursor const & c(text->cursor);

	bool const show_change = buffer->params.tracking_changes
		&& c.pos() != c.par()->size()
		&& c.par()->lookupChange(c.pos()) != Change::UNCHANGED;

	if (show_change) {
		Change change(c.par()->lookupChangeFull(c.pos()));
		Author const & a(bv->buffer()->authors().get(change.author));
		state << _("Change: ") << a.name();
		if (!a.email().empty()) {
			state << " (" << a.email() << ")";
		}
		if (change.changetime)
			state << _(" at ") << ctime(&change.changetime);
		state << " : ";
	}
 
	// I think we should only show changes from the default
	// font. (Asger)
	LyXFont font = text->real_current_font;
	LyXFont const & defaultfont =
		buffer->params.getLyXTextClass().defaultfont();
	font.reduce(defaultfont);

#if USE_BOOST_FORMAT
	state << boost::format(_("Font: %1$s")) % font.stateText(&buffer->params);
#else
	state << _("Font: ") << font.stateText(&buffer->params);
#endif

	// The paragraph depth
	int depth = text->getDepth();
	if (depth > 0) {
#if USE_BOOST_FORMAT
		state << boost::format(_(", Depth: %1$d")) % depth;
#else
		state << _(", Depth: ") << depth;
#endif
	}


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
			state << _("OneHalf");
			break;
		case Spacing::Double:
			state << _("Double");
			break;
		case Spacing::Other:
			state << _("Other (")
			      << text->cursor.par()->params().spacing().getValue()
			      << ')';
			break;
		case Spacing::Default:
			// should never happen, do nothing
			break;
		}
	}
#ifdef DEVEL_VERSION
	state << _(", Paragraph: ") << text->cursor.par()->id();
#endif
	return STRCONV(state.str());
}


/* Does the actual toggle job of the calls above.
 * Also shows the current font state.
 */
void toggleAndShow(BufferView * bv, LyXFont const & font, bool toggleall)
{
	if (!bv->available())
		return;

	if (bv->theLockingInset()) {
		bv->theLockingInset()->setFont(bv, font, toggleall);
		return;
	}

	LyXText * text = bv->getLyXText();
	// FIXME: can this happen ??
	if (!text)
		return;

	bv->hideCursor();
	bv->update(text, BufferView::SELECT | BufferView::FITCUR);
	text->toggleFree(bv, font, toggleall);
	bv->update(text, BufferView::SELECT | BufferView::FITCUR | BufferView::CHANGE);

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
