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
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "bufferview_funcs.h"

#include "author.h"
#include "buffer.h"
#include "bufferparams.h"
#include "BufferView.h"
#include "cursor.h"
#include "gettext.h"
#include "language.h"
#include "LColor.h"
#include "lyxlex.h"
#include "lyxrow.h"
#include "paragraph.h"
#include "ParagraphParameters.h"
#include "iterators.h"

#include "frontends/Alert.h"
#include "frontends/LyXView.h"

#include "insets/insettext.h"

#include "mathed/math_cursor.h"

#include "support/tostr.h"

#include "support/std_sstream.h"

using lyx::support::bformat;

using std::istringstream;
using std::ostringstream;
using std::string;


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


bool changeDepthAllowed(BufferView * bv, LyXText * text, DEPTH_CHANGE type)
{
	if (!bv->available() || !text)
		return false;

	return text->changeDepthAllowed(type);
}


void changeDepth(BufferView * bv, LyXText * text, DEPTH_CHANGE type)
{
	if (!bv->available() || !text)
		return;
	text->changeDepth(type);
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
	CursorSlice const & c = text->cursor();

	bool const show_change = buffer->params().tracking_changes
		&& text->cursor().pos() != text->cursorPar()->size()
		&& text->cursorPar()->lookupChange(c.pos()) != Change::UNCHANGED;

	if (show_change) {
		Change change = text->cursorPar()->lookupChangeFull(c.pos());
		Author const & a = bv->buffer()->params().authors().get(change.author);
		state << _("Change: ") << a.name();
		if (!a.email().empty())
			state << " (" << a.email() << ")";
		if (change.changetime)
			state << _(" at ") << ctime(&change.changetime);
		state << " : ";
	}

	// I think we should only show changes from the default
	// font. (Asger)
	LyXFont font = text->real_current_font;
	font.reduce(buffer->params().getLyXTextClass().defaultfont());

	// avoid _(...) re-entrance problem
	string const s = font.stateText(&buffer->params());
	state << bformat(_("Font: %1$s"), s);

	// state << bformat(_("Font: %1$s"), font.stateText(&buffer->params));

	// The paragraph depth
	int depth = text->getDepth();
	if (depth > 0)
		state << bformat(_(", Depth: %1$s"), tostr(depth));

	// The paragraph spacing, but only if different from
	// buffer spacing.
	if (!text->cursorPar()->params().spacing().isDefault()) {
		Spacing::Space cur_space =
			text->cursorPar()->params().spacing().getSpace();
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
			      << text->cursorPar()->params().spacing().getValue()
			      << ')';
			break;
		case Spacing::Default:
			// should never happen, do nothing
			break;
		}
	}
#ifdef DEVEL_VERSION
	ParagraphList::iterator pit = text->cursorPar();
	state << _(", Paragraph: ") << pit->id();
	state << _(", Position: ") << text->cursor().pos();
	RowList::iterator rit = pit->getRow(text->cursor().pos());
	state << bformat(_(", Row b:%1$d e:%2$d"), rit->pos(), rit->endpos());
	state << _(", Inset: ");
	InsetOld * inset = pit->inInset();
	if (inset)
		state << inset << " owner: " << inset->owner();
	else
		state << -1;
#endif
	return state.str();
}


// deletes a selection during an insertion
void replaceSelection(LyXText * text)
{
	if (text->bv()->selection().set()) {
		text->cutSelection(true, false);
		text->bv()->update();
	}
}

} // namespace bv_funcs
