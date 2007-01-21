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
#include "bufferview_funcs.h"
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
#include "frontends/Selection.h"

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

using std::auto_ptr;
using std::advance;
using std::distance;
using std::max;
using std::min;
using std::endl;
using std::string;

namespace lyx {

using support::bformat;
using support::contains;
using support::lowercase;
using support::split;
using support::uppercase;

using cap::cutSelection;
using cap::pasteParagraphList;

using frontend::FontMetrics;

namespace {

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


int LyXText::leftMargin(Buffer const & buffer, int max_width, pit_type pit) const
{
	BOOST_ASSERT(pit >= 0);
	BOOST_ASSERT(pit < int(pars_.size()));
	return leftMargin(buffer, max_width, pit, pars_[pit].size());
}


int LyXText::leftMargin(Buffer const & buffer, int max_width,
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
				l_margin = leftMargin(buffer, max_width, newpar);
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
		int minfill = max_width;
		for ( ; rit != end; ++rit)
			if (rit->fill() < minfill)
				minfill = rit->fill();
		l_margin += theFontMetrics(params.getFont()).signedWidth(layout->leftmargin);
		l_margin += minfill;
#endif
		// also wrong, but much shorter.
		l_margin += max_width / 2;
		break;
	}
	}

	if (!par.params().leftIndent().zero())
		l_margin += par.params().leftIndent().inPixels(max_width);

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


LColor_color LyXText::backgroundColor() const
{
	return LColor_color(LColor::color(background_color_));
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

	while (!pars_[next_par].empty() && pars_[next_par].isNewline(0)) {
		if (!pars_[next_par].eraseChar(0, cur.buffer().params().trackChanges))
			break; // the character couldn't be deleted physically due to change tracking
	}

	ParIterator current_it(cur);
	ParIterator last_it(cur);
	++last_it;
	++last_it;

	updateLabels(cur.buffer(), current_it, last_it);

	// A singlePar update is not enough in this case.
	cur.updateFlags(Update::Force);

	// This check is necessary. Otherwise the new empty paragraph will
	// be deleted automatically. And it is more friendly for the user!
	if (cur.pos() != 0 || isempty)
		setCursor(cur.top(), cur.pit() + 1, 0);
	else
		setCursor(cur.top(), cur.pit(), 0);
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

//		cur.updateFlags(Update::Force);
	setCursor(cur.top(), cur.pit(), cur.pos() + 1);
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
	theSelection().haveSelection(cur.selection());
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

	if (!cur.selection())
		return;

	recordUndoSelection(cur, Undo::ATOMIC);

	DocIterator beg = cur.selectionBegin();
	DocIterator end = cur.selectionEnd();

	// first, accept changes within each individual paragraph (do not consider end-of-par)
 	
	for (pit_type pit = beg.pit(); pit <= end.pit(); ++pit) {
		// ignore empty paragraphs; otherwise, an assertion will fail for acceptChanges(0, 0) 
		if (pars_[pit].size() == 0)
			continue;

		// do not consider first paragraph if the cursor starts at pos size()
		if (pit == beg.pit() && beg.pos() == pars_[pit].size())
			continue;

		// do not consider last paragraph if the cursor ends at pos 0
		if (pit == end.pit() && end.pos() == 0) 
			break; // last iteration anyway

		pos_type left  = (pit == beg.pit() ? beg.pos() : 0);
		pos_type right = (pit == end.pit() ? end.pos() : pars_[pit].size());
		pars_[pit].acceptChanges(left, right);
	}
	
	// next, accept imaginary end-of-par characters
	// iterate from last to first paragraph such that we don't have to care for a changing 'end' 
 
	pos_type endpit = end.pit();	
	for (pit_type pit = endpit; pit >= beg.pit(); --pit) {
		pos_type pos = pars_[pit].size();

		// last paragraph of the selection requires special handling
		if (pit == end.pit()) {
			// skip if the selection ends before the end-of-par
			if (end.pos < pos)
				continue;

			// skip if the selection ends with the end-of-par and this is not the
			// last paragraph of the document
			// note: the user must be able to accept the end-of-par of the last par!
			if (end.pos == pos && pit != pars_.size() - 1)
				continue;
		}


		if (!pars_[pit].isUnchanged(pos)) {
			if (pars_[pit].isInserted(pos)) {
				pars_[pit].setChange(pos, Change(Change::UNCHANGED));
			} else { // isDeleted
				if (pit == pars_.size()) {
					// we cannot remove a par break at the end of the last paragraph
					// instead, we mark it unchanged
					pars_[pit].setChange(pos, Change(Change::UNCHANGED));
				} else {
					mergeParagraph(cur.buffer().params(), pars_, pit);
					endpit--;
				}
			}
		}
	}

	// finally, invoke the DEPM
	// FIXME: the following code will be changed in the near future
	setCursorIntern(cur, endpit, 0);
	for (pit_type pit = endpit - 1; pit >= beg.pit(); --pit) {
		bool dummy;
		LCursor old = cur;
		setCursorIntern(cur, pit, 0);
		deleteEmptyParagraphMechanism(cur, old, dummy);
	}

	finishUndo();
	cur.clearSelection();
	setCursorIntern(cur, beg.pit(), beg.pos());
	cur.updateFlags(Update::Force);
	updateLabels(cur.buffer());
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

		// handle imaginary end-of-par character first
		if (right == pars_[pit].size() && !pars_[pit].isUnchanged(right)) {
			if (pars_[pit].isDeleted(right)) {
				pars_[pit].setChange(right, Change(Change::UNCHANGED));
			} else {
				// if (pit + 1 < et.pit()) {
				//	setCursorIntern(cur, pit + 1, 0);
				//	backspacePos0(cur);
				// }
			}
		}
		
		pars_[pit].rejectChanges(left, right);
	}
	finishUndo();
	cur.clearSelection();
	setCursorIntern(cur, it.pit(), it.pos());
	cur.updateFlags(Update::Force);
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
		if(!par.eraseChar(cur.pos(), cur.buffer().params().trackChanges)) {
			// the character has been logically deleted only => skip it
			cur.forwardPosNoDescend();
		}
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

