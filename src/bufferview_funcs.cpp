/**
 * \file bufferview_funcs.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author John Levon
 * \author Angus Leeming
 * \author Juergen Vigna
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "bufferview_funcs.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "Cursor.h"
#include "CoordCache.h"
#include "gettext.h"
#include "Language.h"
#include "Color.h"
#include "Lexer.h"

#include "frontends/alert.h"

#include "insets/InsetCommand.h"
#include "insets/InsetText.h"

#include "support/convert.h"

#include <sstream>

using std::istringstream;
using std::ostringstream;
using std::string;
using std::vector;
using std::find;


namespace lyx {

using support::bformat;

namespace bv_funcs {

// Set data using font and toggle
// If successful, returns true
bool font2string(Font const & font, bool const toggle, string & data)
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
bool string2font(string const & data, Font & font, bool & toggle)
{
	istringstream is(data);
	Lexer lex(0,0);
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
			font.setFamily(Font::FONT_FAMILY(next));

		} else if (token == "series") {
			int const next = lex.getInteger();
			font.setSeries(Font::FONT_SERIES(next));

		} else if (token == "shape") {
			int const next = lex.getInteger();
			font.setShape(Font::FONT_SHAPE(next));

		} else if (token == "size") {
			int const next = lex.getInteger();
			font.setSize(Font::FONT_SIZE(next));

		} else if (token == "emph" || token == "underbar" ||
			   token == "noun" || token == "number") {

			int const next = lex.getInteger();
			Font::FONT_MISC_STATE const misc =
				Font::FONT_MISC_STATE(next);

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
			font.setColor(Color::color(next));

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
Point coordOffset(BufferView const & bv, DocIterator const & dit,
		bool boundary)
{
	int x = 0;
	int y = 0;
	int lastw = 0;

	// Addup contribution of nested insets, from inside to outside,
 	// keeping the outer paragraph for a special handling below
	for (size_t i = dit.depth() - 1; i >= 1; --i) {
		CursorSlice const & sl = dit[i];
		int xx = 0;
		int yy = 0;
		
		// get relative position inside sl.inset()
		sl.inset().cursorPos(bv, sl, boundary && ((i+1) == dit.depth()), xx, yy);
		
		// Make relative position inside of the edited inset relative to sl.inset()
		x += xx;
		y += yy;
		
		// In case of an RTL inset, the edited inset will be positioned to the left
		// of xx:yy
		if (sl.text()) {
			bool boundary_i = boundary && i + 1 == dit.depth();
			bool rtl = bv.textMetrics(sl.text()).isRTL(sl, boundary_i);
			if (rtl)
				x -= lastw;
		}

		// remember width for the case that sl.inset() is positioned in an RTL inset
		if (i && dit[i - 1].text()) {
			// If this Inset is inside a Text Inset, retrieve the Dimension
			// from the containing text instead of using Inset::dimension() which
			// might not be implemented.
			// FIXME (Abdel 23/09/2007): this is a bit messy because of the
			// elimination of Inset::dim_ cache. This coordOffset() method needs
			// to be rewritten in light of the new design.
			Dimension const & dim = bv.parMetrics(dit[i - 1].text(),
				dit[i - 1].pit()).insetDimension(&sl.inset());
			lastw = dim.wid;
		} else {
			Dimension const dim = sl.inset().dimension(bv);
			lastw = dim.wid;
		}
		
		//lyxerr << "Cursor::getPos, i: "
		// << i << " x: " << xx << " y: " << y << endl;
	}

	// Add contribution of initial rows of outermost paragraph
	CursorSlice const & sl = dit[0];
	TextMetrics const & tm = bv.textMetrics(sl.text());
	ParagraphMetrics const & pm = tm.parMetrics(sl.pit());
	BOOST_ASSERT(!pm.rows().empty());
	y -= pm.rows()[0].ascent();
#if 1
	// FIXME: document this mess
	size_t rend;
	if (sl.pos() > 0 && dit.depth() == 1) {
		int pos = sl.pos();
		if (pos && boundary)
			--pos;
//		lyxerr << "coordOffset: boundary:" << boundary << " depth:" << dit.depth() << " pos:" << pos << " sl.pos:" << sl.pos() << std::endl;
		rend = pm.pos2row(pos);
	} else
		rend = pm.pos2row(sl.pos());
#else
	size_t rend = pm.pos2row(sl.pos());
#endif
	for (size_t rit = 0; rit != rend; ++rit)
		y += pm.rows()[rit].height();
	y += pm.rows()[rend].ascent();
	
	TextMetrics const & bottom_tm = bv.textMetrics(dit.bottom().text());
	
	// Make relative position from the nested inset now bufferview absolute.
	int xx = bottom_tm.cursorX(dit.bottom(), boundary && dit.depth() == 1);
	x += xx;
	
	// In the RTL case place the nested inset at the left of the cursor in 
	// the outer paragraph
	bool boundary_1 = boundary && 1 == dit.depth();
	bool rtl = bottom_tm.isRTL(dit.bottom(), boundary_1);
	if (rtl)
		x -= lastw;
	
	return Point(x, y);
}


Point getPos(BufferView const & bv, DocIterator const & dit, bool boundary)
{
	CursorSlice const & bot = dit.bottom();
	TextMetrics const & tm = bv.textMetrics(bot.text());
	if (!tm.has(bot.pit()))
		return Point(-1, -1);

	Point p = coordOffset(bv, dit, boundary); // offset from outer paragraph
	p.y_ += tm.parMetrics(bot.pit()).position();
	return p;
}

} // namespace bv_funcs


} // namespace lyx
