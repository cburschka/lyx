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
#include "bufferparams.h"
#include "BufferView.h"
#include "cursor.h"
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
#include "lyxrow_funcs.h"
#include "metricsinfo.h"
#include "paragraph.h"
#include "paragraph_funcs.h"
#include "ParagraphParameters.h"
#include "rowpainter.h"
#include "undo.h"
#include "vspace.h"
#include "WordLangTuple.h"

#include "frontends/font_metrics.h"
#include "frontends/LyXView.h"

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
#include "support/tostr.h"

#include <sstream>

using lyx::par_type;
using lyx::pos_type;
using lyx::word_location;

using lyx::support::bformat;
using lyx::support::contains;
using lyx::support::lowercase;
using lyx::support::split;
using lyx::support::uppercase;

using lyx::cap::cutSelection;

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


unsigned int maxParagraphWidth(ParagraphList const & plist)
{
	unsigned int width = 0;
	ParagraphList::const_iterator pit = plist.begin();
	ParagraphList::const_iterator end = plist.end();
		for (; pit != end; ++pit)
			width = std::max(width, pit->width);
	return width;
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
	pos_type const last = row.endpos() - 1;
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
	string const & token, LyXFont & font)
{
	static Change change;

	BufferParams const & bp = buf.params();

	if (token[0] != '\\') {
		string::const_iterator cit = token.begin();
		for (; cit != token.end(); ++cit) {
			par.insertChar(par.size(), (*cit), font, change);
		}
	} else if (token == "\\begin_layout") {
		lex.eatLine();
		string layoutname = lex.getString();

		font = LyXFont(LyXFont::ALL_INHERIT, bp.language);
		change = Change();

		LyXTextClass const & tclass = bp.getLyXTextClass();

		if (layoutname.empty()) {
			layoutname = tclass.defaultLayoutName();
		}

		bool hasLayout = tclass.hasLayout(layoutname);

		if (!hasLayout) {
			buf.error(ErrorItem(_("Unknown layout"),
			bformat(_("Layout '%1$s' does not exists in textclass '%2$s'\nTrying to use the default instead.\n"),
				layoutname, tclass.name()), par.id(), 0, par.size()));
			layoutname = tclass.defaultLayoutName();
		}

		par.layout(bp.getLyXTextClass()[layoutname]);

		// Test whether the layout is obsolete.
		LyXLayout_ptr const & layout = par.layout();
		if (!layout->obsoleted_by().empty())
			par.layout(bp.getLyXTextClass()[layout->obsoleted_by()]);

		par.params().read(lex);

	} else if (token == "\\end_layout") {
		lyxerr << "Solitary \\end_layout in line " << lex.getLineNo() << "\n"
		       << "Missing \\begin_layout?.\n";
	} else if (token == "\\end_inset") {
		lyxerr << "Solitary \\end_inset in line " << lex.getLineNo() << "\n"
		       << "Missing \\begin_inset?.\n";
	} else if (token == "\\begin_inset") {
		InsetBase * inset = readInset(lex, buf);
		if (inset)
			par.insertInset(par.size(), inset, font, change);
		else {
			lex.eatLine();
			string line = lex.getString();
			buf.error(ErrorItem(_("Unknown Inset"), line,
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
	} else if (token == "\\bibitem") {
		InsetCommandParams p("bibitem", "dummy");
		auto_ptr<InsetBibitem> inset(new InsetBibitem(p));
		inset->read(buf, lex);
		par.insertInset(par.size(), inset.release(), font, change);
	} else if (token == "\\hfill") {
		par.insertInset(par.size(), new InsetHFill, font, change);
	} else if (token == "\\lyxline") {
		par.insertInset(par.size(), new InsetLine, font, change);
	} else if (token == "\\newpage") {
		par.insertInset(par.size(), new InsetPagebreak, font, change);
	} else if (token == "\\change_unchanged") {
		// Hack ! Needed for empty paragraphs :/
		// FIXME: is it still ??
		if (!par.size())
			par.cleanChanges();
		change = Change(Change::UNCHANGED);
	} else if (token == "\\change_inserted") {
		lex.nextToken();
		std::istringstream is(lex.getString());
		int aid;
		lyx::time_type ct;
		is >> aid >> ct;
		change = Change(Change::INSERTED, bp.author_map[aid], ct);
	} else if (token == "\\change_deleted") {
		lex.nextToken();
		std::istringstream is(lex.getString());
		int aid;
		lyx::time_type ct;
		is >> aid >> ct;
		change = Change(Change::DELETED, bp.author_map[aid], ct);
	} else {
		lex.eatLine();
		buf.error(ErrorItem(_("Unknown token"),
			bformat(_("Unknown token: %1$s %2$s\n"), token, lex.getString()),
			par.id(), 0, par.size()));
	}
}


void readParagraph(Buffer const & buf, Paragraph & par, LyXLex & lex)
{
	lex.nextToken();
	string token = lex.getString();
	LyXFont font;

	while (lex.isOK()) {

		readParToken(buf, par, lex, token, font);

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
}


} // namespace anon


BufferView * LyXText::bv()
{
	BOOST_ASSERT(bv_owner != 0);
	return bv_owner;
}


BufferView * LyXText::bv() const
{
	BOOST_ASSERT(bv_owner != 0);
	return bv_owner;
}


double LyXText::spacing(Paragraph const & par) const
{
	if (par.params().spacing().isDefault())
		return bv()->buffer()->params().spacing().getValue();
	return par.params().spacing().getValue();
}


void LyXText::updateParPositions()
{
	par_type pit = 0;
	par_type end = pars_.size();
	for (height_ = 0; pit != end; ++pit) {
		pars_[pit].y = height_;
		height_ += pars_[pit].height;
	}
}


int LyXText::width() const
{
	return width_;
}


int LyXText::height() const
{
	return height_;
}


int LyXText::singleWidth(Paragraph const & par, pos_type pos) const
{
	return singleWidth(par, pos, par.getChar(pos), getFont(par, pos));
}


int LyXText::singleWidth(Paragraph const & par,
			 pos_type pos, char c, LyXFont const & font) const
{
	BOOST_ASSERT(pos < par.size());

	// The most common case is handled first (Asger)
	if (IsPrintable(c)) {
		if (font.language()->RightToLeft()) {
			if ((lyxrc.font_norm_type == LyXRC::ISO_8859_6_8 ||
			     lyxrc.font_norm_type == LyXRC::ISO_10646_1)
			    && font.language()->lang() == "arabic") {
				if (Encodings::IsComposeChar_arabic(c))
					return 0;
				else
					c = par.transformChar(c, pos);
			} else if (font.language()->lang() == "hebrew" &&
				   Encodings::IsComposeChar_hebrew(c))
				return 0;
		}
		return font_metrics::width(c, font);
	}

	if (c == Paragraph::META_INSET)
		return par.getInset(pos)->width();

	if (IsSeparatorChar(c))
		c = ' ';
	return font_metrics::width(c, font);
}


int LyXText::leftMargin(par_type pit) const
{
	return leftMargin(pit, pars_[pit].size());
}


int LyXText::leftMargin(par_type const pit, pos_type const pos) const
{
	Paragraph const & par = pars_[pit];
	LyXTextClass const & tclass =
		bv()->buffer()->params().getLyXTextClass();
	LyXLayout_ptr const & layout = par.layout();

	string parindent = layout->parindent;

	int l_margin = 0;

	if (xo_ == 0)
		l_margin += changebarMargin();

	l_margin += font_metrics::signedWidth(tclass.leftmargin(), tclass.defaultfont());

	if (par.getDepth() != 0) {
	// find the next level paragraph
	par_type newpar = outerHook(pit, pars_);
		if (newpar != par_type(pars_.size())) {
			if (pars_[newpar].layout()->isEnvironment()) {
				l_margin = leftMargin(newpar);
			}
			if (par.layout() == tclass.defaultLayout()) {
				if (pars_[newpar].params().noindent())
					parindent.erase();
				else
					parindent = pars_[newpar].layout()->parindent;
			}
		}
	}

	LyXFont const labelfont = getLabelFont(par);
	switch (layout->margintype) {
	case MARGIN_DYNAMIC:
		if (!layout->leftmargin.empty())
			l_margin += font_metrics::signedWidth(layout->leftmargin,
						  tclass.defaultfont());
		if (!par.getLabelstring().empty()) {
			l_margin += font_metrics::signedWidth(layout->labelindent,
						  labelfont);
			l_margin += font_metrics::width(par.getLabelstring(),
					    labelfont);
			l_margin += font_metrics::width(layout->labelsep, labelfont);
		}
		break;

	case MARGIN_MANUAL:
		l_margin += font_metrics::signedWidth(layout->labelindent, labelfont);
		// The width of an empty par, even with manual label, should be 0
		if (!par.empty() && pos >= par.beginOfBody()) {
			if (!par.getLabelWidthString().empty()) {
				l_margin += font_metrics::width(par.getLabelWidthString(),
					       labelfont);
				l_margin += font_metrics::width(layout->labelsep, labelfont);
			}
		}
		break;

	case MARGIN_STATIC:
		l_margin += font_metrics::signedWidth(layout->leftmargin, tclass.defaultfont()) * 4
			/ (par.getDepth() + 4);
		break;

	case MARGIN_FIRST_DYNAMIC:
		if (layout->labeltype == LABEL_MANUAL) {
			if (pos >= par.beginOfBody()) {
				l_margin += font_metrics::signedWidth(layout->leftmargin,
							  labelfont);
			} else {
				l_margin += font_metrics::signedWidth(layout->labelindent,
							  labelfont);
			}
		} else if (pos != 0
			   // Special case to fix problems with
			   // theorems (JMarc)
			   || (layout->labeltype == LABEL_STATIC
			       && layout->latextype == LATEX_ENVIRONMENT
			       && !isFirstInSequence(pit, pars_))) {
			l_margin += font_metrics::signedWidth(layout->leftmargin,
						  labelfont);
		} else if (layout->labeltype != LABEL_TOP_ENVIRONMENT
			   && layout->labeltype != LABEL_BIBLIO
			   && layout->labeltype !=
			   LABEL_CENTERED_TOP_ENVIRONMENT) {
			l_margin += font_metrics::signedWidth(layout->labelindent,
						  labelfont);
			l_margin += font_metrics::width(layout->labelsep, labelfont);
			l_margin += font_metrics::width(par.getLabelstring(),
					    labelfont);
		}
		break;

	case MARGIN_RIGHT_ADDRESS_BOX: {
#if 0
		// ok, a terrible hack. The left margin depends on the widest
		// row in this paragraph.
		RowList::iterator rit = par.rows.begin();
		RowList::iterator end = par.rows.end();
#ifdef WITH_WARNINGS
#warning This is wrong.
#endif
		int minfill = maxwidth_;
		for ( ; rit != end; ++rit)
			if (rit->fill() < minfill)
				minfill = rit->fill();
		l_margin += font_metrics::signedWidth(layout->leftmargin,
			tclass.defaultfont());
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
	    // in tabulars and ert paragraphs are never indented!
	    && (par.ownerCode() != InsetBase::TEXT_CODE
	            && par.ownerCode() != InsetBase::ERT_CODE)
	    && (par.layout() != tclass.defaultLayout()
	        || bv()->buffer()->params().paragraph_separation ==
	           BufferParams::PARSEP_INDENT))
	{
		l_margin += font_metrics::signedWidth(parindent, tclass.defaultfont());
	}

	return l_margin;
}


int LyXText::rightMargin(Paragraph const & par) const
{
	LyXTextClass const & tclass = bv()->buffer()->params().getLyXTextClass();

	// We do not want rightmargins on inner texts.
	if (bv()->text() != this)
		return 0;

	int const r_margin =
		::rightMargin()
		+ font_metrics::signedWidth(tclass.rightmargin(),
					    tclass.defaultfont())
		+ font_metrics::signedWidth(par.layout()->rightmargin,
					    tclass.defaultfont())
		* 4 / (par.getDepth() + 4);

	return r_margin;

}


int LyXText::labelEnd(par_type const pit) const
{
	// labelEnd is only needed if the layout fills a flushleft label.
	if (pars_[pit].layout()->margintype != MARGIN_MANUAL)
		return 0;
	// return the beginning of the body
	return leftMargin(pit);
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


void LyXText::rowBreakPoint(par_type const pit, Row & row) const
{
	Paragraph const & par = pars_[pit];
	pos_type const end = par.size();
	pos_type const pos = row.pos();
	if (pos == end) {
		row.endpos(end);
		return;
	}

	// maximum pixel width of a row
	int width = maxwidth_ - rightMargin(par); // - leftMargin(pit, row);
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

	int const left = leftMargin(pit, pos);
	int x = left;

	// pixel width since last breakpoint
	int chunkwidth = 0;

	FontIterator fi = FontIterator(*this, pit, pos);
	pos_type point = end;
	pos_type i = pos;
	for ( ; i < end; ++i, ++fi) {
		char const c = par.getChar(i);

		{
			int thiswidth = singleWidth(par, i, c, *fi);

			// add the auto-hfill from label end to the body
			if (body_pos && i == body_pos) {
				int add = font_metrics::width(layout->labelsep, getLabelFont(par));
				if (par.isLineSeparator(i - 1))
					add -= singleWidth(par, i - 1);

				add = std::max(add, labelEnd(pit) - x);
				thiswidth += add;
			}

			x += thiswidth;
			chunkwidth += thiswidth;
		}

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


void LyXText::setRowWidth(par_type const pit, Row & row) const
{
	// get the pure distance
	pos_type const end = row.endpos();

	Paragraph const & par = pars_[pit];
	string labelsep = par.layout()->labelsep;
	int w = leftMargin(pit, row.pos());

	pos_type const body_pos = par.beginOfBody();
	pos_type i = row.pos();

	if (i < end) {
		FontIterator fi = FontIterator(*this, pit, i);
		for ( ; i < end; ++i, ++fi) {
			if (body_pos > 0 && i == body_pos) {
				w += font_metrics::width(labelsep, getLabelFont(par));
				if (par.isLineSeparator(i - 1))
					w -= singleWidth(par, i - 1);
				w = max(w, labelEnd(pit));
			}
			char const c = par.getChar(i);
			w += singleWidth(par, i, c, *fi);
		}
	}

	if (body_pos > 0 && body_pos >= end) {
		w += font_metrics::width(labelsep, getLabelFont(par));
		if (end > 0 && par.isLineSeparator(end - 1))
			w -= singleWidth(par, end - 1);
		w = max(w, labelEnd(pit));
	}

	row.width(w + rightMargin(par));
}


// returns the minimum space a manual label needs on the screen in pixel
int LyXText::labelFill(Paragraph const & par, Row const & row) const
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
		w += singleWidth(par, i);

	string const & label = par.params().labelWidthString();
	if (label.empty())
		return 0;

	return max(0, font_metrics::width(label, getLabelFont(par)) - w);
}


LColor_color LyXText::backgroundColor() const
{
	return LColor_color(LColor::color(background_color_));
}


void LyXText::setHeightOfRow(par_type const pit, Row & row)
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
	LyXFont font = getFont(par, row.pos());
	LyXFont::FONT_SIZE const tmpsize = font.size();
	font = getLayoutFont(pit);
	LyXFont::FONT_SIZE const size = font.size();
	font.setSize(tmpsize);

	LyXFont labelfont = getLabelFont(par);

	// these are minimum values
	double const spacing_val = layout->spacing.getValue() * spacing(par);
	//lyxerr << "spacing_val = " << spacing_val << endl;
	int maxasc  = int(font_metrics::maxAscent(font)  * spacing_val);
	int maxdesc = int(font_metrics::maxDescent(font) * spacing_val);

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
		maxasc  = max(maxasc,  font_metrics::maxAscent(font));
		maxdesc = max(maxdesc, font_metrics::maxDescent(font));
	}

	// This is nicer with box insets:
	++maxasc;
	++maxdesc;

	row.ascent_of_text(maxasc);

	// is it a top line?
	if (row.pos() == 0) {
		BufferParams const & bufparams = bv()->buffer()->params();
		// some parksips VERY EASY IMPLEMENTATION
		if (bv()->buffer()->params().paragraph_separation
		    == BufferParams::PARSEP_SKIP
			&& pit != 0
			&& ((layout->isParagraph() && par.getDepth() == 0)
			    || (pars_[pit - 1].layout()->isParagraph()
			        && pars_[pit - 1].getDepth() == 0)))
		{
				maxasc += bufparams.getDefSkip().inPixels(*bv());
		}

		if (pars_[pit].params().startOfAppendix())
			maxasc += int(3 * dh);

		// This is special code for the chapter, since the label of this
		// layout is printed in an extra row
		if (layout->counter == "chapter" && bufparams.secnumdepth >= 0) {
			labeladdon = int(font_metrics::maxHeight(labelfont)
			             * layout->spacing.getValue()
			             * spacing(par));
		}

		// special code for the top label
		if ((layout->labeltype == LABEL_TOP_ENVIRONMENT
		     || layout->labeltype == LABEL_BIBLIO
		     || layout->labeltype == LABEL_CENTERED_TOP_ENVIRONMENT)
		    && isFirstInSequence(pit, paragraphs())
		    && !par.getLabelstring().empty())
		{
			labeladdon = int(
				  font_metrics::maxHeight(labelfont)
					* layout->spacing.getValue()
					* spacing(par)
				+ (layout->topsep + layout->labelbottomsep) * dh);
		}

		// Add the layout spaces, for example before and after
		// a section, or between the items of a itemize or enumerate
		// environment.

		par_type prev = depthHook(pit, pars_, par.getDepth());
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
		if (prev != par_type(pars_.size())) {
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
		par_type nextpit = pit + 1;
		if (nextpit != par_type(pars_.size())) {
			par_type cpit = pit;
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

	row.height(maxasc + maxdesc + labeladdon);
	row.baseline(maxasc + labeladdon);
	row.top_of_text(row.baseline() - font_metrics::maxAscent(font));
}


namespace {

}

void LyXText::breakParagraph(LCursor & cur, char keep_layout)
{
	BOOST_ASSERT(this == cur.text());
	// allow only if at start or end, or all previous is new text
	Paragraph & cpar = cur.paragraph();
	par_type cpit = cur.par();

	if (cur.pos() != 0 && cur.pos() != cur.lastpos()
	    && cpar.isChangeEdited(0, cur.pos()))
		return;

	LyXTextClass const & tclass = cur.buffer().params().getLyXTextClass();
	LyXLayout_ptr const & layout = cpar.layout();

	// this is only allowed, if the current paragraph is not empty
	// or caption and if it has not the keepempty flag active
	if (cur.lastpos() == 0 && !cpar.allowEmpty()
	   && layout->labeltype != LABEL_SENSITIVE)
		return;

	// a layout change may affect also the following paragraph
	recUndo(cur.par(), undoSpan(cur.par()) - 1);

	// Always break behind a space
	// It is better to erase the space (Dekel)
	if (cur.pos() != cur.lastpos() && cpar.isLineSeparator(cur.pos()))
		cpar.erase(cur.pos());

	// break the paragraph
	if (keep_layout)
		keep_layout = 2;
	else
		keep_layout = layout->isEnvironment();

	// we need to set this before we insert the paragraph. IMO the
	// breakParagraph call should return a bool if it inserts the
	// paragraph before or behind and we should react on that one
	// but we can fix this in 1.3.0 (Jug 20020509)
	bool const isempty = cpar.allowEmpty() && cpar.empty();
	::breakParagraph(cur.buffer().params(), paragraphs(), cpit,
			 cur.pos(), keep_layout);

	cpit = cur.par();
	par_type next_par = cpit + 1;

	// well this is the caption hack since one caption is really enough
	if (layout->labeltype == LABEL_SENSITIVE) {
		if (cur.pos() == 0)
			// set to standard-layout
			pars_[cpit].applyLayout(tclass.defaultLayout());
		else
			// set to standard-layout
			pars_[next_par].applyLayout(tclass.defaultLayout());
	}

	// if the cursor is at the beginning of a row without prior newline,
	// move one row up!
	// This touches only the screen-update. Otherwise we would may have
	// an empty row on the screen
	if (cur.pos() != 0 && cur.textRow().pos() == cur.pos()
	    && !pars_[cpit].isNewline(cur.pos() - 1))
	{
		cursorLeft(cur);
	}

	while (!pars_[next_par].empty() && pars_[next_par].isNewline(0))
		pars_[next_par].erase(0);

	updateCounters();
	redoParagraph(cpit);
	redoParagraph(next_par);

	// This check is necessary. Otherwise the new empty paragraph will
	// be deleted automatically. And it is more friendly for the user!
	if (cur.pos() != 0 || isempty)
		setCursor(cur, cur.par() + 1, 0);
	else
		setCursor(cur, cur.par(), 0);
}


// convenience function
void LyXText::redoParagraph(LCursor & cur)
{
	BOOST_ASSERT(this == cur.text());
	cur.clearSelection();
	redoParagraph(cur.par());
	setCursorIntern(cur, cur.par(), cur.pos());
}


// insert a character, moves all the following breaks in the
// same Paragraph one to the right and make a rebreak
void LyXText::insertChar(LCursor & cur, char c)
{
	BOOST_ASSERT(this == cur.text());
	BOOST_ASSERT(c != Paragraph::META_INSET);

	recordUndo(cur, Undo::INSERT);

	Paragraph & par = cur.paragraph();
	// try to remove this
	par_type const pit = cur.par();

	bool const freeSpacing = par.layout()->free_spacing ||
		par.isFreeSpacing();

	if (lyxrc.auto_number) {
		static string const number_operators = "+-/*";
		static string const number_unary_operators = "+-";
		static string const number_seperators = ".,:";

		if (current_font.number() == LyXFont::ON) {
			if (!IsDigit(c) && !contains(number_operators, c) &&
			    !(contains(number_seperators, c) &&
			      cur.pos() != 0 &&
			      cur.pos() != cur.lastpos() &&
			      getFont(par, cur.pos()).number() == LyXFont::ON &&
			      getFont(par, cur.pos() - 1).number() == LyXFont::ON)
			   )
				number(cur); // Set current_font.number to OFF
		} else if (IsDigit(c) &&
			   real_current_font.isVisibleRightToLeft()) {
			number(cur); // Set current_font.number to ON

			if (cur.pos() != 0) {
				char const c = par.getChar(cur.pos() - 1);
				if (contains(number_unary_operators, c) &&
				    (cur.pos() == 1
				     || par.isSeparator(cur.pos() - 2)
				     || par.isNewline(cur.pos() - 2))
				  ) {
					setCharFont(pit, cur.pos() - 1, current_font);
				} else if (contains(number_seperators, c)
				     && cur.pos() >= 2
				     && getFont(par, cur.pos() - 2).number() == LyXFont::ON) {
					setCharFont(pit, cur.pos() - 1, current_font);
				}
			}
		}
	}

	// First check, if there will be two blanks together or a blank at
	// the beginning of a paragraph.
	// I decided to handle blanks like normal characters, the main
	// difference are the special checks when calculating the row.fill
	// (blank does not count at the end of a row) and the check here

	// The bug is triggered when we type in a description environment:
	// The current_font is not changed when we go from label to main text
	// and it should (along with realtmpfont) when we type the space.
	// CHECK There is a bug here! (Asger)

	// store the current font.  This is because of the use of cursor
	// movements. The moving cursor would refresh the current font
	LyXFont realtmpfont = real_current_font;
	LyXFont rawtmpfont = current_font;

	// When the free-spacing option is set for the current layout,
	// disable the double-space checking
	if (!freeSpacing && IsLineSeparatorChar(c)) {
		if (cur.pos() == 0) {
			static bool sent_space_message = false;
			if (!sent_space_message) {
				cur.message(_("You cannot insert a space at the "
					"beginning of a paragraph. Please read the Tutorial."));
				sent_space_message = true;
				return;
			}
		}
		BOOST_ASSERT(cur.pos() > 0);
		if (par.isLineSeparator(cur.pos() - 1)
		    || par.isNewline(cur.pos() - 1)) {
			static bool sent_space_message = false;
			if (!sent_space_message) {
				cur.message(_("You cannot type two spaces this way. "
					"Please read the Tutorial."));
				sent_space_message = true;
			}
			return;
		}
	}

	par.insertChar(cur.pos(), c, rawtmpfont);

	current_font = rawtmpfont;
	real_current_font = realtmpfont;
	redoParagraph(cur);
	setCursor(cur, cur.par(), cur.pos() + 1, false, cur.boundary());
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


RowMetrics LyXText::computeRowMetrics(par_type const pit, Row const & row) const
{
	RowMetrics result;
	Paragraph const & par = pars_[pit];

	double w = width_ - row.width();

	bool const is_rtl = isRTL(par);
	if (is_rtl)
		result.x = rightMargin(par);
	else
		result.x = leftMargin(pit, row.pos());

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
			result.label_hfill = labelFill(par, row) / double(nlh);
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

	bidi.computeTables(par, *bv()->buffer(), row);
	if (is_rtl) {
		pos_type body_pos = par.beginOfBody();
		pos_type end = row.endpos();

		if (body_pos > 0
		    && (body_pos > end || !par.isLineSeparator(body_pos - 1)))
		{
			result.x += font_metrics::width(layout->labelsep, getLabelFont(par));
			if (body_pos <= end)
				result.x += result.label_hfill;
		}
	}

	return result;
}


// the cursor set functions have a special mechanism. When they
// realize, that you left an empty paragraph, they will delete it.

void LyXText::cursorRightOneWord(LCursor & cur)
{
	BOOST_ASSERT(this == cur.text());
	if (cur.pos() == cur.lastpos() && cur.par() != cur.lastpar()) {
		++cur.par();
		cur.pos() = 0;
	} else {
		// Skip through initial nonword stuff.
		// Treat floats and insets as words.
		while (cur.pos() != cur.lastpos() && !cur.paragraph().isWord(cur.pos()))
			++cur.pos();
		// Advance through word.
		while (cur.pos() != cur.lastpos() && cur.paragraph().isWord(cur.pos()))
			++cur.pos();
	}
	setCursor(cur, cur.par(), cur.pos());
}


void LyXText::cursorLeftOneWord(LCursor & cur)
{
	BOOST_ASSERT(this == cur.text());
	if (cur.pos() == 0 && cur.par() != 0) {
		--cur.par();
		cur.pos() = cur.lastpos();
	} else {
		// Skip through initial nonword stuff.
		// Treat floats and insets as words.
		while (cur.pos() != 0 && !cur.paragraph().isWord(cur.pos() - 1))
			--cur.pos();
		// Advance through word.
		while (cur.pos() != 0 && cur.paragraph().isWord(cur.pos() - 1))
			--cur.pos();
	}
	setCursor(cur, cur.par(), cur.pos());
}


void LyXText::selectWord(LCursor & cur, word_location loc)
{
	BOOST_ASSERT(this == cur.text());
	CursorSlice from = cur.top();
	CursorSlice to = cur.top();
	getWord(from, to, loc);
	if (cur.top() != from)
		setCursor(cur, from.par(), from.pos());
	if (to == from)
		return;
	cur.resetAnchor();
	setCursor(cur, to.par(), to.pos());
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
	BOOST_ASSERT(this == cur.text());
	if (!cur.selection() && cur.lastpos() != 0)
		return;

	CursorSlice const & startc = cur.selBegin();
	CursorSlice const & endc = cur.selEnd();
	if (startc.par() == endc.par()) {
		recordUndoSelection(cur, Undo::INSERT);
		pars_[startc.par()].acceptChange(startc.pos(), endc.pos());
		finishUndo();
		cur.clearSelection();
		redoParagraph(startc.par());
		setCursorIntern(cur, startc.par(), 0);
	}
#ifdef WITH_WARNINGS
#warning handle multi par selection
#endif
}


void LyXText::rejectChange(LCursor & cur)
{
	BOOST_ASSERT(this == cur.text());
	if (!cur.selection() && cur.lastpos() != 0)
		return;

	CursorSlice const & startc = cur.selBegin();
	CursorSlice const & endc = cur.selEnd();
	if (startc.par() == endc.par()) {
		recordUndoSelection(cur, Undo::INSERT);
		pars_[startc.par()].rejectChange(startc.pos(), endc.pos());
		finishUndo();
		cur.clearSelection();
		redoParagraph(startc.par());
		setCursorIntern(cur, startc.par(), 0);
	}
#ifdef WITH_WARNINGS
#warning handle multi par selection
#endif
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
		getWord(from, to, lyx::PARTIAL_WORD);
		setCursor(cur, to.par(), to.pos() + 1);
	}

	recordUndoSelection(cur);

	pos_type pos = from.pos();
	int par = from.par();

	while (par != int(pars_.size()) && (pos != to.pos() || par != to.par())) {
		par_type pit = par;
		if (pos == pars_[pit].size()) {
			++par;
			pos = 0;
			continue;
		}
		unsigned char c = pars_[pit].getChar(pos);
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
#ifdef WITH_WARNINGS
#warning changes
#endif
		pars_[pit].setChar(pos, c);
		++pos;
	}
}


void LyXText::Delete(LCursor & cur)
{
	BOOST_ASSERT(this == cur.text());
	if (cur.pos() != cur.lastpos()) {
		recordUndo(cur, Undo::DELETE, cur.par());
		setCursorIntern(cur, cur.par(), cur.pos() + 1, false, cur.boundary());
		backspace(cur);
	}
	// should we do anything in an else branch?
}


void LyXText::backspace(LCursor & cur)
{
	BOOST_ASSERT(this == cur.text());
	if (cur.pos() == 0) {
		// The cursor is at the beginning of a paragraph, so
		// the the backspace will collapse two paragraphs into
		// one.

		// but it's not allowed unless it's new
		Paragraph & par = cur.paragraph();
		if (par.isChangeEdited(0, par.size()))
			return;

		// we may paste some paragraphs

		// is it an empty paragraph?
		pos_type lastpos = cur.lastpos();
		if (lastpos == 0 || (lastpos == 1 && par.isSeparator(0))) {
			// This is an empty paragraph and we delete it just
			// by moving the cursor one step
			// left and let the DeleteEmptyParagraphMechanism
			// handle the actual deletion of the paragraph.

			if (cur.par() != 0) {
				cursorLeft(cur);
				// the layout things can change the height of a row !
				redoParagraph(cur);
				return;
			}
		}

		if (cur.par() != 0)
			recordUndo(cur, Undo::DELETE, cur.par() - 1);

		par_type tmppit = cur.par();
		// We used to do cursorLeftIntern() here, but it is
		// not a good idea since it triggers the auto-delete
		// mechanism. So we do a cursorLeftIntern()-lite,
		// without the dreaded mechanism. (JMarc)
		if (cur.par() != 0) {
			// steps into the above paragraph.
			setCursorIntern(cur, cur.par() - 1,
					pars_[cur.par() - 1].size(),
					false);
		}

		// Pasting is not allowed, if the paragraphs have different
		// layout. I think it is a real bug of all other
		// word processors to allow it. It confuses the user.
		// Correction: Pasting is always allowed with standard-layout
		Buffer & buf = cur.buffer();
		BufferParams const & bufparams = buf.params();
		LyXTextClass const & tclass = bufparams.getLyXTextClass();
		par_type const cpit = cur.par();

		if (cpit != tmppit
		    && (pars_[cpit].layout() == pars_[tmppit].layout()
		        || pars_[tmppit].layout() == tclass.defaultLayout())
		    && pars_[cpit].getAlign() == pars_[tmppit].getAlign()) {
			mergeParagraph(bufparams, buf.paragraphs(), cpit);

			if (cur.pos() != 0 && pars_[cpit].isSeparator(cur.pos() - 1))
				--cur.pos();

			// the counters may have changed
			updateCounters();
			setCursor(cur, cur.par(), cur.pos(), false);
		}
	} else {
		// this is the code for a normal backspace, not pasting
		// any paragraphs
		recordUndo(cur, Undo::DELETE);
		// We used to do cursorLeftIntern() here, but it is
		// not a good idea since it triggers the auto-delete
		// mechanism. So we do a cursorLeftIntern()-lite,
		// without the dreaded mechanism. (JMarc)
		setCursorIntern(cur, cur.par(), cur.pos() - 1,
				false, cur.boundary());
		cur.paragraph().erase(cur.pos());
	}

	if (cur.pos() == cur.lastpos())
		setCurrentFont(cur);

	redoParagraph(cur);
	setCursor(cur, cur.par(), cur.pos(), false, cur.boundary());
}


Paragraph & LyXText::getPar(par_type par) const
{
	//lyxerr << "getPar: " << par << " from " << paragraphs().size() << endl;
	BOOST_ASSERT(par >= 0);
	BOOST_ASSERT(par < int(paragraphs().size()));
	return paragraphs()[par];
}


Row const & LyXText::firstRow() const
{
	return *paragraphs().front().rows.begin();
}


void LyXText::redoParagraphInternal(par_type const pit)
{
	// remove rows of paragraph, keep track of height changes
	Paragraph & par = pars_[pit];
	height_ -= par.height;

	// clear old data
	par.rows.clear();
	par.height = 0;
	par.width = 0;

	// redo insets
	InsetList::iterator ii = par.insetlist.begin();
	InsetList::iterator iend = par.insetlist.end();
	for (; ii != iend; ++ii) {
		Dimension dim;
		int const w = maxwidth_ - leftMargin(pit) - rightMargin(par);
		MetricsInfo mi(bv(), getFont(par, ii->pos), w);
		ii->inset->metrics(mi, dim);
	}

	// rebreak the paragraph
	par.setBeginOfBody();
	pos_type z = 0;
	do {
		Row row(z);
		rowBreakPoint(pit, row);
		setRowWidth(pit, row);
		setHeightOfRow(pit, row);
		row.y_offset(par.height);
		par.rows.push_back(row);
		par.width = std::max(par.width, row.width());
		par.height += row.height();
		z = row.endpos();
	} while (z < par.size());

	height_ += par.height;
	//lyxerr << "redoParagraph: " << par.rows.size() << " rows\n";
}


void LyXText::redoParagraphs(par_type pit, par_type end)
{
	for (; pit != end; ++pit)
		redoParagraphInternal(pit);
	updateParPositions();
	updateCounters();
}


void LyXText::redoParagraph(par_type pit)
{
	redoParagraphInternal(pit);
	updateParPositions();
}


void LyXText::fullRebreak()
{
	redoParagraphs(0, paragraphs().size());
	bv()->cursor().resetAnchor();
}


void LyXText::metrics(MetricsInfo & mi, Dimension & dim)
{
	//BOOST_ASSERT(mi.base.textwidth);
	if (mi.base.textwidth)
		maxwidth_ = mi.base.textwidth;
	//lyxerr << "LyXText::metrics: width: " << mi.base.textwidth
	//<< " maxWidth: " << maxwidth << "\nfont: " << mi.base.font
	//<< endl;

	// Rebuild row cache. This recomputes height as well.
	redoParagraphs(0, paragraphs().size());

	width_ = maxParagraphWidth(paragraphs());

	// final dimension
	dim.asc = firstRow().ascent_of_text();
	dim.des = height_ - dim.asc;
	dim.wid = width_;
}


// only used for inset right now. should also be used for main text
void LyXText::draw(PainterInfo & pi, int x, int y) const
{
	xo_ = x;
	yo_ = y;
	paintTextInset(*this, pi);
}


// only used for inset right now. should also be used for main text
void LyXText::drawSelection(PainterInfo &, int, int) const
{
	//lyxerr << "LyXText::drawSelection at " << x << " " << y << endl;
}


bool LyXText::isLastRow(par_type pit, Row const & row) const
{
	return row.endpos() >= pars_[pit].size()
		&& pit + 1 == par_type(paragraphs().size());
}


bool LyXText::isFirstRow(par_type pit, Row const & row) const
{
	return row.pos() == 0 && pit == 0;
}


void LyXText::getWord(CursorSlice & from, CursorSlice & to,
	word_location const loc)
{
	Paragraph const & from_par = pars_[from.par()];
	switch (loc) {
	case lyx::WHOLE_WORD_STRICT:
		if (from.pos() == 0 || from.pos() == from_par.size()
		    || !from_par.isWord(from.pos())
		    || !from_par.isWord(from.pos() - 1)) {
			to = from;
			return;
		}
		// no break here, we go to the next

	case lyx::WHOLE_WORD:
		// If we are already at the beginning of a word, do nothing
		if (!from.pos() || !from_par.isWord(from.pos() - 1))
			break;
		// no break here, we go to the next

	case lyx::PREVIOUS_WORD:
		// always move the cursor to the beginning of previous word
		while (from.pos() && from_par.isWord(from.pos() - 1))
			--from.pos();
		break;
	case lyx::NEXT_WORD:
		lyxerr << "LyXText::getWord: NEXT_WORD not implemented yet"
		       << endl;
		break;
	case lyx::PARTIAL_WORD:
		// no need to move the 'from' cursor
		break;
	}
	to = from;
	Paragraph & to_par = pars_[to.par()];
	while (to.pos() < to_par.size() && to_par.isWord(to.pos()))
		++to.pos();
}


void LyXText::write(Buffer const & buf, std::ostream & os) const
{
	ParagraphList::const_iterator pit = paragraphs().begin();
	ParagraphList::const_iterator end = paragraphs().end();
	Paragraph::depth_type dth = 0;
	for (; pit != end; ++pit)
		pit->write(buf, os, buf.params(), dth);
}


bool LyXText::read(Buffer const & buf, LyXLex & lex)
{
	static Change current_change;

	Paragraph::depth_type depth = 0;

	while (lex.isOK()) {
		lex.nextToken();
		string const token = lex.getString();

		if (token.empty())
			continue;

		if (token == "\\end_inset") {
			break;
		}

		if (token == "\\end_body") {
			continue;
		}

		if (token == "\\begin_body") {
			continue;
		}

		if (token == "\\end_document") {
			return false;
		}

		if (token == "\\begin_layout") {
			lex.pushToken(token);

			Paragraph par;
			par.params().depth(depth);
			if (buf.params().tracking_changes)
				par.trackChanges();
			par.setFont(0, LyXFont(LyXFont::ALL_INHERIT, buf.params().language));
			pars_.push_back(par);

			// FIXME: goddamn InsetTabular makes us pass a Buffer
			// not BufferParams
			::readParagraph(buf, pars_.back(), lex);

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
	return firstRow().ascent_of_text();
}


int LyXText::descent() const
{
	return height_ - firstRow().ascent_of_text();
}


int LyXText::cursorX(CursorSlice const & cur) const
{
	par_type const pit = cur.par();
	Paragraph const & par = pars_[pit];
	if (par.rows.empty())
		return xo_;

	Row const & row = *par.getRow(cur.pos());

	pos_type pos = cur.pos();
	pos_type cursor_vpos = 0;

	RowMetrics const m = computeRowMetrics(pit, row);
	double x = m.x;

	pos_type const row_pos  = row.pos();
	pos_type const end      = row.endpos();

	if (end <= row_pos)
		cursor_vpos = row_pos;
	else if (pos >= end)
		cursor_vpos = isRTL(par) ? row_pos : end;
	else if (pos > row_pos && pos >= end)
		// Place cursor after char at (logical) position pos - 1
		cursor_vpos = (bidi.level(pos - 1) % 2 == 0)
			? bidi.log2vis(pos - 1) + 1 : bidi.log2vis(pos - 1);
	else
		// Place cursor before char at (logical) position pos
		cursor_vpos = (bidi.level(pos) % 2 == 0)
			? bidi.log2vis(pos) : bidi.log2vis(pos) + 1;

	pos_type body_pos = par.beginOfBody();
	if (body_pos > 0 &&
	    (body_pos > end || !par.isLineSeparator(body_pos - 1)))
		body_pos = 0;

	for (pos_type vpos = row_pos; vpos < cursor_vpos; ++vpos) {
		pos_type pos = bidi.vis2log(vpos);
		if (body_pos > 0 && pos == body_pos - 1) {
			x += m.label_hfill
				+ font_metrics::width(par.layout()->labelsep,
						      getLabelFont(par));
			if (par.isLineSeparator(body_pos - 1))
				x -= singleWidth(par, body_pos - 1);
		}

		if (hfillExpansion(par, row, pos)) {
			x += singleWidth(par, pos);
			if (pos >= body_pos)
				x += m.hfill;
			else
				x += m.label_hfill;
		} else if (par.isSeparator(pos)) {
			x += singleWidth(par, pos);
			if (pos >= body_pos)
				x += m.separator;
		} else
			x += singleWidth(par, pos);
	}
	return xo_ + int(x);
}


int LyXText::cursorY(CursorSlice const & cur) const
{
	Paragraph const & par = getPar(cur.par());
	Row const & row = *par.getRow(cur.pos());
	return yo_ + par.y + row.y_offset() + row.baseline();
}


// Returns the current font and depth as a message.
string LyXText::currentState(LCursor & cur)
{
	BOOST_ASSERT(this == cur.text());
	Buffer & buf = cur.buffer();
	Paragraph const & par = cur.paragraph();
	std::ostringstream os;

	bool const show_change = buf.params().tracking_changes
		&& cur.pos() != cur.lastpos()
		&& par.lookupChange(cur.pos()) != Change::UNCHANGED;

	if (show_change) {
		Change change = par.lookupChangeFull(cur.pos());
		Author const & a = buf.params().authors().get(change.author);
		os << _("Change: ") << a.name();
		if (!a.email().empty())
			os << " (" << a.email() << ")";
		if (change.changetime)
			os << _(" at ") << ctime(&change.changetime);
		os << " : ";
	}

	// I think we should only show changes from the default
	// font. (Asger)
	LyXFont font = real_current_font;
	font.reduce(buf.params().getLyXTextClass().defaultfont());

	// avoid _(...) re-entrance problem
	string const s = font.stateText(&buf.params());
	os << bformat(_("Font: %1$s"), s);

	// os << bformat(_("Font: %1$s"), font.stateText(&buf.params));

	// The paragraph depth
	int depth = cur.paragraph().getDepth();
	if (depth > 0)
		os << bformat(_(", Depth: %1$s"), tostr(depth));

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
			os << _("Other (") << spacing.getValue() << ')';
			break;
		case Spacing::Default:
			// should never happen, do nothing
			break;
		}
	}

#ifdef DEVEL_VERSION
	os << _(", Inset: ") << &cur.inset();
	os << _(", Paragraph: ") << cur.par();
	os << _(", Id: ") << par.id();
	os << _(", Position: ") << cur.pos();
	Row & row = cur.textRow();
	os << bformat(_(", Row b:%1$d e:%2$d"), row.pos(), row.endpos());
#endif
	return os.str();
}


string LyXText::getPossibleLabel(LCursor & cur) const
{
	par_type pit = cur.par();

	LyXLayout_ptr layout = pars_[pit].layout();

	if (layout->latextype == LATEX_PARAGRAPH && pit != 0) {
		LyXLayout_ptr const & layout2 = pars_[pit - 1].layout();
		if (layout2->latextype != LATEX_PARAGRAPH) {
			--pit;
			layout = layout2;
		}
	}

	string text = layout->latexname().substr(0, 3);
	if (layout->latexname() == "theorem")
		text = "thm"; // Create a correct prefix for prettyref

	text += ':';
	if (layout->latextype == LATEX_PARAGRAPH || lyxrc.label_init_length < 0)
		text.erase();

	string par_text = pars_[pit].asString(cur.buffer(), false);
	for (int i = 0; i < lyxrc.label_init_length; ++i) {
		if (par_text.empty())
			break;
		string head;
		par_text = split(par_text, head, ' ');
		// Is it legal to use spaces in labels ?
		if (i > 0)
			text += '-';
		text += head;
	}

	return text;
}


// Manhattan distance to nearest corner
int LyXText::dist(int x, int y) const
{
	int xx = 0;
	int yy = 0;

	if (x < xo_)
		xx = xo_ - x;
	else if (x > xo_ + int(width_))
		xx = x - xo_ - width_;

	if (y < yo_ - ascent())
		yy = yo_ - ascent() - y;
	else if (y > yo_ + descent())
		yy = y - yo_ - descent();

	lyxerr << " xo_=" << xo_ << "  yo_=" << yo_
	       << " width_=" << width_ << " ascent=" << ascent()
	       << " descent=" << descent()
	       << " dist=" << xx+yy <<endl;
	return xx + yy;
}