	if (needsUpdate) {
		// Make sure the cursor is correct. Is this really needed?
		// No, not really... at least not here!
		cur.text()->setCursor(cur.top(), cur.pit(), cur.pos());
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

	// A singlePar update is not enough in this case.
//		cur.updateFlags(Update::Force);
	setCursor(cur.top(), cur.pit(), cur.pos());

	return needsUpdate;
}


bool LyXText::dissolveInset(LCursor & cur) {
	BOOST_ASSERT(this == cur.text());

	if (isMainText(*cur.bv().buffer()) || cur.inset().nargs() != 1)
		return false;

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
		// ERT paragraphs have the Language latex_language.
		// This is invalid outside of ERT, so we need to
		// change it to the buffer language.
		ParagraphList::iterator it = plist.begin();
		ParagraphList::iterator it_end = plist.end();
		for (; it != it_end; it++) {
			it->changeLanguage(b.params(), latex_language,
					b.getLanguage());
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
	TextMetrics const & tm = bv.textMetrics(this);
	ParagraphMetrics const & pm1 = tm.parMetrics(beg.pit());
	ParagraphMetrics const & pm2 = tm.parMetrics(end.pit());

	bool const above = (bv_funcs::status(&bv, beg)
			    == bv_funcs::CUR_ABOVE);
	bool const below = (bv_funcs::status(&bv, end)
			    == bv_funcs::CUR_BELOW);
	int y1,y2,x1,x2;
	if (above) {
		y1 = 0;
		y2 = 0;
		x1 = 0;
		x2 = tm.width();
	} else {
		Row const & row1 = pm1.getRow(beg.pos(), beg.boundary());
		y1 = bv_funcs::getPos(bv, beg, beg.boundary()).y_ - row1.ascent();
		y2 = y1 + row1.height();
		int const startx = cursorX(bv, beg.top(), beg.boundary());
		if (!isRTL(buffer, par1)) {
			x1 = startx;
			x2 = 0 + tm.width();
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
		X2 = tm.width();
	} else {
		Row const & row2 = pm2.getRow(end.pos(), end.boundary());
		Y1 = bv_funcs::getPos(bv, end, end.boundary()).y_ - row2.ascent();
		Y2 = Y1 + row2.height();
		int const endx = cursorX(bv, end.top(), end.boundary());
		if (!isRTL(buffer, par2)) {
			X1 = 0;
			X2 = endx;
		}
		else {
			X1 = endx;
			X2 = 0 + tm.width();
		}
	}

	if (!above && !below && &pm1.getRow(beg.pos(), beg.boundary())
	    == &pm2.getRow(end.pos(), end.boundary()))
	{
		// paint only one rectangle
		int const b( !isRTL(*bv.buffer(), par1) ? x + x1 : x + X1 );
		int const w( !isRTL(*bv.buffer(), par1) ? X2 - x1 : x2 - X1 );
		pi.pain.fillRectangle(b, y1, w, y2 - y1, LColor::selection);
		return;
	}

	if (lyxerr.debugging(Debug::DEBUG)) {
		lyxerr[Debug::DEBUG] << " y1: " << y1 << " y2: " << y2
			   << "X1:" << X1 << " x2: " << X2 << " wid: " << tm.width()
			<< endl;
	}

	// paint upper rectangle
	pi.pain.fillRectangle(x + x1, y1, x2 - x1, y2 - y1,
				      LColor::selection);
	// paint bottom rectangle
	pi.pain.fillRectangle(x + X1, Y1, X2 - X1, Y2 - Y1,
				      LColor::selection);
	// paint center rectangle
	pi.pain.fillRectangle(x, y2, tm.width(),
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

int LyXText::cursorX(BufferView const & bv, CursorSlice const & sl,
		bool boundary) const
{
	TextMetrics const & tm = bv.textMetrics(sl.text());
	pit_type const pit = sl.pit();
	Paragraph const & par = pars_[pit];
	ParagraphMetrics const & pm = tm.parMetrics(pit);
	if (pm.rows().empty())
		return 0;

	pos_type ppos = sl.pos();
	// Correct position in front of big insets
	bool const boundary_correction = ppos != 0 && boundary;
	if (boundary_correction)
		--ppos;

	Row const & row = pm.getRow(sl.pos(), boundary);

	pos_type cursor_vpos = 0;

	Buffer const & buffer = *bv.buffer();
	RowMetrics const m = tm.computeRowMetrics(pit, row);
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


int LyXText::cursorY(BufferView const & bv, CursorSlice const & sl, bool boundary) const
{
	//lyxerr << "LyXText::cursorY: boundary: " << boundary << std::endl;
	ParagraphMetrics const & pm = bv.parMetrics(this, sl.pit());
	if (pm.rows().empty())
		return 0;

	int h = 0;
	h -= bv.parMetrics(this, 0).rows()[0].ascent();
	for (pit_type pit = 0; pit < sl.pit(); ++pit) {
		h += bv.parMetrics(this, pit).height();
	}
	int pos = sl.pos();
	if (pos && boundary)
		--pos;
	size_t const rend = pm.pos2row(pos);
	for (size_t rit = 0; rit != rend; ++rit)
		h += pm.rows()[rit].height();
	h += pm.rows()[rend].ascent();
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
		os << _("[Change Tracking] ");

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

	os << bformat(_("Font: %1$s"), font.stateText(&buf.params()));

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


void LyXText::setCursorFromCoordinates(LCursor & cur, int const x, int const y)
{
	BOOST_ASSERT(this == cur.text());
	pit_type pit = getPitNearY(cur.bv(), y);

	TextMetrics const & tm = cur.bv().textMetrics(this);
	ParagraphMetrics const & pm = tm.parMetrics(pit);

	int yy = cur.bv().coordCache().get(this, pit).y_ - pm.ascent();
	lyxerr[Debug::DEBUG]
		<< BOOST_CURRENT_FUNCTION
		<< ": x: " << x
		<< " y: " << y
		<< " pit: " << pit
		<< " yy: " << yy << endl;

	int r = 0;
	BOOST_ASSERT(pm.rows().size());
	for (; r < int(pm.rows().size()) - 1; ++r) {
		Row const & row = pm.rows()[r];
		if (int(yy + row.height()) > y)
			break;
		yy += row.height();
	}

	Row const & row = pm.rows()[r];

	lyxerr[Debug::DEBUG]
		<< BOOST_CURRENT_FUNCTION
		<< ": row " << r
		<< " from pos: " << row.pos()
		<< endl;

	bool bound = false;
	int xx = x;
	pos_type const pos = row.pos() 
		+ tm.getColumnNearX(pit, row, xx, bound);

	lyxerr[Debug::DEBUG]
		<< BOOST_CURRENT_FUNCTION
		<< ": setting cursor pit: " << pit
		<< " pos: " << pos
		<< endl;

	setCursor(cur, pit, pos, true, bound);
	// remember new position.
	cur.setTargetX();
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
