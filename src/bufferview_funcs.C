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
#include "coordcache.h"
#include "gettext.h"
#include "language.h"
#include "LColor.h"
#include "lyxlex.h"
#include "lyxrow.h"
#include "paragraph.h"
#include "ParagraphParameters.h"
#include "pariterator.h"

#include "frontends/Alert.h"
#include "frontends/LyXView.h"

#include "insets/insettext.h"

#include "support/convert.h"

#include <sstream>

using lyx::support::bformat;

using std::istringstream;
using std::ostringstream;
using std::string;


namespace bv_funcs {

// Set data using font and toggle
// If successful, returns true
bool font2string(LyXFont const & font, bool const toggle, string & data)
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
	   << "toggleall " << convert<string>(toggle);
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


// the next two should probably go elsewhere
// this give the position relative to (0, baseline) of outermost
// paragraph
Point coordOffset(DocIterator const & dit)
{
	int x = 0;
	int y = 0;

	// Contribution of nested insets
	for (size_t i = 1; i != dit.depth(); ++i) {
		CursorSlice const & sl = dit[i];
		int xx = 0;
		int yy = 0;
		sl.inset().getCursorPos(sl, xx, yy);
		x += xx;
		y += yy;
		//lyxerr << "LCursor::getPos, i: " << i << " x: " << xx << " y: " << y << endl;
	}

	// Add contribution of initial rows of outermost paragraph
	CursorSlice const & sl = dit[0];
	Paragraph const & par = sl.text()->getPar(sl.pit());
	y -= par.rows()[0].ascent();
	for (size_t rit = 0, rend = par.pos2row(sl.pos()); rit != rend; ++rit)
		y += par.rows()[rit].height();
	y += par.rows()[par.pos2row(sl.pos())].ascent();
	x += dit.bottom().text()->cursorX(dit.bottom());
	return Point(x,y);
}


Point getPos(DocIterator const & dit)
{
	CursorSlice const & bot = dit.bottom();
	CoordCache::InnerParPosCache const & cache = theCoords.getParPos().find(bot.text())->second;
	CoordCache::InnerParPosCache::const_iterator it = cache.find(bot.pit());
	if (it == cache.end()) {
		//lyxerr << "cursor out of view" << std::endl;
		return Point(-1, -1);
	}
	Point p = coordOffset(dit); // offset from outer paragraph
	p.y_ += it->second.y_;
	return p;
}


// this could be used elsewhere as well?
CurStatus status(BufferView const * bv, DocIterator const & dit)
{
	CoordCache::InnerParPosCache const & cache = theCoords.getParPos().find(dit.bottom().text())->second;

	if (cache.find(dit.bottom().pit()) != cache.end())
		return CUR_INSIDE;
	else if (dit.bottom().pit() < bv->anchor_ref())
		return CUR_ABOVE;
	else
		return CUR_BELOW;
}


} // namespace bv_funcs
