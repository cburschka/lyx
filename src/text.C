/**
 * \file src/text.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author John Levon
 * \author André Pönitz
 * \author Dekel Tsur
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "lyxtext.h"

#include "author.h"
#include "buffer.h"
#include "buffer_funcs.h"
#include "bufferparams.h"
#include "BufferView.h"
#include "cursor.h"
#include "pariterator.h"
#include "coordcache.h"
#include "CutAndPaste.h"
#include "debug.h"
#include "dispatchresult.h"
#include "encoding.h"
#include "errorlist.h"
#include "funcrequest.h"
#include "factory.h"
#include "FontIterator.h"
#include "gettext.h"
#include "language.h"
#include "LColor.h"
#include "lyxlength.h"
#include "lyxlex.h"
#include "lyxrc.h"
#include "lyxrow.h"
#include "metricsinfo.h"
#include "paragraph.h"
#include "paragraph_funcs.h"
#include "ParagraphParameters.h"
#include "rowpainter.h"
#include "undo.h"
#include "vspace.h"
#include "WordLangTuple.h"

#include "frontends/FontMetrics.h"
#include "frontends/Painter.h"

#include "insets/insettext.h"
#include "insets/insetbibitem.h"
#include "insets/insethfill.h"
#include "insets/insetlatexaccent.h"
#include "insets/insetline.h"
#include "insets/insetnewline.h"
#include "insets/insetpagebreak.h"
#include "insets/insetoptarg.h"
#include "insets/insetspace.h"
#include "insets/insetspecialchar.h"
#include "insets/insettabular.h"

#include "support/lstrings.h"
#include "support/textutils.h"
#include "support/convert.h"

#include <boost/current_function.hpp>

#include <sstream>


namespace lyx {

using support::bformat;
using support::contains;
using support::lowercase;
using support::split;
using support::uppercase;

using cap::cutSelection;
using cap::pasteParagraphList;

using frontend::FontMetrics;

using std::auto_ptr;
using std::advance;
using std::distance;
using std::max;
using std::min;
using std::endl;
using std::string;


namespace {

int numberOfSeparators(Paragraph const & par, Row const & row)
{
	pos_type const first = max(row.pos(), par.beginOfBody());
	pos_type const last = row.endpos() - 1;
	int n = 0;
	for (pos_type p = first; p < last; ++p) {
		if (par.isSeparator(p))
			++n;
	}
	return n;
}


int numberOfLabelHfills(Paragraph const & par, Row const & row)
{
	pos_type last = row.endpos() - 1;
	pos_type first = row.pos();

	// hfill *DO* count at the beginning of paragraphs!
	if (first) {
		while (first < last && par.isHfill(first))
			++first;
	}

	last = min(last, par.beginOfBody());
	int n = 0;
	for (pos_type p = first; p < last; ++p) {
		if (par.isHfill(p))
			++n;
	}
	return n;
}


int numberOfHfills(Paragraph const & par, Row const & row)
{
	pos_type const last = row.endpos();
	pos_type first = row.pos();

	// hfill *DO* count at the beginning of paragraphs!
	if (first) {
		while (first < last && par.isHfill(first))
			++first;
	}

	first = max(first, par.beginOfBody());

	int n = 0;
	for (pos_type p = first; p < last; ++p) {
		if (par.isHfill(p))
			++n;
	}
	return n;
}


void readParToken(Buffer const & buf, Paragraph & par, LyXLex & lex,
	string const & token, LyXFont & font, Change & change, ErrorList & errorList)
{
	BufferParams const & bp = buf.params();

	if (token[0] != '\\') {
#if 0
		string::const_iterator cit = token.begin();
		for (; cit != token.end(); ++cit)
			par.insertChar(par.size(), (*cit), font, change);
#else
		docstring dstr = lex.getDocString();
		docstring::const_iterator cit = dstr.begin();
		docstring::const_iterator cend = dstr.end();
		for (; cit != cend; ++cit)
			par.insertChar(par.size(), *cit, font, change);
#endif
	} else if (token == "\\begin_layout") {
		lex.eatLine();
		string layoutname = lex.getString();

		font = LyXFont(LyXFont::ALL_INHERIT, bp.language);
		change = Change(Change::UNCHANGED);

		LyXTextClass const & tclass = bp.getLyXTextClass();

		if (layoutname.empty()) {
			layoutname = tclass.defaultLayoutName();
		}

		bool hasLayout = tclass.hasLayout(layoutname);

		if (!hasLayout) {
			errorList.push_back(ErrorItem(_("Unknown layout"),
			bformat(_("Layout '%1$s' does not exist in textclass '%2$s'\nTrying to use the default instead.\n"),
			from_utf8(layoutname), from_utf8(tclass.name())), par.id(), 0, par.size()));
			layoutname = tclass.defaultLayoutName();
		}

		par.layout(bp.getLyXTextClass()[layoutname]);

		// Test whether the layout is obsolete.
		LyXLayout_ptr const & layout = par.layout();
		if (!layout->obsoleted_by().empty())
			par.layout(bp.getLyXTextClass()[layout->obsoleted_by()]);

		par.params().read(lex);

	} else if (token == "\\end_layout") {
		lyxerr << BOOST_CURRENT_FUNCTION
		       << ": Solitary \\end_layout in line "
		       << lex.getLineNo() << "\n"
		       << "Missing \\begin_layout?.\n";
	} else if (token == "\\end_inset") {
		lyxerr << BOOST_CURRENT_FUNCTION
		       << ": Solitary \\end_inset in line "
		       << lex.getLineNo() << "\n"
		       << "Missing \\begin_inset?.\n";
	} else if (token == "\\begin_inset") {
		InsetBase * inset = readInset(lex, buf);
		if (inset)
			par.insertInset(par.size(), inset, font, change);
		else {
			lex.eatLine();
			docstring line = lex.getDocString();
			errorList.push_back(ErrorItem(_("Unknown Inset"), line,
					    par.id(), 0, par.size()));
		}
	} else if (token == "\\family") {
		lex.next();
		font.setLyXFamily(lex.getString());
	} else if (token == "\\series") {
		lex.next();
		font.setLyXSeries(lex.getString());
	} else if (token == "\\shape") {
		lex.next();
		font.setLyXShape(lex.getString());
	} else if (token == "\\size") {
		lex.next();
		font.setLyXSize(lex.getString());
	} else if (token == "\\lang") {
		lex.next();
		string const tok = lex.getString();
		Language const * lang = languages.getLanguage(tok);
		if (lang) {
			font.setLanguage(lang);
		} else {
			font.setLanguage(bp.language);
			lex.printError("Unknown language `$$Token'");
		}
	} else if (token == "\\numeric") {
		lex.next();
		font.setNumber(font.setLyXMisc(lex.getString()));
	} else if (token == "\\emph") {
		lex.next();
		font.setEmph(font.setLyXMisc(lex.getString()));
	} else if (token == "\\bar") {
		lex.next();
		string const tok = lex.getString();

		if (tok == "under")
			font.setUnderbar(LyXFont::ON);
		else if (tok == "no")
			font.setUnderbar(LyXFont::OFF);
		else if (tok == "default")
			font.setUnderbar(LyXFont::INHERIT);
		else
			lex.printError("Unknown bar font flag "
				       "`$$Token'");
	} else if (token == "\\noun") {
		lex.next();
		font.setNoun(font.setLyXMisc(lex.getString()));
	} else if (token == "\\color") {
		lex.next();
		font.setLyXColor(lex.getString());
	} else if (token == "\\InsetSpace" || token == "\\SpecialChar") {

		// Insets don't make sense in a free-spacing context! ---Kayvan
		if (par.isFreeSpacing()) {
			if (token == "\\InsetSpace")
				par.insertChar(par.size(), ' ', font, change);
			else if (lex.isOK()) {
				lex.next();
				string const next_token = lex.getString();
				if (next_token == "\\-")
					par.insertChar(par.size(), '-', font, change);
				else {
					lex.printError("Token `$$Token' "
						       "is in free space "
						       "paragraph layout!");
				}
			}
		} else {
			auto_ptr<InsetBase> inset;
			if (token == "\\SpecialChar" )
				inset.reset(new InsetSpecialChar);
			else
				inset.reset(new InsetSpace);
			inset->read(buf, lex);
			par.insertInset(par.size(), inset.release(),
					font, change);
		}
	} else if (token == "\\i") {
		auto_ptr<InsetBase> inset(new InsetLatexAccent);
		inset->read(buf, lex);
		par.insertInset(par.size(), inset.release(), font, change);
	} else if (token == "\\backslash") {
		par.insertChar(par.size(), '\\', font, change);
	} else if (token == "\\newline") {
		auto_ptr<InsetBase> inset(new InsetNewline);
		inset->read(buf, lex);
		par.insertInset(par.size(), inset.release(), font, change);
	} else if (token == "\\LyXTable") {
		auto_ptr<InsetBase> inset(new InsetTabular(buf));
		inset->read(buf, lex);
		par.insertInset(par.size(), inset.release(), font, change);
	} else if (token == "\\hfill") {
		par.insertInset(par.size(), new InsetHFill, font, change);
	} else if (token == "\\lyxline") {
		par.insertInset(par.size(), new InsetLine, font, change);
	} else if (token == "\\newpage") {
		par.insertInset(par.size(), new InsetPagebreak, font, change);
	} else if (token == "\\clearpage") {
		par.insertInset(par.size(), new InsetClearPage, font, change);
	} else if (token == "\\cleardoublepage") {
		par.insertInset(par.size(), new InsetClearDoublePage, font, change);
	} else if (token == "\\change_unchanged") {
		change = Change(Change::UNCHANGED);
	} else if (token == "\\change_inserted") {
		lex.eatLine();
		std::istringstream is(lex.getString());
		unsigned int aid;
		time_type ct;
		is >> aid >> ct;
		if (aid >= bp.author_map.size()) {
			errorList.push_back(ErrorItem(_("Change tracking error"),
					    bformat(_("Unknown author index for insertion: %1$d\n"), aid),
					    par.id(), 0, par.size()));
			change = Change(Change::UNCHANGED);
		} else
			change = Change(Change::INSERTED, bp.author_map[aid], ct);
	} else if (token == "\\change_deleted") {
		lex.eatLine();
		std::istringstream is(lex.getString());
		unsigned int aid;
		time_type ct;
		is >> aid >> ct;
		if (aid >= bp.author_map.size()) {
			errorList.push_back(ErrorItem(_("Change tracking error"),
					    bformat(_("Unknown author index for deletion: %1$d\n"), aid),
					    par.id(), 0, par.size()));
			change = Change(Change::UNCHANGED);
		} else
			change = Change(Change::DELETED, bp.author_map[aid], ct);
	} else {
		lex.eatLine();
		errorList.push_back(ErrorItem(_("Unknown token"),
			bformat(_("Unknown token: %1$s %2$s\n"), from_utf8(token),
			lex.getDocString()),
			par.id(), 0, par.size()));
	}
}


void readParagraph(Buffer const & buf, Paragraph & par, LyXLex & lex,
	ErrorList & errorList)
{
	lex.nextToken();
	string token = lex.getString();
	LyXFont font;
	Change change(Change::UNCHANGED);

	while (lex.isOK()) {
		readParToken(buf, par, lex, token, font, change, errorList);

		lex.nextToken();
		token = lex.getString();

		if (token.empty())
			continue;

		if (token == "\\end_layout") {
			//Ok, paragraph finished
			break;
		}

		lyxerr[Debug::PARSER] << "Handling paragraph token: `"
				      << token << '\'' << endl;
		if (token == "\\begin_layout" || token == "\\end_document"
		    || token == "\\end_inset" || token == "\\begin_deeper"
		    || token == "\\end_deeper") {
			lex.pushToken(token);
			lyxerr << "Paragraph ended in line "
			       << lex.getLineNo() << "\n"
			       << "Missing \\end_layout.\n";
			break;
		}
	}
	// Final change goes to paragraph break:
	par.setChange(par.size(), change);

	// Initialize begin_of_body_ on load; redoParagraph maintains
	par.setBeginOfBody();
}


} // namespace anon



double LyXText::spacing(Buffer const & buffer,
		Paragraph const & par) const
{
	if (par.params().spacing().isDefault())
		return buffer.params().spacing().getValue();
	return par.params().spacing().getValue();
}


int LyXText::width() const
{
	return dim_.wid;
}


int LyXText::height() const
{
	return dim_.height();
}


int LyXText::singleWidth(Buffer const & buffer, Paragraph const & par,
		pos_type pos) const
{
	return singleWidth(par, pos, par.getChar(pos),
		getFont(buffer, par, pos));
}


int LyXText::singleWidth(Paragraph const & par,
			 pos_type pos, char_type c, LyXFont const & font) const
{
	// The most common case is handled first (Asger)
	if (isPrintable(c)) {
		Language const * language = font.language();
		if (language->rightToLeft()) {
			if ((lyxrc.font_norm_type == LyXRC::ISO_8859_6_8 ||
			     lyxrc.font_norm_type == LyXRC::ISO_10646_1)
			    && language->lang() == "arabic") {
				if (Encodings::isComposeChar_arabic(c))
					return 0;
				c = par.transformChar(c, pos);
			} else if (language->lang() == "hebrew" &&
				   Encodings::isComposeChar_hebrew(c))
				return 0;
		}
		return theFontMetrics(font).width(c);
	}

	if (c == Paragraph::META_INSET)
		return par.getInset(pos)->width();

	return theFontMetrics(font).width(c);
}


int LyXText::leftMargin(Buffer const & buffer, pit_type pit) const
{
	BOOST_ASSERT(pit >= 0);
	BOOST_ASSERT(pit < int(pars_.size()));
	return leftMargin(buffer, pit, pars_[pit].size());
}


int LyXText::leftMargin(Buffer const & buffer,
		pit_type const pit, pos_type const pos) const
{
	BOOST_ASSERT(pit >= 0);
	BOOST_ASSERT(pit < int(pars_.size()));
	Paragraph const & par = pars_[pit];
	BOOST_ASSERT(pos >= 0);
	BOOST_ASSERT(pos <= par.size());
	//lyxerr << "LyXText::leftMargin: pit: " << pit << " pos: " << pos << endl;
	LyXTextClass const & tclass = buffer.params().getLyXTextClass();
	LyXLayout_ptr const & layout = par.layout();

	string parindent = layout->parindent;

	int l_margin = 0;

	if (isMainText(buffer))
		l_margin += changebarMargin();

	// FIXME UNICODE
	docstring leftm = from_utf8(tclass.leftmargin());
	l_margin += theFontMetrics(buffer.params().getFont()).signedWidth(leftm);

	if (par.getDepth() != 0) {
		// find the next level paragraph
		pit_type newpar = outerHook(pit, pars_);
		if (newpar != pit_type(pars_.size())) {
			if (pars_[newpar].layout()->isEnvironment()) {
				l_margin = leftMargin(buffer, newpar);
			}
			if (par.layout() == tclass.defaultLayout()) {
				if (pars_[newpar].params().noindent())
					parindent.erase();
				else
					parindent = pars_[newpar].layout()->parindent;
			}
		}
	}

	// This happens after sections in standard classes. The 1.3.x
	// code compared depths too, but it does not seem necessary
	// (JMarc)
	if (par.layout() == tclass.defaultLayout()
	    && pit > 0 && pars_[pit - 1].layout()->nextnoindent)
		parindent.erase();

	LyXFont const labelfont = getLabelFont(buffer, par);
	FontMetrics const & labelfont_metrics = theFontMetrics(labelfont);

	switch (layout->margintype) {
	case MARGIN_DYNAMIC:
		if (!layout->leftmargin.empty()) {
			// FIXME UNICODE
			docstring leftm = from_utf8(layout->leftmargin);
			l_margin += theFontMetrics(buffer.params().getFont()).signedWidth(leftm);
		}
		if (!par.getLabelstring().empty()) {
			// FIXME UNICODE
			docstring labin = from_utf8(layout->labelindent);
			l_margin += labelfont_metrics.signedWidth(labin);
			docstring labstr = par.getLabelstring();
			l_margin += labelfont_metrics.width(labstr);
			docstring labsep = from_utf8(layout->labelsep);
			l_margin += labelfont_metrics.width(labsep);
		}
		break;

	case MARGIN_MANUAL: {
		// FIXME UNICODE
		docstring labin = from_utf8(layout->labelindent);
		l_margin += labelfont_metrics.signedWidth(labin);
		// The width of an empty par, even with manual label, should be 0
		if (!par.empty() && pos >= par.beginOfBody()) {
			if (!par.getLabelWidthString().empty()) {
				docstring labstr = par.getLabelWidthString();
				l_margin += labelfont_metrics.width(labstr);
				docstring labsep = from_utf8(layout->labelsep);
				l_margin += labelfont_metrics.width(labsep);
			}
		}
		break;
	}

	case MARGIN_STATIC: {
		// FIXME UNICODE
		docstring leftm = from_utf8(layout->leftmargin);
		l_margin += theFontMetrics(buffer.params().getFont()).signedWidth(leftm)
			* 4	/ (par.getDepth() + 4);
		break;
	}

	case MARGIN_FIRST_DYNAMIC:
		if (layout->labeltype == LABEL_MANUAL) {
			if (pos >= par.beginOfBody()) {
				// FIXME UNICODE
				l_margin += labelfont_metrics.signedWidth(
					from_utf8(layout->leftmargin));
			} else {
				// FIXME UNICODE
				l_margin += labelfont_metrics.signedWidth(
					from_utf8(layout->labelindent));
			}
		} else if (pos != 0
			   // Special case to fix problems with
			   // theorems (JMarc)
			   || (layout->labeltype == LABEL_STATIC
			       && layout->latextype == LATEX_ENVIRONMENT
			       && !isFirstInSequence(pit, pars_))) {
			// FIXME UNICODE
			l_margin += labelfont_metrics.signedWidth(from_utf8(layout->leftmargin));
		} else if (layout->labeltype != LABEL_TOP_ENVIRONMENT
			   && layout->labeltype != LABEL_BIBLIO
			   && layout->labeltype !=
			   LABEL_CENTERED_TOP_ENVIRONMENT) {
			l_margin += labelfont_metrics.signedWidth(from_utf8(layout->labelindent));
			l_margin += labelfont_metrics.width(from_utf8(layout->labelsep));
			l_margin += labelfont_metrics.width(par.getLabelstring());
		}
		break;

	case MARGIN_RIGHT_ADDRESS_BOX: {
#if 0
		// ok, a terrible hack. The left margin depends on the widest
		// row in this paragraph.
		RowList::iterator rit = par.rows().begin();
		RowList::iterator end = par.rows().end();
#ifdef WITH_WARNINGS
#warning This is wrong.
#endif
		int minfill = maxwidth_;
		for ( ; rit != end; ++rit)
			if (rit->fill() < minfill)
				minfill = rit->fill();
		l_margin += theFontMetrics(params.getFont()).signedWidth(layout->leftmargin);
		l_margin += minfill;
#endif
		// also wrong, but much shorter.
		l_margin += maxwidth_ / 2;
		break;
	}
	}

	if (!par.params().leftIndent().zero())
		l_margin += par.params().leftIndent().inPixels(maxwidth_);

	LyXAlignment align;

	if (par.params().align() == LYX_ALIGN_LAYOUT)
		align = layout->align;
	else
		align = par.params().align();

	// set the correct parindent
	if (pos == 0
	    && (layout->labeltype == LABEL_NO_LABEL
	       || layout->labeltype == LABEL_TOP_ENVIRONMENT
	       || layout->labeltype == LABEL_CENTERED_TOP_ENVIRONMENT
	       || (layout->labeltype == LABEL_STATIC
		   && layout->latextype == LATEX_ENVIRONMENT
		   && !isFirstInSequence(pit, pars_)))
	    && align == LYX_ALIGN_BLOCK
	    && !par.params().noindent()
	    // in some insets, paragraphs are never indented
	    && !(par.inInset() && par.inInset()->neverIndent(buffer))
	    // display style insets are always centered, omit indentation
	    && !(!par.empty()
		    && par.isInset(pos)
		    && par.getInset(pos)->display())
	    && (par.layout() != tclass.defaultLayout()
		|| buffer.params().paragraph_separation ==
		   BufferParams::PARSEP_INDENT))
	{
		docstring din = from_utf8(parindent);
		l_margin += theFontMetrics(buffer.params().getFont()).signedWidth(din);
	}

	return l_margin;
}


int LyXText::rightMargin(Buffer const & buffer, Paragraph const & par) const
{
	// FIXME: the correct way is to only call rightMargin() only
	// within the main LyXText. The following test is thus bogus.
	LyXText const & text = buffer.text();
	// We do not want rightmargins on inner texts.
	if (&text != this)
		return 0;

	BufferParams const & params = buffer.params();
	LyXTextClass const & tclass = params.getLyXTextClass();
	docstring trmarg = from_utf8(tclass.rightmargin());
	docstring lrmarg = from_utf8(par.layout()->rightmargin);
	FontMetrics const & fm = theFontMetrics(params.getFont());
	int const r_margin =
		lyx::rightMargin()
		+ fm.signedWidth(trmarg)
		+ fm.signedWidth(lrmarg)
		* 4 / (par.getDepth() + 4);

	return r_margin;
}


int LyXText::labelEnd(Buffer const & buffer, pit_type const pit) const
{
	// labelEnd is only needed if the layout fills a flushleft label.
	if (pars_[pit].layout()->margintype != MARGIN_MANUAL)
		return 0;
	// return the beginning of the body
	return leftMargin(buffer, pit);
}


namespace {

// this needs special handling - only newlines count as a break point
pos_type addressBreakPoint(pos_type i, Paragraph const & par)
{
	pos_type const end = par.size();

	for (; i < end; ++i)
		if (par.isNewline(i))
			return i + 1;

	return end;
}

};


void LyXText::rowBreakPoint(Buffer const & buffer, int right_margin,
		pit_type const pit,	Row & row) const
{
	Paragraph const & par = pars_[pit];
	pos_type const end = par.size();
	pos_type const pos = row.pos();
	if (pos == end) {
		row.endpos(end);
		return;
	}

	// maximum pixel width of a row
	int width = maxwidth_ - right_margin; // - leftMargin(buffer, pit, row);
	if (width < 0) {
		row.endpos(end);
		return;
	}

	LyXLayout_ptr const & layout = par.layout();

	if (layout->margintype == MARGIN_RIGHT_ADDRESS_BOX) {
		row.endpos(addressBreakPoint(pos, par));
		return;
	}

	pos_type const body_pos = par.beginOfBody();


	// Now we iterate through until we reach the right margin
	// or the end of the par, then choose the possible break
	// nearest that.

	int const left = leftMargin(buffer, pit, pos);
	int x = left;

	// pixel width since last breakpoint
	int chunkwidth = 0;

	FontIterator fi = FontIterator(buffer, *this, par, pos);
	pos_type point = end;
	pos_type i = pos;
	FontMetrics const & fm = theFontMetrics(getLabelFont(buffer, par));
	for ( ; i < end; ++i, ++fi) {
		char_type const c = par.getChar(i);
		int thiswidth = singleWidth(par, i, c, *fi);

		// add the auto-hfill from label end to the body
		if (body_pos && i == body_pos) {
			docstring lsep = from_utf8(layout->labelsep);
			int add = fm.width(lsep);
			if (par.isLineSeparator(i - 1))
				add -= singleWidth(buffer, par, i - 1);

			add = std::max(add, labelEnd(buffer, pit) - x);
			thiswidth += add;
		}

		x += thiswidth;
		chunkwidth += thiswidth;

		// break before a character that will fall off
		// the right of the row
		if (x >= width) {
			// if no break before, break here
			if (point == end || chunkwidth >= width - left) {
				if (i > pos)
					point = i;
				else
					point = i + 1;

			}
			// exit on last registered breakpoint:
			break;
		}

		if (par.isNewline(i)) {
			point = i + 1;
			break;
		}
		// Break before...
		if (i + 1 < end) {
			if (par.isInset(i + 1) && par.getInset(i + 1)->display()) {
				point = i + 1;
				break;
			}
			// ...and after.
			if (par.isInset(i) && par.getInset(i)->display()) {
				point = i + 1;
				break;
			}
		}

		if (!par.isInset(i) || par.getInset(i)->isChar()) {
			// some insets are line separators too
			if (par.isLineSeparator(i)) {
				// register breakpoint:
				point = i + 1;
				chunkwidth = 0;
			}
		}
	}

	// maybe found one, but the par is short enough.
	if (i == end && x < width)
		point = end;

	// manual labels cannot be broken in LaTeX. But we
	// want to make our on-screen rendering of footnotes
	// etc. still break
	if (body_pos && point < body_pos)
		point = body_pos;

	row.endpos(point);
}


void LyXText::setRowWidth(Buffer const & buffer, pit_type const pit, Row & row) const
{
	// get the pure distance
	pos_type const end = row.endpos();

	Paragraph const & par = pars_[pit];
	docstring const labelsep = from_utf8(par.layout()->labelsep);
	int w = leftMargin(buffer, pit, row.pos());

	pos_type const body_pos = par.beginOfBody();
	pos_type i = row.pos();

	FontMetrics const & fm = theFontMetrics(getLabelFont(buffer, par));

	if (i < end) {
		FontIterator fi = FontIterator(buffer, *this, par, i);
		for ( ; i < end; ++i, ++fi) {
			if (body_pos > 0 && i == body_pos) {
				w += fm.width(labelsep);
				if (par.isLineSeparator(i - 1))
					w -= singleWidth(buffer, par, i - 1);
				w = max(w, labelEnd(buffer, pit));
			}
			char_type const c = par.getChar(i);
			w += singleWidth(par, i, c, *fi);
		}
	}

	if (body_pos > 0 && body_pos >= end) {
		w += fm.width(labelsep);
		if (end > 0 && par.isLineSeparator(end - 1))
			w -= singleWidth(buffer, par, end - 1);
		w = max(w, labelEnd(buffer, pit));
	}

	row.width(w + rightMargin(buffer, par));
}


// returns the minimum space a manual label needs on the screen in pixel
int LyXText::labelFill(Buffer const & buffer, Paragraph const & par,
		Row const & row) const
{
	pos_type last = par.beginOfBody();

	BOOST_ASSERT(last > 0);

	// -1 because a label ends with a space that is in the label
	--last;

	// a separator at this end does not count
	if (par.isLineSeparator(last))
		--last;

	int w = 0;
	for (pos_type i = row.pos(); i <= last; ++i)
		w += singleWidth(buffer, par, i);

	docstring const & label = par.params().labelWidthString();
	if (label.empty())
		return 0;

	FontMetrics const & fm = theFontMetrics(getLabelFont(buffer, par));

	return max(0, fm.width(label) - w);
}


LColor_color LyXText::backgroundColor() const
{
	return LColor_color(LColor::color(background_color_));
}


void LyXText::setHeightOfRow(BufferView const & bv, pit_type const pit,
		Row & row)
{
	Paragraph const & par = pars_[pit];
	// get the maximum ascent and the maximum descent
	double layoutasc = 0;
	double layoutdesc = 0;
	double const dh = defaultRowHeight();

	// ok, let us initialize the maxasc and maxdesc value.
	// Only the fontsize count. The other properties
	// are taken from the layoutfont. Nicer on the screen :)
	LyXLayout_ptr const & layout = par.layout();

	// as max get the first character of this row then it can
	// increase but not decrease the height. Just some point to
	// start with so we don't have to do the assignment below too
	// often.
	Buffer const & buffer = *bv.buffer();
	LyXFont font = getFont(buffer, par, row.pos());
	LyXFont::FONT_SIZE const tmpsize = font.size();
	font = getLayoutFont(buffer, pit);
	LyXFont::FONT_SIZE const size = font.size();
	font.setSize(tmpsize);

	LyXFont labelfont = getLabelFont(buffer, par);

	FontMetrics const & labelfont_metrics = theFontMetrics(labelfont);
	FontMetrics const & fontmetrics = theFontMetrics(font);

	// these are minimum values
	double const spacing_val = layout->spacing.getValue()
		* spacing(*bv.buffer(), par);
	//lyxerr << "spacing_val = " << spacing_val << endl;
	int maxasc  = int(fontmetrics.maxAscent()  * spacing_val);
	int maxdesc = int(fontmetrics.maxDescent() * spacing_val);

	// insets may be taller
	InsetList::const_iterator ii = par.insetlist.begin();
	InsetList::const_iterator iend = par.insetlist.end();
	for ( ; ii != iend; ++ii) {
		if (ii->pos >= row.pos() && ii->pos < row.endpos()) {
			maxasc  = max(maxasc,  ii->inset->ascent());
			maxdesc = max(maxdesc, ii->inset->descent());
		}
	}

	// Check if any custom fonts are larger (Asger)
	// This is not completely correct, but we can live with the small,
	// cosmetic error for now.
	int labeladdon = 0;
	pos_type const pos_end = row.endpos();

	LyXFont::FONT_SIZE maxsize =
		par.highestFontInRange(row.pos(), pos_end, size);
	if (maxsize > font.size()) {
		font.setSize(maxsize);
		maxasc  = max(maxasc,  fontmetrics.maxAscent());
		maxdesc = max(maxdesc, fontmetrics.maxDescent());
	}

	// This is nicer with box insets:
	++maxasc;
	++maxdesc;

	row.ascent(maxasc);

	// is it a top line?
	if (row.pos() == 0) {
		Buffer const & buffer = *bv.buffer();
		BufferParams const & bufparams = buffer.params();
		// some parksips VERY EASY IMPLEMENTATION
		if (bufparams.paragraph_separation
		    == BufferParams::PARSEP_SKIP
			&& pit != 0
			&& ((layout->isParagraph() && par.getDepth() == 0)
			    || (pars_[pit - 1].layout()->isParagraph()
				&& pars_[pit - 1].getDepth() == 0)))
		{
				maxasc += bufparams.getDefSkip().inPixels(bv);
		}

		if (par.params().startOfAppendix())
			maxasc += int(3 * dh);

		// This is special code for the chapter, since the label of this
		// layout is printed in an extra row
		if (layout->counter == "chapter"
		    && !par.params().labelString().empty()) {
			labeladdon = int(labelfont_metrics.maxHeight()
				     * layout->spacing.getValue()
				     * spacing(buffer, par));
		}

		// special code for the top label
		if ((layout->labeltype == LABEL_TOP_ENVIRONMENT
		     || layout->labeltype == LABEL_BIBLIO
		     || layout->labeltype == LABEL_CENTERED_TOP_ENVIRONMENT)
		    && isFirstInSequence(pit, paragraphs())
		    && !par.getLabelstring().empty())
		{
			labeladdon = int(
				  labelfont_metrics.maxHeight()
					* layout->spacing.getValue()
					* spacing(*bv.buffer(), par)
				+ (layout->topsep + layout->labelbottomsep) * dh);
		}

		// Add the layout spaces, for example before and after
		// a section, or between the items of a itemize or enumerate
		// environment.

		pit_type prev = depthHook(pit, pars_, par.getDepth());
		if (prev != pit
		    && pars_[prev].layout() == layout
		    && pars_[prev].getDepth() == par.getDepth()
		    && pars_[prev].getLabelWidthString() == par.getLabelWidthString())
		{
			layoutasc = layout->itemsep * dh;
		} else if (pit != 0 || row.pos() != 0) {
			if (layout->topsep > 0)
				layoutasc = layout->topsep * dh;
		}

		prev = outerHook(pit, pars_);
		if (prev != pit_type(pars_.size())) {
			maxasc += int(pars_[prev].layout()->parsep * dh);
		} else if (pit != 0) {
			if (pars_[pit - 1].getDepth() != 0 ||
					pars_[pit - 1].layout() == layout) {
				maxasc += int(layout->parsep * dh);
			}
		}
	}

	// is it a bottom line?
	if (row.endpos() >= par.size()) {
		// add the layout spaces, for example before and after
		// a section, or between the items of a itemize or enumerate
		// environment
		pit_type nextpit = pit + 1;
		if (nextpit != pit_type(pars_.size())) {
			pit_type cpit = pit;
			double usual = 0;
			double unusual = 0;

			if (pars_[cpit].getDepth() > pars_[nextpit].getDepth()) {
				usual = pars_[cpit].layout()->bottomsep * dh;
				cpit = depthHook(cpit, paragraphs(), pars_[nextpit].getDepth());
				if (pars_[cpit].layout() != pars_[nextpit].layout()
					|| pars_[nextpit].getLabelWidthString() != pars_[cpit].getLabelWidthString())
				{
					unusual = pars_[cpit].layout()->bottomsep * dh;
				}
				layoutdesc = max(unusual, usual);
			} else if (pars_[cpit].getDepth() == pars_[nextpit].getDepth()) {
				if (pars_[cpit].layout() != pars_[nextpit].layout()
					|| pars_[nextpit].getLabelWidthString() != pars_[cpit].getLabelWidthString())
					layoutdesc = int(pars_[cpit].layout()->bottomsep * dh);
			}
		}
	}

	// incalculate the layout spaces
	maxasc  += int(layoutasc  * 2 / (2 + pars_[pit].getDepth()));
	maxdesc += int(layoutdesc * 2 / (2 + pars_[pit].getDepth()));

	// FIXME: the correct way is to do the following is to move the 
	// following code in another method specially tailored for the 
	// main LyXText. The following test is thus bogus.
	LyXText const & text = buffer.text();
	// Top and bottom margin of the document (only at top-level)
	if (&text == this) {
		if (pit == 0 && row.pos() == 0)
			maxasc += 20;
		if (pit + 1 == pit_type(pars_.size()) &&
		    row.endpos() == par.size())
			maxdesc += 20;
	}

	row.ascent(maxasc + labeladdon);
	row.descent(maxdesc);
}


void LyXText::breakParagraph(LCursor & cur, bool keep_layout)
{
	BOOST_ASSERT(this == cur.text());

	Paragraph & cpar = cur.paragraph();
	pit_type cpit = cur.pit();

	LyXTextClass const & tclass = cur.buffer().params().getLyXTextClass();
	LyXLayout_ptr const & layout = cpar.layout();

	// this is only allowed, if the current paragraph is not empty
	// or caption and if it has not the keepempty flag active
	if (cur.lastpos() == 0 && !cpar.allowEmpty() &&
	    layout->labeltype != LABEL_SENSITIVE)
		return;

	// a layout change may affect also the following paragraph
	recUndo(cur, cur.pit(), undoSpan(cur.pit()) - 1);

	// Always break behind a space
	// It is better to erase the space (Dekel)
	if (cur.pos() != cur.lastpos() && cpar.isLineSeparator(cur.pos()))
		cpar.eraseChar(cur.pos(), cur.buffer().params().trackChanges);

	// What should the layout for the new paragraph be?
	int preserve_layout = 0;
	if (keep_layout)
		preserve_layout = 2;
	else
		preserve_layout = layout->isEnvironment();

	// We need to remember this before we break the paragraph, because
	// that invalidates the layout variable
	bool sensitive = layout->labeltype == LABEL_SENSITIVE;

	// we need to set this before we insert the paragraph.
	bool const isempty = cpar.allowEmpty() && cpar.empty();

	lyx::breakParagraph(cur.buffer().params(), paragraphs(), cpit,
			 cur.pos(), preserve_layout);

	// After this, neither paragraph contains any rows!

	cpit = cur.pit();
	pit_type next_par = cpit + 1;

	// well this is the caption hack since one caption is really enough
	if (sensitive) {
		if (cur.pos() == 0)
			// set to standard-layout
			pars_[cpit].applyLayout(tclass.defaultLayout());
		else
			// set to standard-layout
			pars_[next_par].applyLayout(tclass.defaultLayout());
	}

	while (!pars_[next_par].empty() && pars_[next_par].isNewline(0))
		pars_[next_par].eraseChar(0, cur.buffer().params().trackChanges);

	ParIterator current_it(cur);
	ParIterator last_it(cur);
	++last_it;
	++last_it;

	updateLabels(cur.buffer(), current_it, last_it);

	// FIXME: Breaking a paragraph has nothing to do with setting a cursor.
	// Because of the mix between the model (the paragraph contents) and the
	// view (the paragraph breaking in rows, we have to do this here before
	// the setCursor() call below.
	bool changed_height = redoParagraph(cur.bv(), cpit);
	changed_height |= redoParagraph(cur.bv(), cpit + 1);
	if (changed_height)
		// A singlePar update is not enough in this case.
		cur.updateFlags(Update::Force);

	// This check is necessary. Otherwise the new empty paragraph will
	// be deleted automatically. And it is more friendly for the user!
	if (cur.pos() != 0 || isempty)
		setCursor(cur, cur.pit() + 1, 0);
	else
		setCursor(cur, cur.pit(), 0);
}


// insert a character, moves all the following breaks in the
// same Paragraph one to the right and make a rebreak
void LyXText::insertChar(LCursor & cur, char_type c)
{
	BOOST_ASSERT(this == cur.text());
	BOOST_ASSERT(c != Paragraph::META_INSET);

	recordUndo(cur, Undo::INSERT);

	Buffer const & buffer = cur.buffer();
	Paragraph & par = cur.paragraph();
	// try to remove this
	pit_type const pit = cur.pit();

	bool const freeSpacing = par.layout()->free_spacing ||
		par.isFreeSpacing();

	if (lyxrc.auto_number) {
		static docstring const number_operators = from_ascii("+-/*");
		static docstring const number_unary_operators = from_ascii("+-");
		static docstring const number_seperators = from_ascii(".,:");

		if (current_font.number() == LyXFont::ON) {
			if (!isDigit(c) && !contains(number_operators, c) &&
			    !(contains(number_seperators, c) &&
			      cur.pos() != 0 &&
			      cur.pos() != cur.lastpos() &&
			      getFont(buffer, par, cur.pos()).number() == LyXFont::ON &&
			      getFont(buffer, par, cur.pos() - 1).number() == LyXFont::ON)
			   )
				number(cur); // Set current_font.number to OFF
		} else if (isDigit(c) &&
			   real_current_font.isVisibleRightToLeft()) {
			number(cur); // Set current_font.number to ON

			if (cur.pos() != 0) {
				char_type const c = par.getChar(cur.pos() - 1);
				if (contains(number_unary_operators, c) &&
				    (cur.pos() == 1
				     || par.isSeparator(cur.pos() - 2)
				     || par.isNewline(cur.pos() - 2))
				  ) {
					setCharFont(buffer, pit, cur.pos() - 1, current_font);
				} else if (contains(number_seperators, c)
				     && cur.pos() >= 2
				     && getFont(buffer, par, cur.pos() - 2).number() == LyXFont::ON) {
					setCharFont(buffer, pit, cur.pos() - 1, current_font);
				}
			}
		}
	}

	// First check, if there will be two blanks together or a blank at
	// the beginning of a paragraph.
	// I decided to handle blanks like normal characters, the main
	// difference are the special checks when calculating the row.fill
	// (blank does not count at the end of a row) and the check here

	// When the free-spacing option is set for the current layout,
	// disable the double-space checking
	if (!freeSpacing && isLineSeparatorChar(c)) {
		if (cur.pos() == 0) {
			static bool sent_space_message = false;
			if (!sent_space_message) {
				cur.message(_("You cannot insert a space at the "
							   "beginning of a paragraph. Please read the Tutorial."));
				sent_space_message = true;
			}
			return;
		}
		BOOST_ASSERT(cur.pos() > 0);
		if ((par.isLineSeparator(cur.pos() - 1) || par.isNewline(cur.pos() - 1))
		    && !par.isDeleted(cur.pos() - 1)) {
			static bool sent_space_message = false;
			if (!sent_space_message) {
				cur.message(_("You cannot type two spaces this way. "
							   "Please read the Tutorial."));
				sent_space_message = true;
			}
			return;
		}
	}

	par.insertChar(cur.pos(), c, current_font, cur.buffer().params().trackChanges);

	// FIXME: Inserting a character has nothing to do with setting a cursor.
	// Because of the mix between the model (the paragraph contents) and the
	// view (the paragraph breaking in rows, we have to do this here.
	if (redoParagraph(cur.bv(), cur.pit()))
		// A singlePar update is not enough in this case.
		cur.updateFlags(Update::Force);
	setCursor(cur, cur.pit(), cur.pos() + 1, false, cur.boundary());
	charInserted();
}


void LyXText::charInserted()
{
	// Here we call finishUndo for every 20 characters inserted.
	// This is from my experience how emacs does it. (Lgb)
	static unsigned int counter;
	if (counter < 20) {
		++counter;
	} else {
		finishUndo();
		counter = 0;
	}
}


RowMetrics LyXText::computeRowMetrics(Buffer const & buffer,
		pit_type const pit, Row const & row) const
{
	RowMetrics result;
	Paragraph const & par = pars_[pit];

	double w = dim_.wid - row.width();

	bool const is_rtl = isRTL(buffer, par);
	if (is_rtl)
		result.x = rightMargin(buffer, par);
	else
		result.x = leftMargin(buffer, pit, row.pos());

	// is there a manual margin with a manual label
	LyXLayout_ptr const & layout = par.layout();

	if (layout->margintype == MARGIN_MANUAL
	    && layout->labeltype == LABEL_MANUAL) {
		/// We might have real hfills in the label part
		int nlh = numberOfLabelHfills(par, row);

		// A manual label par (e.g. List) has an auto-hfill
		// between the label text and the body of the
		// paragraph too.
		// But we don't want to do this auto hfill if the par
		// is empty.
		if (!par.empty())
			++nlh;

		if (nlh && !par.getLabelWidthString().empty())
			result.label_hfill = labelFill(buffer, par, row) / double(nlh);
	}

	// are there any hfills in the row?
	int const nh = numberOfHfills(par, row);

	if (nh) {
		if (w > 0)
			result.hfill = w / nh;
	// we don't have to look at the alignment if it is ALIGN_LEFT and
	// if the row is already larger then the permitted width as then
	// we force the LEFT_ALIGN'edness!
	} else if (int(row.width()) < maxwidth_) {
		// is it block, flushleft or flushright?
		// set x how you need it
		int align;
		if (par.params().align() == LYX_ALIGN_LAYOUT)
			align = layout->align;
		else
			align = par.params().align();

		// Display-style insets should always be on a centred row
		// The test on par.size() is to catch zero-size pars, which
		// would trigger the assert in Paragraph::getInset().
		//inset = par.size() ? par.getInset(row.pos()) : 0;
		if (!par.empty()
		    && par.isInset(row.pos())
		    && par.getInset(row.pos())->display())
		{
			align = LYX_ALIGN_CENTER;
		}

		switch (align) {
		case LYX_ALIGN_BLOCK: {
			int const ns = numberOfSeparators(par, row);
			bool disp_inset = false;
			if (row.endpos() < par.size()) {
				InsetBase const * in = par.getInset(row.endpos());
				if (in)
					disp_inset = in->display();
			}
			// If we have separators, this is not the last row of a
			// par, does not end in newline, and is not row above a
			// display inset... then stretch it
			if (ns
			    && row.endpos() < par.size()
			    && !par.isNewline(row.endpos() - 1)
			    && !disp_inset
				) {
				result.separator = w / ns;
			} else if (is_rtl) {
				result.x += w;
			}
			break;
		}
		case LYX_ALIGN_RIGHT:
			result.x += w;
			break;
		case LYX_ALIGN_CENTER:
			result.x += w / 2;
			break;
		}
	}

	bidi.computeTables(par, buffer, row);
	if (is_rtl) {
		pos_type body_pos = par.beginOfBody();
		pos_type end = row.endpos();

		if (body_pos > 0
		    && (body_pos > end || !par.isLineSeparator(body_pos - 1)))
		{
			docstring const lsep = from_utf8(layout->labelsep);
			result.x += theFontMetrics(getLabelFont(buffer, par)).width(lsep);
			if (body_pos <= end)
				result.x += result.label_hfill;
		}
	}

	return result;
}


// the cursor set functions have a special mechanism. When they
// realize, that you left an empty paragraph, they will delete it.

bool LyXText::cursorRightOneWord(LCursor & cur)
{
	BOOST_ASSERT(this == cur.text());

	LCursor old = cur;

	if (old.pos() == old.lastpos() && old.pit() != old.lastpit()) {
		++old.pit();
		old.pos() = 0;
	} else {
		// Advance through word.
		while (old.pos() != old.lastpos() && old.paragraph().isLetter(old.pos()))
			++old.pos();
		// Skip through trailing nonword stuff.
		while (old.pos() != old.lastpos() && !old.paragraph().isLetter(old.pos()))
			++old.pos();
	}
	return setCursor(cur, old.pit(), old.pos());
}


bool LyXText::cursorLeftOneWord(LCursor & cur)
{
	BOOST_ASSERT(this == cur.text());

	LCursor old = cur;

	if (old.pos() == 0 && old.pit() != 0) {
		--old.pit();
		old.pos() = old.lastpos();
	} else {
		// Skip through initial nonword stuff.
		while (old.pos() != 0 && !old.paragraph().isLetter(old.pos() - 1))
			--old.pos();
		// Advance through word.
		while (old.pos() != 0 && old.paragraph().isLetter(old.pos() - 1))
			--old.pos();
	}
	return setCursor(cur, old.pit(), old.pos());
}


void LyXText::selectWord(LCursor & cur, word_location loc)
{
	BOOST_ASSERT(this == cur.text());
	CursorSlice from = cur.top();
	CursorSlice to = cur.top();
	getWord(from, to, loc);
	if (cur.top() != from)
		setCursor(cur, from.pit(), from.pos());
	if (to == from)
		return;
	cur.resetAnchor();
	setCursor(cur, to.pit(), to.pos());
	cur.setSelection();
}


// Select the word currently under the cursor when no
// selection is currently set
bool LyXText::selectWordWhenUnderCursor(LCursor & cur, word_location loc)
{
	BOOST_ASSERT(this == cur.text());
	if (cur.selection())
		return false;
	selectWord(cur, loc);
	return cur.selection();
}


void LyXText::acceptChange(LCursor & cur)
{
	// FIXME: change tracking (MG)

	BOOST_ASSERT(this == cur.text());

	if (!cur.selection() && cur.lastpos() != 0)
		return;

	// FIXME: we must handle start = end = 0

	recordUndoSelection(cur, Undo::INSERT);

	DocIterator it = cur.selectionBegin();
	DocIterator et = cur.selectionEnd();
	pit_type pit = it.pit();
	for (; pit <= et.pit(); ++pit) {
		pos_type left  = (pit == it.pit() ? it.pos() : 0);
		pos_type right = (pit == et.pit() ? et.pos() : pars_[pit].size());
		pars_[pit].acceptChanges(left, right);

		// merge paragraph if appropriate:
		// if (right >= pars_[pit].size() && pit + 1 < et.pit() &&
		//    pars_[pit].isDeleted(pars_[pit].size())) {
		//	setCursorIntern(cur, pit + 1, 0);
		//	backspacePos0(cur);
		//}
	}
	finishUndo();
	cur.clearSelection();
	setCursorIntern(cur, it.pit(), 0);
}


void LyXText::rejectChange(LCursor & cur)
{
	// FIXME: change tracking (MG)

	BOOST_ASSERT(this == cur.text());

	if (!cur.selection() && cur.lastpos() != 0)
		return;

	// FIXME: we must handle start = end = 0

	recordUndoSelection(cur, Undo::INSERT);

	DocIterator it = cur.selectionBegin();
	DocIterator et = cur.selectionEnd();
	pit_type pit = it.pit();
	for (; pit <= et.pit(); ++pit) {
		pos_type left  = (pit == it.pit() ? it.pos() : 0);
		pos_type right = (pit == et.pit() ? et.pos() : pars_[pit].size());
		pars_[pit].rejectChanges(left, right);

		// merge paragraph if appropriate:	
		// if (right >= pars_[pit].size() && pit + 1 < et.pit() &&
		//    pars_[pit].isInserted(pars_[pit].size())) {
		//	setCursorIntern(cur, pit + 1, 0);
		//	backspacePos0(cur);
		//}
	}
	finishUndo();
	cur.clearSelection();
	setCursorIntern(cur, it.pit(), 0);
}


// Delete from cursor up to the end of the current or next word.
void LyXText::deleteWordForward(LCursor & cur)
{
	BOOST_ASSERT(this == cur.text());
	if (cur.lastpos() == 0)
		cursorRight(cur);
	else {
		cur.resetAnchor();
		cur.selection() = true;
		cursorRightOneWord(cur);
		cur.setSelection();
		cutSelection(cur, true, false);
	}
}


// Delete from cursor to start of current or prior word.
void LyXText::deleteWordBackward(LCursor & cur)
{
	BOOST_ASSERT(this == cur.text());
	if (cur.lastpos() == 0)
		cursorLeft(cur);
	else {
		cur.resetAnchor();
		cur.selection() = true;
		cursorLeftOneWord(cur);
		cur.setSelection();
		cutSelection(cur, true, false);
	}
}


// Kill to end of line.
void LyXText::deleteLineForward(LCursor & cur)
{
	BOOST_ASSERT(this == cur.text());
	if (cur.lastpos() == 0) {
		// Paragraph is empty, so we just go to the right
		cursorRight(cur);
	} else {
		cur.resetAnchor();
		cur.selection() = true; // to avoid deletion
		cursorEnd(cur);
		cur.setSelection();
		// What is this test for ??? (JMarc)
		if (!cur.selection())
			deleteWordForward(cur);
		else
			cutSelection(cur, true, false);
	}
}


void LyXText::changeCase(LCursor & cur, LyXText::TextCase action)
{
	BOOST_ASSERT(this == cur.text());
	CursorSlice from;
	CursorSlice to;

	if (cur.selection()) {
		from = cur.selBegin();
		to = cur.selEnd();
	} else {
		from = cur.top();
		getWord(from, to, PARTIAL_WORD);
		cursorRightOneWord(cur);
	}

	recordUndoSelection(cur);

	pos_type pos = from.pos();
	int par = from.pit();

	while (par != int(pars_.size()) && (pos != to.pos() || par != to.pit())) {
		pit_type pit = par;
		if (pos == pars_[pit].size()) {
			++par;
			pos = 0;
			continue;
		}
		char_type c = pars_[pit].getChar(pos);
		if (c != Paragraph::META_INSET) {
			switch (action) {
			case text_lowercase:
				c = lowercase(c);
				break;
			case text_capitalization:
				c = uppercase(c);
				action = text_lowercase;
				break;
			case text_uppercase:
				c = uppercase(c);
				break;
			}
		}

		// FIXME: change tracking (MG)
		// sorry but we are no longer allowed to set a single character directly
		// we have to rewrite this method in terms of erase&insert operations
		//pars_[pit].setChar(pos, c);
		++pos;
	}
}


bool LyXText::erase(LCursor & cur)
{
	BOOST_ASSERT(this == cur.text());
	bool needsUpdate = false;
	Paragraph & par = cur.paragraph();

	if (cur.pos() != cur.lastpos()) {
		// this is the code for a normal delete, not pasting
		// any paragraphs
		recordUndo(cur, Undo::DELETE);
		par.eraseChar(cur.pos(), cur.buffer().params().trackChanges);
		if (par.isDeleted(cur.pos()))
			cur.forwardPosNoDescend();
		needsUpdate = true;
	} else {
		if (cur.pit() == cur.lastpit())
			return dissolveInset(cur);

		if (!par.isMergedOnEndOfParDeletion(cur.buffer().params().trackChanges)) {
			par.setChange(cur.pos(), Change(Change::DELETED));
			cur.forwardPos();
			needsUpdate = true;
		} else {
			setCursorIntern(cur, cur.pit() + 1, 0);
			needsUpdate = backspacePos0(cur);
		}
	}

	// FIXME: Inserting characters has nothing to do with setting a cursor.
	// Because of the mix between the model (the paragraph contents)
	// and the view (the paragraph breaking in rows, we have to do this
	// here before the setCursorIntern() call.
	if (needsUpdate) {
		if (redoParagraph(cur.bv(), cur.pit()))
			// A singlePar update is not enough in this case.
			cur.updateFlags(Update::Force);
		// Make sure the cursor is correct. Is this really needed?
		// No, not really... at least not here!
		cur.text()->setCursorIntern(cur, cur.pit(), cur.pos());
	}
	
	return needsUpdate;
}


bool LyXText::backspacePos0(LCursor & cur)
{
	BOOST_ASSERT(this == cur.text());
	if (cur.pit() == 0)
		return false;

	bool needsUpdate = false;

	BufferParams const & bufparams = cur.buffer().params();
	LyXTextClass const & tclass = bufparams.getLyXTextClass();
	ParagraphList & plist = cur.text()->paragraphs();
	Paragraph const & par = cur.paragraph();
	LCursor prevcur = cur;
	--prevcur.pit();
	prevcur.pos() = prevcur.lastpos();
	Paragraph const & prevpar = prevcur.paragraph();

	// is it an empty paragraph?
	if (cur.lastpos() == 0 
	    || (cur.lastpos() == 1 && par.isSeparator(0))) {
		recordUndo(cur, Undo::ATOMIC, prevcur.pit(), cur.pit());
		plist.erase(boost::next(plist.begin(), cur.pit()));
		needsUpdate = true;
	}
	// is previous par empty?
	else if (prevcur.lastpos() == 0 
		 || (prevcur.lastpos() == 1 && prevpar.isSeparator(0))) {
		recordUndo(cur, Undo::ATOMIC, prevcur.pit(), cur.pit());
		plist.erase(boost::next(plist.begin(), prevcur.pit()));
		needsUpdate = true;
	}
	// Pasting is not allowed, if the paragraphs have different
	// layouts. I think it is a real bug of all other
	// word processors to allow it. It confuses the user.
	// Correction: Pasting is always allowed with standard-layout
	else if (par.layout() == prevpar.layout()
		 || par.layout() == tclass.defaultLayout()) {
		recordUndo(cur, Undo::ATOMIC, prevcur.pit());
		mergeParagraph(bufparams, plist, prevcur.pit());
		needsUpdate = true;
	}

	if (needsUpdate) {
		updateLabels(cur.buffer());
		setCursorIntern(cur, prevcur.pit(), prevcur.pos());
	}

	return needsUpdate;
}


bool LyXText::backspace(LCursor & cur)
{
	BOOST_ASSERT(this == cur.text());
	bool needsUpdate = false;
	if (cur.pos() == 0) {
		if (cur.pit() == 0)
			return dissolveInset(cur);

		Paragraph & prev_par = pars_[cur.pit() - 1];

		if (!prev_par.isMergedOnEndOfParDeletion(cur.buffer().params().trackChanges)) {
			prev_par.setChange(prev_par.size(), Change(Change::DELETED));
			setCursorIntern(cur, cur.pit() - 1, prev_par.size());
			return true;
		}
		// The cursor is at the beginning of a paragraph, so
		// the backspace will collapse two paragraphs into one.
		needsUpdate = backspacePos0(cur);

	} else {
		// this is the code for a normal backspace, not pasting
		// any paragraphs
		recordUndo(cur, Undo::DELETE);
		// We used to do cursorLeftIntern() here, but it is
		// not a good idea since it triggers the auto-delete
		// mechanism. So we do a cursorLeftIntern()-lite,
		// without the dreaded mechanism. (JMarc)
		setCursorIntern(cur, cur.pit(), cur.pos() - 1,
				false, cur.boundary());
		cur.paragraph().eraseChar(cur.pos(), cur.buffer().params().trackChanges);
	}

	if (cur.pos() == cur.lastpos())
		setCurrentFont(cur);

	// FIXME: Backspacing has nothing to do with setting a cursor.
	// Because of the mix between the model (the paragraph contents)
	// and the view (the paragraph breaking in rows, we have to do this
	// here before the setCursor() call.
	if (redoParagraph(cur.bv(), cur.pit()))
		// A singlePar update is not enough in this case.
		cur.updateFlags(Update::Force);
	setCursor(cur, cur.pit(), cur.pos(), false, cur.boundary());

	return needsUpdate;
}


bool LyXText::dissolveInset(LCursor & cur) {
	BOOST_ASSERT(this == cur.text());

	if (isMainText(*cur.bv().buffer()) || cur.inset().nargs() != 1)
		return false;

	bool const in_ert = cur.inset().lyxCode() == InsetBase::ERT_CODE;
	recordUndoInset(cur);
	cur.selHandle(false);
	// save position
	pos_type spos = cur.pos();
	pit_type spit = cur.pit();
	ParagraphList plist;
	if (cur.lastpit() != 0 || cur.lastpos() != 0)
		plist = paragraphs();
	cur.popLeft();
	// store cursor offset
	if (spit == 0)
		spos += cur.pos();
	spit += cur.pit();
	Buffer & b = cur.buffer();
	cur.paragraph().eraseChar(cur.pos(), b.params().trackChanges);
	if (!plist.empty()) {
		if (in_ert) {
			// ERT paragraphs have the Language latex_language.
			// This is invalid outside of ERT, so we need to
			// change it to the buffer language.
			ParagraphList::iterator it = plist.begin();
			ParagraphList::iterator it_end = plist.end();
			for (; it != it_end; it++) {
				it->changeLanguage(b.params(),
						latex_language,
						b.getLanguage());
			}
		}

		pasteParagraphList(cur, plist, b.params().textclass,
				   b.errorList("Paste"));
		// restore position
		cur.pit() = std::min(cur.lastpit(), spit);
		cur.pos() = std::min(cur.lastpos(), spos);
	}
	cur.clearSelection();
	cur.resetAnchor();
	return true;
}


Row const & LyXText::firstRow() const
{
	return *paragraphs().front().rows().begin();
}


bool LyXText::redoParagraph(BufferView & bv, pit_type const pit)
{
	// remove rows of paragraph, keep track of height changes
	Paragraph & par = pars_[pit];
	Buffer const & buffer = *bv.buffer();

	bool changed = false;

	// Add bibitem insets if necessary
	if (par.layout()->labeltype == LABEL_BIBLIO) {
		bool hasbibitem(false);
		if (!par.insetlist.empty()
			// Insist on it being in pos 0
			&& par.getChar(0) == Paragraph::META_INSET) {
			InsetBase * inset = par.insetlist.begin()->inset;
			if (inset->lyxCode() == InsetBase::BIBITEM_CODE)
				hasbibitem = true;
		}
		if (!hasbibitem) {
			InsetBibitem * inset(new
				InsetBibitem(InsetCommandParams("bibitem")));
			par.insertInset(0, static_cast<InsetBase *>(inset),
			                Change(buffer.params().trackChanges ?
			                       Change::INSERTED : Change::UNCHANGED));
			bv.cursor().posRight();
		}
	}

	// Optimisation: this is used in the next two loops
	// so better to calculate that once here.
	int right_margin = rightMargin(buffer, par);

	// redo insets
	// FIXME: We should always use getFont(), see documentation of
	// noFontChange() in insetbase.h.
	LyXFont const bufferfont = buffer.params().getFont();
	InsetList::const_iterator ii = par.insetlist.begin();
	InsetList::const_iterator iend = par.insetlist.end();
	for (; ii != iend; ++ii) {
		Dimension dim;
		int const w = maxwidth_ - leftMargin(buffer, pit, ii->pos)
			- right_margin;
		LyXFont const & font = ii->inset->noFontChange() ?
			bufferfont :
			getFont(buffer, par, ii->pos);
		MetricsInfo mi(&bv, font, w);
		changed |= ii->inset->metrics(mi, dim);
	}

	// rebreak the paragraph
	par.rows().clear();
	Dimension dim;

	par.setBeginOfBody();
	pos_type z = 0;
	do {
		Row row(z);
		rowBreakPoint(buffer, right_margin, pit, row);
		setRowWidth(buffer, pit, row);
		setHeightOfRow(bv, pit, row);
		par.rows().push_back(row);
		dim.wid = std::max(dim.wid, row.width());
		dim.des += row.height();
		z = row.endpos();
	} while (z < par.size());

	// Make sure that if a par ends in newline, there is one more row
	// under it
	// FIXME this is a dirty trick. Now the _same_ position in the
	// paragraph occurs in _two_ different rows, and has two different
	// display positions, leading to weird behaviour when moving up/down.
	if (z > 0 && par.isNewline(z - 1)) {
		Row row(z - 1);
		row.endpos(z - 1);
		setRowWidth(buffer, pit, row);
		setHeightOfRow(bv, pit, row);
		par.rows().push_back(row);
		dim.des += row.height();
	}

	dim.asc += par.rows()[0].ascent();
	dim.des -= par.rows()[0].ascent();

	changed |= dim.height() != par.dim().height();

	par.dim() = dim;
	//lyxerr << "redoParagraph: " << par.rows().size() << " rows\n";

	return changed;
}


bool LyXText::metrics(MetricsInfo & mi, Dimension & dim)
{
	//BOOST_ASSERT(mi.base.textwidth);
	if (mi.base.textwidth)
		maxwidth_ = mi.base.textwidth;
	//lyxerr << "LyXText::metrics: width: " << mi.base.textwidth
	//	<< " maxWidth: " << maxwidth_ << "\nfont: " << mi.base.font << endl;
	// save the caller's font locally:
	font_ = mi.base.font;

	bool changed = false;

	unsigned int h = 0;
	unsigned int w = 0;
	for (pit_type pit = 0, n = paragraphs().size(); pit != n; ++pit) {
		changed |= redoParagraph(*mi.base.bv, pit);
		Paragraph & par = paragraphs()[pit];
		h += par.height();
		if (w < par.width())
			w = par.width();
	}

	dim.wid = w;
	dim.asc = pars_[0].ascent();
	dim.des = h - dim.asc;

	changed |= dim_ != dim;
	dim_ = dim;
	return changed;
}


// only used for inset right now. should also be used for main text
void LyXText::draw(PainterInfo & pi, int x, int y) const
{
	paintTextInset(*this, pi, x, y);
}


// only used for inset right now. should also be used for main text
void LyXText::drawSelection(PainterInfo & pi, int x, int) const
{
	LCursor & cur = pi.base.bv->cursor();
	if (!cur.selection())
		return;
	if (!ptr_cmp(cur.text(), this))
		return;

	if (lyxerr.debugging(Debug::DEBUG)) {
		lyxerr[Debug::DEBUG]
			<< BOOST_CURRENT_FUNCTION
			<< "draw selection at " << x
			<< endl;
	}

	DocIterator beg = cur.selectionBegin();
	DocIterator end = cur.selectionEnd();

	BufferView & bv = *pi.base.bv;
	Buffer const & buffer = *bv.buffer();

	// the selection doesn't touch the visible screen
	if (bv_funcs::status(&bv, beg) == bv_funcs::CUR_BELOW
	    || bv_funcs::status(&bv, end) == bv_funcs::CUR_ABOVE)
		return;

	Paragraph const & par1 = pars_[beg.pit()];
	Paragraph const & par2 = pars_[end.pit()];

	bool const above = (bv_funcs::status(&bv, beg)
			    == bv_funcs::CUR_ABOVE);
	bool const below = (bv_funcs::status(&bv, end)
			    == bv_funcs::CUR_BELOW);
	int y1,y2,x1,x2;
	if (above) {
		y1 = 0;
		y2 = 0;
		x1 = 0;
		x2 = dim_.wid;
	} else {
		Row const & row1 = par1.getRow(beg.pos(), beg.boundary());
		y1 = bv_funcs::getPos(bv, beg, beg.boundary()).y_ - row1.ascent();
		y2 = y1 + row1.height();
		int const startx = cursorX(buffer, beg.top(), beg.boundary());
		if (!isRTL(buffer, par1)) {
			x1 = startx;
			x2 = 0 + dim_.wid;
		}
		else {
			x1 = 0;
			x2 = startx;
		}
	}

	int Y1,Y2,X1,X2;
	if (below) {
		Y1 = bv.workHeight();
		Y2 = bv.workHeight();
		X1 = 0;
		X2 = dim_.wid;
	} else {
		Row const & row2 = par2.getRow(end.pos(), end.boundary());
		Y1 = bv_funcs::getPos(bv, end, end.boundary()).y_ - row2.ascent();
		Y2 = Y1 + row2.height();
		int const endx = cursorX(buffer, end.top(), end.boundary());
		if (!isRTL(buffer, par2)) {
			X1 = 0;
			X2 = endx;
		}
		else {
			X1 = endx;
			X2 = 0 + dim_.wid;
		}
	}

	if (!above && !below && &par1.getRow(beg.pos(), beg.boundary())
	    == &par2.getRow(end.pos(), end.boundary()))
	{
		// paint only one rectangle
		int const b( !isRTL(*bv.buffer(), par1) ? x + x1 : x + X1 );
		int const w( !isRTL(*bv.buffer(), par1) ? X2 - x1 : x2 - X1 );
		pi.pain.fillRectangle(b, y1, w, y2 - y1, LColor::selection);
		return;
	}

	if (lyxerr.debugging(Debug::DEBUG)) {
		lyxerr[Debug::DEBUG] << " y1: " << y1 << " y2: " << y2
			   << "X1:" << X1 << " x2: " << X2 << " wid: " << dim_.wid
			<< endl;
	}

	// paint upper rectangle
	pi.pain.fillRectangle(x + x1, y1, x2 - x1, y2 - y1,
				      LColor::selection);
	// paint bottom rectangle
	pi.pain.fillRectangle(x + X1, Y1, X2 - X1, Y2 - Y1,
				      LColor::selection);
	// paint center rectangle
	pi.pain.fillRectangle(x, y2, dim_.wid,
			      Y1 - y2, LColor::selection);
}

bool LyXText::isLastRow(pit_type pit, Row const & row) const
{
	return row.endpos() >= pars_[pit].size()
		&& pit + 1 == pit_type(paragraphs().size());
}


bool LyXText::isFirstRow(pit_type pit, Row const & row) const
{
	return row.pos() == 0 && pit == 0;
}


void LyXText::getWord(CursorSlice & from, CursorSlice & to,
	word_location const loc)
{
	Paragraph const & from_par = pars_[from.pit()];
	switch (loc) {
	case WHOLE_WORD_STRICT:
		if (from.pos() == 0 || from.pos() == from_par.size()
		    || !from_par.isLetter(from.pos())
		    || !from_par.isLetter(from.pos() - 1)) {
			to = from;
			return;
		}
		// no break here, we go to the next

	case WHOLE_WORD:
		// If we are already at the beginning of a word, do nothing
		if (!from.pos() || !from_par.isLetter(from.pos() - 1))
			break;
		// no break here, we go to the next

	case PREVIOUS_WORD:
		// always move the cursor to the beginning of previous word
		while (from.pos() && from_par.isLetter(from.pos() - 1))
			--from.pos();
		break;
	case NEXT_WORD:
		lyxerr << "LyXText::getWord: NEXT_WORD not implemented yet"
		       << endl;
		break;
	case PARTIAL_WORD:
		// no need to move the 'from' cursor
		break;
	}
	to = from;
	Paragraph & to_par = pars_[to.pit()];
	while (to.pos() < to_par.size() && to_par.isLetter(to.pos()))
		++to.pos();
}


void LyXText::write(Buffer const & buf, std::ostream & os) const
{
	ParagraphList::const_iterator pit = paragraphs().begin();
	ParagraphList::const_iterator end = paragraphs().end();
	depth_type dth = 0;
	for (; pit != end; ++pit)
		pit->write(buf, os, buf.params(), dth);
}


bool LyXText::read(Buffer const & buf, LyXLex & lex, ErrorList & errorList)
{
	depth_type depth = 0;

	while (lex.isOK()) {
		lex.nextToken();
		string const token = lex.getString();

		if (token.empty())
			continue;

		if (token == "\\end_inset")
			break;

		if (token == "\\end_body")
			continue;

		if (token == "\\begin_body")
			continue;

		if (token == "\\end_document")
			return false;

		if (token == "\\begin_layout") {
			lex.pushToken(token);

			Paragraph par;
			par.params().depth(depth);
			par.setFont(0, LyXFont(LyXFont::ALL_INHERIT, buf.params().language));
			pars_.push_back(par);

			// FIXME: goddamn InsetTabular makes us pass a Buffer
			// not BufferParams
			lyx::readParagraph(buf, pars_.back(), lex, errorList);

		} else if (token == "\\begin_deeper") {
			++depth;
		} else if (token == "\\end_deeper") {
			if (!depth) {
				lex.printError("\\end_deeper: " "depth is already null");
			} else {
				--depth;
			}
		} else {
			lyxerr << "Handling unknown body token: `"
			       << token << '\'' << endl;
		}
	}
	return true;
}


int LyXText::ascent() const
{
	return dim_.asc;
}


int LyXText::descent() const
{
	return dim_.des;
}


int LyXText::cursorX(Buffer const & buffer, CursorSlice const & sl,
		bool boundary) const
{
	pit_type const pit = sl.pit();
	Paragraph const & par = pars_[pit];
	if (par.rows().empty())
		return 0;

	pos_type ppos = sl.pos();
	// Correct position in front of big insets
	bool const boundary_correction = ppos != 0 && boundary;
	if (boundary_correction)
		--ppos;

	Row const & row = par.getRow(sl.pos(), boundary);

	pos_type cursor_vpos = 0;

	RowMetrics const m = computeRowMetrics(buffer, pit, row);
	double x = m.x;

	pos_type const row_pos  = row.pos();
	pos_type const end      = row.endpos();

	if (end <= row_pos)
		cursor_vpos = row_pos;
	else if (ppos >= end)
		cursor_vpos = isRTL(buffer, par) ? row_pos : end;
	else if (ppos > row_pos && ppos >= end)
		// Place cursor after char at (logical) position pos - 1
		cursor_vpos = (bidi.level(ppos - 1) % 2 == 0)
			? bidi.log2vis(ppos - 1) + 1 : bidi.log2vis(ppos - 1);
	else
		// Place cursor before char at (logical) position ppos
		cursor_vpos = (bidi.level(ppos) % 2 == 0)
			? bidi.log2vis(ppos) : bidi.log2vis(ppos) + 1;

	pos_type body_pos = par.beginOfBody();
	if (body_pos > 0 &&
	    (body_pos > end || !par.isLineSeparator(body_pos - 1)))
		body_pos = 0;

	// Use font span to speed things up, see below
	FontSpan font_span;
	LyXFont font;
	FontMetrics const & labelfm = theFontMetrics(
		getLabelFont(buffer, par));

	for (pos_type vpos = row_pos; vpos < cursor_vpos; ++vpos) {
		pos_type pos = bidi.vis2log(vpos);
		if (body_pos > 0 && pos == body_pos - 1) {
			// FIXME UNICODE
			docstring const lsep = from_utf8(par.layout()->labelsep);
			x += m.label_hfill + labelfm.width(lsep);
			if (par.isLineSeparator(body_pos - 1))
				x -= singleWidth(buffer, par, body_pos - 1);
		}

		// Use font span to speed things up, see above
		if (pos < font_span.first || pos > font_span.last) {
			font_span = par.fontSpan(pos);
			font = getFont(buffer, par, pos);
		}

		x += singleWidth(par, pos, par.getChar(pos), font);

		if (par.hfillExpansion(row, pos))
			x += (pos >= body_pos) ? m.hfill : m.label_hfill;
		else if (par.isSeparator(pos) && pos >= body_pos)
			x += m.separator;
	}

	// see correction above
	if (boundary_correction)
		if (getFont(buffer, par, ppos).isVisibleRightToLeft())
			x -= singleWidth(buffer, par, ppos);
		else
			x += singleWidth(buffer, par, ppos);

	// Make sure inside an inset we always count from the left
	// edge (bidi!) -- MV
	if (sl.pos() < par.size()) {
		font = getFont(buffer, par, sl.pos());
		if (!boundary && font.isVisibleRightToLeft()
		  && par.isInset(sl.pos()))
			x -= par.getInset(sl.pos())->width();
	}
	return int(x);
}


int LyXText::cursorY(CursorSlice const & sl, bool boundary) const
{
	//lyxerr << "LyXText::cursorY: boundary: " << boundary << std::endl;
	Paragraph const & par = getPar(sl.pit());
	if (par.rows().empty())
		return 0;

	int h = 0;
	h -= pars_[0].rows()[0].ascent();
	for (pit_type pit = 0; pit < sl.pit(); ++pit)
		h += pars_[pit].height();
	int pos = sl.pos();
	if (pos && boundary)
		--pos;
	size_t const rend = par.pos2row(pos);
	for (size_t rit = 0; rit != rend; ++rit)
		h += par.rows()[rit].height();
	h += par.rows()[rend].ascent();
	return h;
}


// Returns the current font and depth as a message.
docstring LyXText::currentState(LCursor & cur)
{
	BOOST_ASSERT(this == cur.text());
	Buffer & buf = cur.buffer();
	Paragraph const & par = cur.paragraph();
	odocstringstream os;

	if (buf.params().trackChanges)
		os << "[C] ";

	Change change = par.lookupChange(cur.pos());

	if (change.type != Change::UNCHANGED) {
		Author const & a = buf.params().authors().get(change.author);
		os << _("Change: ") << a.name();
		if (!a.email().empty())
			os << " (" << a.email() << ")";
		// FIXME ctime is english, we should translate that
		os << _(" at ") << ctime(&change.changetime);
		os << " : ";
	}

	// I think we should only show changes from the default
	// font. (Asger)
	// No, from the document font (MV)
	LyXFont font = real_current_font;
	font.reduce(buf.params().getFont());

	// FIXME UNICODE
	os << bformat(_("Font: %1$s"), from_utf8(font.stateText(&buf.params())));

	// The paragraph depth
	int depth = cur.paragraph().getDepth();
	if (depth > 0)
		os << bformat(_(", Depth: %1$d"), depth);

	// The paragraph spacing, but only if different from
	// buffer spacing.
	Spacing const & spacing = par.params().spacing();
	if (!spacing.isDefault()) {
		os << _(", Spacing: ");
		switch (spacing.getSpace()) {
		case Spacing::Single:
			os << _("Single");
			break;
		case Spacing::Onehalf:
			os << _("OneHalf");
			break;
		case Spacing::Double:
			os << _("Double");
			break;
		case Spacing::Other:
			os << _("Other (") << from_ascii(spacing.getValueAsString()) << ')';
			break;
		case Spacing::Default:
			// should never happen, do nothing
			break;
		}
	}

#ifdef DEVEL_VERSION
	os << _(", Inset: ") << &cur.inset();
	os << _(", Paragraph: ") << cur.pit();
	os << _(", Id: ") << par.id();
	os << _(", Position: ") << cur.pos();
	os << _(", Boundary: ") << cur.boundary();
//	Row & row = cur.textRow();
//	os << bformat(_(", Row b:%1$d e:%2$d"), row.pos(), row.endpos());
#endif
	return os.str();
}


docstring LyXText::getPossibleLabel(LCursor & cur) const
{
	pit_type pit = cur.pit();

	LyXLayout_ptr layout = pars_[pit].layout();

	if (layout->latextype == LATEX_PARAGRAPH && pit != 0) {
		LyXLayout_ptr const & layout2 = pars_[pit - 1].layout();
		if (layout2->latextype != LATEX_PARAGRAPH) {
			--pit;
			layout = layout2;
		}
	}

	docstring name = from_ascii(layout->latexname());

	// for captions, we want the abbreviation of the float type
	if (layout->labeltype == LABEL_SENSITIVE) {
		// Search for the first float or wrap inset in the iterator
		for (int i = cur.depth(); --i >= 0; ) {
			InsetBase * const in = &cur[i].inset();
			if (in->lyxCode() == InsetBase::FLOAT_CODE
			    || in->lyxCode() == InsetBase::WRAP_CODE) {
				name = in->getInsetName();
				break;
			}
		}
	}

	docstring text = name.substr(0, 3);
	if (name == "theorem")
		text = from_ascii("thm"); // Create a correct prefix for prettyref

	text += ':';
	if (layout->latextype == LATEX_PARAGRAPH || lyxrc.label_init_length < 0)
		text.erase();

	docstring par_text = pars_[pit].asString(cur.buffer(), false);
	for (int i = 0; i < lyxrc.label_init_length; ++i) {
		if (par_text.empty())
			break;
		docstring head;
		par_text = split(par_text, head, ' ');
		// Is it legal to use spaces in labels ?
		if (i > 0)
			text += '-';
		text += head;
	}

	return text;
}


//pos_type LyXText::x2pos(pit_type pit, int row, int x) const
//{
//	int lastx = 0;
//	int currx = 0;
//	Paragraph const & par = pars_[pit];
//	Row const & r = par.rows()[row];
//	int pos = r.pos();
//	for (; currx < x && pos < r.endpos(); ++pos) {
//		lastx = currx;
//		currx += singleWidth(buffer, par, pos);
//	}
//	if (abs(lastx - x) < abs(currx - x) && pos != r.pos())
//		--pos;
//	return pos;
//}


pos_type LyXText::x2pos(BufferView const & bv, pit_type pit, int row,
		int x) const
{
	BOOST_ASSERT(!pars_[pit].rows().empty());
	BOOST_ASSERT(row < int(pars_[pit].rows().size()));
	bool bound = false;
	Row const & r = pars_[pit].rows()[row];
	return r.pos() + getColumnNearX(bv, pit, r, x, bound);
}


//int LyXText::pos2x(pit_type pit, pos_type pos) const
//{
//	Paragraph const & par = pars_[pit];
//	Row const & r = par.rows()[row];
//	int x = 0;
//	pos -= r.pos();
//}


// x,y are screen coordinates
// sets cursor only within this LyXText
bool LyXText::setCursorFromCoordinates(LCursor & cur, int const x, int const y)
{
	BOOST_ASSERT(this == cur.text());
	pit_type pit = getPitNearY(cur.bv(), y);
	int yy = cur.bv().coordCache().get(this, pit).y_ - pars_[pit].ascent();
	lyxerr[Debug::DEBUG]
		<< BOOST_CURRENT_FUNCTION
		<< ": x: " << x
		<< " y: " << y
		<< " pit: " << pit
		<< " yy: " << yy << endl;

	Paragraph const & par = pars_[pit];
	int r = 0;
	BOOST_ASSERT(par.rows().size());
	for (; r < int(par.rows().size()) - 1; ++r) {
		Row const & row = par.rows()[r];
		if (int(yy + row.height()) > y)
			break;
		yy += row.height();
	}

	Row const & row = par.rows()[r];

	lyxerr[Debug::DEBUG]
		<< BOOST_CURRENT_FUNCTION
		<< ": row " << r
		<< " from pos: " << row.pos()
		<< endl;

	bool bound = false;
	int xx = x;
	pos_type const pos = row.pos() + getColumnNearX(cur.bv(), pit, row,
		xx, bound);

	lyxerr[Debug::DEBUG]
		<< BOOST_CURRENT_FUNCTION
		<< ": setting cursor pit: " << pit
		<< " pos: " << pos
		<< endl;

	return setCursor(cur, pit, pos, true, bound);
}


void LyXText::charsTranspose(LCursor & cur)
{
	BOOST_ASSERT(this == cur.text());

	pos_type pos = cur.pos();

	// If cursor is at beginning or end of paragraph, do nothing.
	if (pos == cur.lastpos() || pos == 0)
		return;

	Paragraph & par = cur.paragraph();

	// Get the positions of the characters to be transposed. 
	pos_type pos1 = pos - 1;
	pos_type pos2 = pos;

	// In change tracking mode, ignore deleted characters.
	while (pos2 < cur.lastpos() && par.isDeleted(pos2))
		++pos2;
	if (pos2 == cur.lastpos())
		return;

	while (pos1 >= 0 && par.isDeleted(pos1))
		--pos1;
	if (pos1 < 0)
		return;

	// Don't do anything if one of the "characters" is not regular text.
	if (par.isInset(pos1) || par.isInset(pos2))
		return;

	// Store the characters to be transposed (including font information).
	char_type char1 = par.getChar(pos1);
	LyXFont const font1 =
		par.getFontSettings(cur.buffer().params(), pos1);
	
	char_type char2 = par.getChar(pos2);
	LyXFont const font2 =
		par.getFontSettings(cur.buffer().params(), pos2);

	// And finally, we are ready to perform the transposition.
	// Track the changes if Change Tracking is enabled.
	bool const trackChanges = cur.buffer().params().trackChanges;

	recordUndo(cur);

	par.eraseChar(pos2, trackChanges);
	par.eraseChar(pos1, trackChanges);
	par.insertChar(pos1, char2, font2, trackChanges);
	par.insertChar(pos2, char1, font1, trackChanges);

	// After the transposition, move cursor to after the transposition.
	setCursor(cur, cur.pit(), pos2);
	cur.forwardPos();
}


} // namespace lyx
