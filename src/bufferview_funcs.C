/**
 * \file bufferview_funcs.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "bufferview_funcs.h"
#include "BufferView.h"
#include "paragraph.h"
#include "lyxfont.h"
#include "lyxlex.h"
#include "lyxtext.h"
#include "buffer.h"
#include "lyx_cb.h"
#include "language.h"
#include "gettext.h"
#include "ParagraphParameters.h"
#include "author.h"
#include "changes.h"

#include "frontends/LyXView.h"
#include "frontends/Alert.h"
#include "mathed/math_cursor.h"

#include "support/lstrings.h"
#include "Lsstream.h"

#include "insets/updatableinset.h"

#include "support/BoostFormat.h"

namespace {

LyXFont freefont(LyXFont::ALL_IGNORE);
bool toggleall(false);

}

namespace bv_funcs {

// Set data using font and toggle
// If successful, returns true
bool font2string(LyXFont const & font, bool toggle, string & data)
{
	string lang = "ignore";
	if (font.language())
		lang = font.language()->lang();

	ostringstream os;
	os << "family " << font.family() << '\n'
	   << "series " << font.series() << '\n'
	   << "shape " << font.shape() << '\n'
	   << "size " << font.size() << '\n'
	   << "emph " << font.emph() << '\n'
	   << "underbar " << font.underbar() << '\n'
	   << "noun " << font.noun() << '\n'
	   << "number " << font.number() << '\n'
	   << "color " << font.color() << '\n'
	   << "language " << lang << '\n'
	   << "toggleall " << tostr(toggle);
	data = os.str();
	return true;
}


// Set font and toggle using data
// If successful, returns true
bool string2font(string const & data, LyXFont & font, bool & toggle)
{
	istringstream is(data);
	LyXLex lex(0,0);
	lex.setStream(is);

	int nset = 0;
	while (lex.isOK()) {
		string token;
		if (lex.next())
			token = lex.getString();

		if (token.empty() || !lex.next())
			break;

		if (token == "family") {
			int const next = lex.getInteger();
			font.setFamily(LyXFont::FONT_FAMILY(next));

		} else if (token == "series") {
			int const next = lex.getInteger();
			font.setSeries(LyXFont::FONT_SERIES(next));

		} else if (token == "shape") {
			int const next = lex.getInteger();
			font.setShape(LyXFont::FONT_SHAPE(next));

		} else if (token == "size") {
			int const next = lex.getInteger();
			font.setSize(LyXFont::FONT_SIZE(next));

		} else if (token == "emph" || token == "underbar" ||
			   token == "noun" || token == "number") {

			int const next = lex.getInteger();
			LyXFont::FONT_MISC_STATE const misc =
				LyXFont::FONT_MISC_STATE(next);

			if (token == "emph")
			    font.setEmph(misc);
			else if (token == "underbar")
				font.setUnderbar(misc);
			else if (token == "noun")
				font.setNoun(misc);
			else if (token == "number")
				font.setNumber(misc);

		} else if (token == "color") {
			int const next = lex.getInteger();
			font.setColor(LColor::color(next));

		} else if (token == "language") {
			string const next = lex.getString();
			if (next == "ignore")
				font.setLanguage(ignore_language);
			else
				font.setLanguage(languages.getLanguage(next));

		} else if (token == "toggleall") {
			toggle = lex.getBool();

		} else {
			// Unrecognised token
			break;
		}

		++nset;
	}
	return (nset > 0);
}


string const freefont2string()
{
	string data;
	if (font2string(freefont, toggleall, data))
		return data;
	return string();
}


void update_and_apply_freefont(BufferView * bv, string const & data)
{
	LyXFont font;
	bool toggle;
	if (string2font(data, font, toggle)) {
		freefont = font;
		toggleall = toggle;
		apply_freefont(bv);
	}
}


void apply_freefont(BufferView * bv)
{
	toggleAndShow(bv, freefont, toggleall);
	bv->owner()->view_state_changed();
	bv->owner()->message(_("Character set"));
}


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
	if (!lang)
		return;

	font.setLanguage(lang);
	toggleAndShow(bv, font);
}


bool changeDepth(BufferView * bv, LyXText * text, DEPTH_CHANGE type, bool test_only)
{
	if (!bv->available() || !text)
	    return false;

	if (test_only)
		return text->changeDepth(type, true);

	bv->hideCursor();
	bv->update(BufferView::SELECT);
	bool const changed = text->changeDepth(type, false);
	if (text->inset_owner)
		bv->updateInset((Inset *)text->inset_owner);
	bv->update(BufferView::SELECT);
	return changed;
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
	if (!bv->available())
		return string();

	if (mathcursor)
		return mathcursor->info();

	ostringstream state;

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
	bv->update(text, BufferView::SELECT);
	text->toggleFree(font, toggleall);
	bv->update(text, BufferView::SELECT);

	if (font.language() != ignore_language ||
	    font.number() != LyXFont::IGNORE) {
		LyXCursor & cursor = text->cursor;
		text->computeBidiTables(bv->buffer(), cursor.row());
		if (cursor.boundary() !=
		    text->isBoundary(bv->buffer(), *cursor.par(), cursor.pos(),
				     text->real_current_font))
			text->setCursor(cursor.par(), cursor.pos(),
					false, !cursor.boundary());
	}
}

}; // namespace bv_funcs
