/**
 * \file bufferview_funcs.C
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

#include "insets/insetcommand.h"
#include "insets/insettext.h"

#include "support/convert.h"

#include <sstream>

using lyx::support::bformat;

using std::istringstream;
using std::ostringstream;
using std::string;
using std::vector;


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
Point coordOffset(DocIterator const & dit, bool boundary)
{
	int x = 0;
	int y = 0;

	// Contribution of nested insets
	for (size_t i = 1; i != dit.depth(); ++i) {
		CursorSlice const & sl = dit[i];
		int xx = 0;
		int yy = 0;
		if (sl.inset().editable() == InsetBase::HIGHLY_EDITABLE)
			sl.inset().cursorPos(sl, boundary && ((i+1) == dit.depth()), xx, yy);
		x += xx;
		y += yy;
		//lyxerr << "LCursor::getPos, i: "
		// << i << " x: " << xx << " y: " << y << endl;
	}

	// Add contribution of initial rows of outermost paragraph
	CursorSlice const & sl = dit[0];
	Paragraph const & par = sl.text()->getPar(sl.pit());
	y -= par.rows()[0].ascent();
#if 1
	size_t rend;
	if (sl.pos() > 0 && dit.depth() == 1) {
		int pos = sl.pos();
		if (pos && boundary)
			--pos;
//		lyxerr << "coordOffset: boundary:" << boundary << " depth:" << dit.depth() << " pos:" << pos << " sl.pos:" << sl.pos() << std::endl;
		rend = par.pos2row(pos);
	} else
		rend = par.pos2row(sl.pos());
#else
	size_t rend = par.pos2row(sl.pos());
#endif
	for (size_t rit = 0; rit != rend; ++rit)
		y += par.rows()[rit].height();
	y += par.rows()[rend].ascent();
	x += dit.bottom().text()->cursorX(dit.bottom(), boundary && dit.depth() == 1);
	// The following correction should not be there at all.
	// The cursor looks much better with the -1, though.
	--x;
	return Point(x, y);
}


Point getPos(DocIterator const & dit, bool boundary)
{
	CursorSlice const & bot = dit.bottom();
	CoordCache::InnerParPosCache const & cache =
		theCoords.getParPos().find(bot.text())->second;
	CoordCache::InnerParPosCache::const_iterator it = cache.find(bot.pit());
	if (it == cache.end()) {
		//lyxerr << "cursor out of view" << std::endl;
		return Point(-1, -1);
	}
	Point p = coordOffset(dit, boundary); // offset from outer paragraph
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

namespace {

bool findNextInset(DocIterator & dit,
		   vector<InsetBase_code> const & codes,
		   string const & contents)
{
	DocIterator tmpdit = dit;

	while (tmpdit) {
		InsetBase const * inset = tmpdit.nextInset();
		if (inset
		    && find(codes.begin(), codes.end(), inset->lyxCode()) != codes.end()
		    && (contents.empty() ||
			static_cast<InsetCommand const *>(inset)->getContents() == contents)) {
			dit = tmpdit;
			return true;
		}
		tmpdit.forwardInset();
	}

	return false;
}

} // namespace anon


bool findInset(DocIterator & dit, vector<InsetBase_code> const & codes,
	       bool same_content)
{
	string contents;
	DocIterator tmpdit = dit;
	tmpdit.forwardInset();

	if (same_content) {
		InsetBase const * inset = tmpdit.nextInset();
		if (inset
		    && find(codes.begin(), codes.end(), inset->lyxCode()) != codes.end()) {
			contents = static_cast<InsetCommand const *>(inset)->getContents();
		}
	}

	if (!findNextInset(tmpdit, codes, contents)) {
		if (dit.depth() != 1 || dit.pit() != 0 || dit.pos() != 0) {
			tmpdit  = doc_iterator_begin(tmpdit.bottom().inset());
			if (!findNextInset(tmpdit, codes, contents)) {
				return false;
			}
		} else
			return false;
	}
	
	dit = tmpdit;
	return true;
}


void findInset(DocIterator & dit, InsetBase_code code, bool same_content)
{
	findInset(dit, vector<InsetBase_code>(1, code), same_content);
}


void gotoInset(BufferView * bv, vector<InsetBase_code> const & codes,
	       bool same_content)
{
	LCursor tmpcur = bv->cursor();
	if (!findInset(tmpcur, codes, same_content)) {
		bv->cursor().message(_("No more insets"));
		return;
	}

	tmpcur.clearSelection();
	bv->setCursor(tmpcur);
}


void gotoInset(BufferView * bv, InsetBase_code code, bool same_content)
{
	gotoInset(bv, vector<InsetBase_code>(1, code), same_content);
}


} // namespace bv_funcs
