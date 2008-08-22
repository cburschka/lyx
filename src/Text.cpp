/**
 * \file src/text.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup
 * \author Lars Gullik Bjønnes
 * \author Dov Feldstern
 * \author Jean-Marc Lasgouttes
 * \author John Levon
 * \author André Pönitz
 * \author Stefan Schimanski
 * \author Dekel Tsur
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Text.h"

#include "Author.h"
#include "Buffer.h"
#include "buffer_funcs.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "bufferview_funcs.h"
#include "Cursor.h"
#include "ParIterator.h"
#include "CoordCache.h"
#include "CutAndPaste.h"
#include "debug.h"
#include "DispatchResult.h"
#include "Encoding.h"
#include "ErrorList.h"
#include "FuncRequest.h"
#include "factory.h"
#include "FontIterator.h"
#include "gettext.h"
#include "Language.h"
#include "Color.h"
#include "Length.h"
#include "Lexer.h"
#include "LyXRC.h"
#include "Row.h"
#include "MetricsInfo.h"
#include "Paragraph.h"
#include "paragraph_funcs.h"
#include "ParagraphParameters.h"
#include "rowpainter.h"
#include "Undo.h"
#include "VSpace.h"
#include "WordLangTuple.h"

#include "frontends/FontMetrics.h"
#include "frontends/Painter.h"

#include "insets/InsetText.h"
#include "insets/InsetBibitem.h"
#include "insets/InsetCaption.h"
#include "insets/InsetHFill.h"
#include "insets/InsetLine.h"
#include "insets/InsetNewline.h"
#include "insets/InsetPagebreak.h"
#include "insets/InsetOptArg.h"
#include "insets/InsetSpace.h"
#include "insets/InsetSpecialChar.h"
#include "insets/InsetTabular.h"

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
using support::subst;
using support::uppercase;

using cap::cutSelection;
using cap::pasteParagraphList;

using frontend::FontMetrics;

namespace {

void readParToken(Buffer const & buf, Paragraph & par, Lexer & lex,
	string const & token, Font & font, Change & change, ErrorList & errorList)
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
		docstring layoutname = lex.getDocString();

		font = Font(Font::ALL_INHERIT, bp.language);
		change = Change(Change::UNCHANGED);

		TextClass const & tclass = bp.getTextClass();

		if (layoutname.empty()) {
			layoutname = tclass.defaultLayoutName();
		}

		bool hasLayout = tclass.hasLayout(layoutname);

		if (!hasLayout) {
			errorList.push_back(ErrorItem(_("Unknown layout"),
			bformat(_("Layout '%1$s' does not exist in textclass '%2$s'\nTrying to use the default instead.\n"),
			layoutname, from_utf8(tclass.name())), par.id(), 0, par.size()));
			layoutname = tclass.defaultLayoutName();
		}

		par.layout(bp.getTextClass()[layoutname]);

		// Test whether the layout is obsolete.
		Layout_ptr const & layout = par.layout();
		if (!layout->obsoleted_by().empty())
			par.layout(bp.getTextClass()[layout->obsoleted_by()]);

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
		Inset * inset = readInset(lex, buf);
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
			font.setUnderbar(Font::ON);
		else if (tok == "no")
			font.setUnderbar(Font::OFF);
		else if (tok == "default")
			font.setUnderbar(Font::INHERIT);
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
		if (par.isFreeSpacing() && token == "\\InsetSpace")
				par.insertChar(par.size(), ' ', font, change);
		else {
			auto_ptr<Inset> inset;
			if (token == "\\SpecialChar" )
				inset.reset(new InsetSpecialChar);
			else
				inset.reset(new InsetSpace);
			inset->read(buf, lex);
			par.insertInset(par.size(), inset.release(),
					font, change);
		}
	} else if (token == "\\backslash") {
		par.insertChar(par.size(), '\\', font, change);
	} else if (token == "\\newline") {
		auto_ptr<Inset> inset(new InsetNewline);
		inset->read(buf, lex);
		par.insertInset(par.size(), inset.release(), font, change);
	} else if (token == "\\LyXTable") {
		auto_ptr<Inset> inset(new InsetTabular(buf));
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


void readParagraph(Buffer const & buf, Paragraph & par, Lexer & lex,
	ErrorList & errorList)
{
	lex.nextToken();
	string token = lex.getString();
	Font font;
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

		LYXERR(Debug::PARSER) << "Handling paragraph token: `"
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



double Text::spacing(Buffer const & buffer,
		Paragraph const & par) const
{
	if (par.params().spacing().isDefault())
		return buffer.params().spacing().getValue();
	return par.params().spacing().getValue();
}


int Text::singleWidth(Buffer const & buffer, Paragraph const & par,
		pos_type pos) const
{
	return singleWidth(par, pos, par.getChar(pos),
		getFont(buffer, par, pos));
}


int Text::singleWidth(Paragraph const & par,
			 pos_type pos, char_type c, Font const & font) const
{
	// The most common case is handled first (Asger)
	if (isPrintable(c)) {
		Language const * language = font.language();
		if (language->rightToLeft()) {
			if (language->lang() == "arabic_arabtex" ||
				language->lang() == "arabic_arabi" ||
			    language->lang() == "farsi") {
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


int Text::leftMargin(Buffer const & buffer, int max_width, pit_type pit) const
{
	BOOST_ASSERT(pit >= 0);
	BOOST_ASSERT(pit < int(pars_.size()));
	return leftMargin(buffer, max_width, pit, pars_[pit].size());
}


int Text::leftMargin(Buffer const & buffer, int max_width,
		pit_type const pit, pos_type const pos) const
{
	BOOST_ASSERT(pit >= 0);
	BOOST_ASSERT(pit < int(pars_.size()));
	Paragraph const & par = pars_[pit];
	BOOST_ASSERT(pos >= 0);
	BOOST_ASSERT(pos <= par.size());
	//lyxerr << "Text::leftMargin: pit: " << pit << " pos: " << pos << endl;
	TextClass const & tclass = buffer.params().getTextClass();
	Layout_ptr const & layout = par.layout();

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

	Font const labelfont = getLabelFont(buffer, par);
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


Color_color Text::backgroundColor() const
{
	return Color_color(Color::color(background_color_));
}


void Text::breakParagraph(Cursor & cur, bool inverse_logic)
{
	BOOST_ASSERT(this == cur.text());

	Paragraph & cpar = cur.paragraph();
	pit_type cpit = cur.pit();

	TextClass const & tclass = cur.buffer().params().getTextClass();
	Layout_ptr const & layout = cpar.layout();

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
	bool keep_layout = inverse_logic ? 
		!layout->isEnvironment() 
		: layout->isEnvironment();

	// We need to remember this before we break the paragraph, because
	// that invalidates the layout variable
	bool sensitive = layout->labeltype == LABEL_SENSITIVE;

	// we need to set this before we insert the paragraph.
	bool const isempty = cpar.allowEmpty() && cpar.empty();

	lyx::breakParagraph(cur.buffer().params(), paragraphs(), cpit,
			 cur.pos(), keep_layout);

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

	updateLabels(cur.buffer());

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
void Text::insertChar(Cursor & cur, char_type c)
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

		if (current_font.number() == Font::ON) {
			if (!isDigit(c) && !contains(number_operators, c) &&
			    !(contains(number_seperators, c) &&
			      cur.pos() != 0 &&
			      cur.pos() != cur.lastpos() &&
			      getFont(buffer, par, cur.pos()).number() == Font::ON &&
			      getFont(buffer, par, cur.pos() - 1).number() == Font::ON)
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
				     && getFont(buffer, par, cur.pos() - 2).number() == Font::ON) {
					setCharFont(buffer, pit, cur.pos() - 1, current_font);
				}
			}
		}
	}

	// In Bidi text, we want spaces to be treated in a special way: spaces
	// which are between words in different languages should get the 
	// paragraph's language; otherwise, spaces should keep the language 
	// they were originally typed in. This is only in effect while typing;
	// after the text is already typed in, the user can always go back and
	// explicitly set the language of a space as desired. But 99.9% of the
	// time, what we're doing here is what the user actually meant.
	// 
	// The following cases are the ones in which the language of the space
	// should be changed to match that of the containing paragraph. In the
	// depictions, lowercase is LTR, uppercase is RTL, underscore (_) 
	// represents a space, pipe (|) represents the cursor position (so the
	// character before it is the one just typed in). The different cases
	// are depicted logically (not visually), from left to right:
	// 
	// 1. A_a|
	// 2. a_A|
	//
	// Theoretically, there are other situations that we should, perhaps, deal
	// with (e.g.: a|_A, A|_a). In practice, though, there really isn't any 
	// point (to understand why, just try to create this situation...).

	if ((cur.pos() >= 2) && (par.isLineSeparator(cur.pos() - 1))) {
		// get font in front and behind the space in question. But do NOT 
		// use getFont(cur.pos()) because the character c is not inserted yet
		Font const & pre_space_font  = getFont(buffer, par, cur.pos() - 2);
		Font const & post_space_font = real_current_font;
		bool pre_space_rtl  = pre_space_font.isVisibleRightToLeft();
		bool post_space_rtl = post_space_font.isVisibleRightToLeft();
		
		if (pre_space_rtl != post_space_rtl) {
			// Set the space's language to match the language of the 
			// adjacent character whose direction is the paragraph's
			// direction; don't touch other properties of the font
			Language const * lang = 
				(pre_space_rtl == par.isRightToLeftPar(buffer.params())) ?
				pre_space_font.language() : post_space_font.language();

			Font space_font = getFont(buffer, par, cur.pos() - 1);
			space_font.setLanguage(lang);
			par.setFont(cur.pos() - 1, space_font);
		}
	}
	
	// Next check, if there will be two blanks together or a blank at
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
	checkBufferStructure(cur.buffer(), cur);

//		cur.updateFlags(Update::Force);
	setCursor(cur.top(), cur.pit(), cur.pos() + 1);
	charInserted();
}


void Text::charInserted()
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

bool Text::cursorRightOneWord(Cursor & cur)
{
	BOOST_ASSERT(this == cur.text());

	pos_type const lastpos = cur.lastpos();
	pit_type pit = cur.pit();
	pos_type pos = cur.pos();
	Paragraph const & par = cur.paragraph();

	// Paragraph boundary is a word boundary
	if (pos == lastpos) {
		if (pit != cur.lastpit())
			return setCursor(cur, pit + 1, 0);
		else
			return false;
	}

	if (lyxrc.mac_like_word_movement) {
		// Skip through trailing punctuation and spaces.
		while (pos != lastpos && (par.isChar(pos) || par.isSpace(pos)))
                        ++pos;

		// Skip over either a non-char inset or a full word
		if (pos != lastpos && !par.isLetter(pos))
			++pos;
		else while (pos != lastpos && par.isLetter(pos))
			     ++pos;
	} else {
		BOOST_ASSERT(pos < lastpos); // see above
		if (par.isLetter(pos))
			while (pos != lastpos && par.isLetter(pos))
				++pos;
		else if (par.isChar(pos))
			while (pos != lastpos && par.isChar(pos))
				++pos;
		else if (!par.isSpace(pos)) // non-char inset
			++pos;

		// Skip over white space
		while (pos != lastpos && par.isSpace(pos))
			     ++pos;		
	}

	return setCursor(cur, pit, pos);
}


bool Text::cursorLeftOneWord(Cursor & cur)
{
	BOOST_ASSERT(this == cur.text());

	pit_type pit = cur.pit();
	pos_type pos = cur.pos();
	Paragraph & par = cur.paragraph();

	// Paragraph boundary is a word boundary
	if (pos == 0 && pit != 0)
		return setCursor(cur, pit - 1, getPar(pit - 1).size());

	if (lyxrc.mac_like_word_movement) {
		// Skip through punctuation and spaces.
		while (pos != 0 && (par.isChar(pos - 1) || par.isSpace(pos - 1)))
			--pos;

		// Skip over either a non-char inset or a full word
		if (pos != 0 && !par.isLetter(pos - 1) && !par.isChar(pos - 1))
			--pos;
		else while (pos != 0 && par.isLetter(pos - 1))
			     --pos;
	} else {
		// Skip over white space
		while (pos != 0 && par.isSpace(pos - 1))
			     --pos;

		if (pos != 0 && par.isLetter(pos - 1))
			while (pos != 0 && par.isLetter(pos - 1))
				--pos;
		else if (pos != 0 && par.isChar(pos - 1))
			while (pos != 0 && par.isChar(pos - 1))
				--pos;
		else if (pos != 0 && !par.isSpace(pos - 1)) // non-char inset
			--pos;
	}

	return setCursor(cur, pit, pos);
}


void Text::selectWord(Cursor & cur, word_location loc)
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
bool Text::selectWordWhenUnderCursor(Cursor & cur, word_location loc)
{
	BOOST_ASSERT(this == cur.text());
	if (cur.selection())
		return false;
	selectWord(cur, loc);
	return cur.selection();
}


void Text::acceptOrRejectChanges(Cursor & cur, ChangeOp op)
{
	BOOST_ASSERT(this == cur.text());

	if (!cur.selection())
		return;

	recordUndoSelection(cur, Undo::ATOMIC);

	pit_type begPit = cur.selectionBegin().pit();
	pit_type endPit = cur.selectionEnd().pit();

	pos_type begPos = cur.selectionBegin().pos();
	pos_type endPos = cur.selectionEnd().pos();

	// keep selection info, because endPos becomes invalid after the first loop
	bool endsBeforeEndOfPar = (endPos < pars_[endPit].size());

	// first, accept/reject changes within each individual paragraph (do not consider end-of-par)

	for (pit_type pit = begPit; pit <= endPit; ++pit) {
		pos_type parSize = pars_[pit].size();

		// ignore empty paragraphs; otherwise, an assertion will fail for
		// acceptChanges(bparams, 0, 0) or rejectChanges(bparams, 0, 0)
		if (parSize == 0)
			continue;

		// do not consider first paragraph if the cursor starts at pos size()
		if (pit == begPit && begPos == parSize)
			continue;

		// do not consider last paragraph if the cursor ends at pos 0
		if (pit == endPit && endPos == 0)
			break; // last iteration anyway

		pos_type left  = (pit == begPit ? begPos : 0);
		pos_type right = (pit == endPit ? endPos : parSize);

		if (op == ACCEPT) {
			pars_[pit].acceptChanges(cur.buffer().params(), left, right);
		} else {
			pars_[pit].rejectChanges(cur.buffer().params(), left, right);
		}
	}

	// next, accept/reject imaginary end-of-par characters

	for (pit_type pit = begPit; pit <= endPit; ++pit) {
		pos_type pos = pars_[pit].size();

		// skip if the selection ends before the end-of-par
		if (pit == endPit && endsBeforeEndOfPar)
			break; // last iteration anyway

		// skip if this is not the last paragraph of the document
		// note: the user should be able to accept/reject the par break of the last par!
		if (pit == endPit && pit + 1 != int(pars_.size()))
			break; // last iteration anway

		if (op == ACCEPT) {
			if (pars_[pit].isInserted(pos)) {
				pars_[pit].setChange(pos, Change(Change::UNCHANGED));
			} else if (pars_[pit].isDeleted(pos)) {
				if (pit + 1 == int(pars_.size())) {
					// we cannot remove a par break at the end of the last paragraph;
					// instead, we mark it unchanged
					pars_[pit].setChange(pos, Change(Change::UNCHANGED));
				} else {
					mergeParagraph(cur.buffer().params(), pars_, pit);
					--endPit;
					--pit;
				}
			}
		} else {
			if (pars_[pit].isDeleted(pos)) {
				pars_[pit].setChange(pos, Change(Change::UNCHANGED));
			} else if (pars_[pit].isInserted(pos)) {
				if (pit + 1 == int(pars_.size())) {
					// we mark the par break at the end of the last paragraph unchanged
					pars_[pit].setChange(pos, Change(Change::UNCHANGED));
				} else {
					mergeParagraph(cur.buffer().params(), pars_, pit);
					--endPit;
					--pit;
				}
			}
		}
	}

	// finally, invoke the DEPM

	deleteEmptyParagraphMechanism(begPit, endPit, cur.buffer().params().trackChanges);

	//

	finishUndo();
	cur.clearSelection();
	setCursorIntern(cur, begPit, begPos);
	cur.updateFlags(Update::Force);
	updateLabels(cur.buffer());
}


void Text::acceptChanges(BufferParams const & bparams)
{
	lyx::acceptChanges(pars_, bparams);
	deleteEmptyParagraphMechanism(0, pars_.size() - 1, bparams.trackChanges);
}


void Text::rejectChanges(BufferParams const & bparams)
{
	pit_type pars_size = static_cast<pit_type>(pars_.size());

	// first, reject changes within each individual paragraph
	// (do not consider end-of-par)
	for (pit_type pit = 0; pit < pars_size; ++pit) {
		if (!pars_[pit].empty())   // prevent assertion failure
			pars_[pit].rejectChanges(bparams, 0, pars_[pit].size());
	}

	// next, reject imaginary end-of-par characters
	for (pit_type pit = 0; pit < pars_size; ++pit) {
		pos_type pos = pars_[pit].size();

		if (pars_[pit].isDeleted(pos)) {
			pars_[pit].setChange(pos, Change(Change::UNCHANGED));
		} else if (pars_[pit].isInserted(pos)) {
			if (pit == pars_size - 1) {
				// we mark the par break at the end of the last
				// paragraph unchanged
				pars_[pit].setChange(pos, Change(Change::UNCHANGED));
			} else {
				mergeParagraph(bparams, pars_, pit);
				--pit;
				--pars_size;
			}
		}
	}

	// finally, invoke the DEPM
	deleteEmptyParagraphMechanism(0, pars_size - 1, bparams.trackChanges);
}


// Delete from cursor up to the end of the current or next word.
void Text::deleteWordForward(Cursor & cur)
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
		checkBufferStructure(cur.buffer(), cur);
	}
}


// Delete from cursor to start of current or prior word.
void Text::deleteWordBackward(Cursor & cur)
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
		checkBufferStructure(cur.buffer(), cur);
	}
}


// Kill to end of line.
void Text::deleteLineForward(Cursor & cur)
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
		checkBufferStructure(cur.buffer(), cur);
	}
}


void Text::changeCase(Cursor & cur, Text::TextCase action)
{
	BOOST_ASSERT(this == cur.text());
	CursorSlice from;
	CursorSlice to;

	bool gotsel = false;
	if (cur.selection()) {
		from = cur.selBegin();
		to = cur.selEnd();
		gotsel = true;
	} else {
		from = cur.top();
		getWord(from, to, PARTIAL_WORD);
		cursorRightOneWord(cur);
	}

	recordUndoSelection(cur, Undo::ATOMIC);

	pit_type begPit = from.pit();
	pit_type endPit = to.pit();

	pos_type begPos = from.pos();
	pos_type endPos = to.pos();

	bool const trackChanges = cur.buffer().params().trackChanges;

	pos_type right = 0; // needed after the for loop

	for (pit_type pit = begPit; pit <= endPit; ++pit) {
		pos_type parSize = pars_[pit].size();

		pos_type pos = (pit == begPit ? begPos : 0);
		right = (pit == endPit ? endPos : parSize);

		// process sequences of modified characters; in change
		// tracking mode, this approach results in much better
		// usability than changing case on a char-by-char basis
		docstring changes;

		bool capitalize = true;

		for (; pos < right; ++pos) {
			char_type oldChar = pars_[pit].getChar(pos);
			char_type newChar = oldChar;

			// ignore insets and don't play with deleted text!
			if (oldChar != Paragraph::META_INSET && !pars_[pit].isDeleted(pos)) {
				switch (action) {
				case text_lowercase:
					newChar = lowercase(oldChar);
					break;
				case text_capitalization:
					if (capitalize) {
						newChar = uppercase(oldChar);
						capitalize = false;
					}
					break;
				case text_uppercase:
					newChar = uppercase(oldChar);
					break;
				}
			}

			if (!pars_[pit].isLetter(pos) || pars_[pit].isDeleted(pos)) {
				capitalize = true; // permit capitalization again
			}

			if (oldChar != newChar) {
				changes += newChar;
			}

			if (oldChar == newChar || pos == right - 1) {
				if (oldChar != newChar) {
					pos++; // step behind the changing area
				}
				int erasePos = pos - changes.size();
				for (size_t i = 0; i < changes.size(); i++) {
					pars_[pit].insertChar(pos, changes[i],
						pars_[pit].getFontSettings(cur.buffer().params(),
								erasePos),
						trackChanges);
					if (!pars_[pit].eraseChar(erasePos, trackChanges)) {
						++erasePos;
						++pos; // advance
						++right; // expand selection
					}
				}
				changes.clear();
			}
		}
	}

	// the selection may have changed due to logically-only deleted chars
	if (gotsel) {
		setCursor(cur, begPit, begPos);
		cur.resetAnchor();
		setCursor(cur, endPit, right);
		cur.setSelection();
	} else
		setCursor(cur, endPit, right);

	checkBufferStructure(cur.buffer(), cur);
}


bool Text::handleBibitems(Cursor & cur)
{
	if (cur.paragraph().layout()->labeltype != LABEL_BIBLIO)
		return false;
	// if a bibitem is deleted, merge with previous paragraph
	// if this is a bibliography item as well
	if (cur.pos() == 0) {
		BufferParams const & bufparams = cur.buffer().params();
		Paragraph const & par = cur.paragraph();
		Cursor prevcur = cur;
		if (cur.pit() > 0) {
			--prevcur.pit();
			prevcur.pos() = prevcur.lastpos();
		}
		Paragraph const & prevpar = prevcur.paragraph();
		if (cur.pit() > 0 && par.layout() == prevpar.layout()) {
			recordUndo(cur, Undo::ATOMIC, prevcur.pit());
			mergeParagraph(bufparams, cur.text()->paragraphs(),
				       prevcur.pit());
			updateLabels(cur.buffer());
			setCursorIntern(cur, prevcur.pit(), prevcur.pos());
			cur.updateFlags(Update::Force);
		// if not, reset the paragraph to default
		} else
			cur.paragraph().layout(
				bufparams.getTextClass().defaultLayout());
		return true;
	}
	return false;
}


bool Text::erase(Cursor & cur)
{
	BOOST_ASSERT(this == cur.text());
	bool needsUpdate = false;
	Paragraph & par = cur.paragraph();

	if (cur.pos() != cur.lastpos()) {
		// this is the code for a normal delete, not pasting
		// any paragraphs
		recordUndo(cur, Undo::DELETE);
		bool const was_inset = cur.paragraph().isInset(cur.pos());
		if(!par.eraseChar(cur.pos(), cur.buffer().params().trackChanges))
			// the character has been logically deleted only => skip it
			cur.forwardPosNoDescend();
		if (was_inset)
			updateLabels(cur.buffer());
		else
			checkBufferStructure(cur.buffer(), cur);
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

	needsUpdate |= handleBibitems(cur);

	if (needsUpdate) {
		// Make sure the cursor is correct. Is this really needed?
		// No, not really... at least not here!
		cur.text()->setCursor(cur.top(), cur.pit(), cur.pos());
		checkBufferStructure(cur.buffer(), cur);
	}

	return needsUpdate;
}


bool Text::backspacePos0(Cursor & cur)
{
	BOOST_ASSERT(this == cur.text());
	if (cur.pit() == 0)
		return false;

	bool needsUpdate = false;

	BufferParams const & bufparams = cur.buffer().params();
	TextClass const & tclass = bufparams.getTextClass();
	ParagraphList & plist = cur.text()->paragraphs();
	Paragraph const & par = cur.paragraph();
	Cursor prevcur = cur;
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


bool Text::backspace(Cursor & cur)
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
		bool const was_inset = cur.paragraph().isInset(cur.pos());
		cur.paragraph().eraseChar(cur.pos(), cur.buffer().params().trackChanges);
		if (was_inset)
			updateLabels(cur.buffer());
		else
			checkBufferStructure(cur.buffer(), cur);
	}

	if (cur.pos() == cur.lastpos())
		setCurrentFont(cur);

	needsUpdate |= handleBibitems(cur);

	// A singlePar update is not enough in this case.
//		cur.updateFlags(Update::Force);
	setCursor(cur.top(), cur.pit(), cur.pos());

	return needsUpdate;
}


bool Text::dissolveInset(Cursor & cur) {
	BOOST_ASSERT(this == cur.text());

	if (isMainText(*cur.bv().buffer()) || cur.inset().nargs() != 1)
		return false;

	recordUndoInset(cur);
	cur.mark() = false;
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
void Text::draw(PainterInfo & pi, int x, int y) const
{
	paintTextInset(*this, pi, x, y);
}


// only used for inset right now. should also be used for main text
void Text::drawSelection(PainterInfo & pi, int x, int) const
{
	Cursor & cur = pi.base.bv->cursor();
	if (!cur.selection())
		return;
	if (!ptr_cmp(cur.text(), this))
		return;

	LYXERR(Debug::DEBUG)
		<< BOOST_CURRENT_FUNCTION
		<< "draw selection at " << x
		<< endl;

	DocIterator beg = cur.selectionBegin();
	DocIterator end = cur.selectionEnd();

	BufferView & bv = *pi.base.bv;

	// the selection doesn't touch the visible screen?
	if (bv_funcs::status(&bv, beg) == bv_funcs::CUR_BELOW
	    || bv_funcs::status(&bv, end) == bv_funcs::CUR_ABOVE)
		return;

	TextMetrics const & tm = bv.textMetrics(this);
	ParagraphMetrics const & pm1 = tm.parMetrics(beg.pit());
	ParagraphMetrics const & pm2 = tm.parMetrics(end.pit());
	Row const & row1 = pm1.getRow(beg.pos(), beg.boundary());
	Row const & row2 = pm2.getRow(end.pos(), end.boundary());

	// clip above
	int middleTop;
	bool const clipAbove = 
		(bv_funcs::status(&bv, beg) == bv_funcs::CUR_ABOVE);
	if (clipAbove)
		middleTop = 0;
	else
		middleTop = bv_funcs::getPos(bv, beg, beg.boundary()).y_ + row1.descent();
	
	// clip below
	int middleBottom;
	bool const clipBelow = 
		(bv_funcs::status(&bv, end) == bv_funcs::CUR_BELOW);
	if (clipBelow)
		middleBottom = bv.workHeight();
	else
		middleBottom = bv_funcs::getPos(bv, end, end.boundary()).y_ - row2.ascent();

	// start and end in the same line?
	if (!(clipAbove || clipBelow) && &row1 == &row2)
		// then only draw this row's selection
		drawRowSelection(pi, x, row1, beg, end, false, false);
	else {
		if (!clipAbove) {
			// get row end
			DocIterator begRowEnd = beg;
			begRowEnd.pos() = row1.endpos();
			begRowEnd.boundary(true);
			
			// draw upper rectangle
			drawRowSelection(pi, x, row1, beg, begRowEnd, false, true);
		}
			
		if (middleTop < middleBottom) {
			// draw middle rectangle
			pi.pain.fillRectangle(x, middleTop, 
														tm.width(), middleBottom - middleTop, 
														Color::selection);
		}

		if (!clipBelow) {
			// get row begin
			DocIterator endRowBeg = end;
			endRowBeg.pos() = row2.pos();
			endRowBeg.boundary(false);
			
			// draw low rectangle
			drawRowSelection(pi, x, row2, endRowBeg, end, true, false);
		}
	}
}


void Text::drawRowSelection(PainterInfo & pi, int x, Row const & row,
														DocIterator const & beg, DocIterator const & end, 
														bool drawOnBegMargin, bool drawOnEndMargin) const
{
	BufferView & bv = *pi.base.bv;
	Buffer & buffer = *bv.buffer();
	TextMetrics const & tm = bv.textMetrics(this);
	DocIterator cur = beg;
	int x1 = cursorX(bv, beg.top(), beg.boundary());
	int x2 = cursorX(bv, end.top(), end.boundary());
	int y1 = bv_funcs::getPos(bv, cur, cur.boundary()).y_ - row.ascent();
	int y2 = y1 + row.height();
	
	// draw the margins
	if (drawOnBegMargin) {
		if (isRTL(buffer, beg.paragraph()))
			pi.pain.fillRectangle(x + x1, y1, tm.width() - x1, y2 - y1, Color::selection);
		else
			pi.pain.fillRectangle(x, y1, x1, y2 - y1, Color::selection);
	}
	
	if (drawOnEndMargin) {
		if (isRTL(buffer, beg.paragraph()))
			pi.pain.fillRectangle(x, y1, x2, y2 - y1, Color::selection);
		else
			pi.pain.fillRectangle(x + x2, y1, tm.width() - x2, y2 - y1, Color::selection);
	}
	
	// if we are on a boundary from the beginning, it's probably
	// a RTL boundary and we jump to the other side directly as this
	// segement is 0-size and confuses the logic below
	if (cur.boundary())
		cur.boundary(false);
	
	// go through row and draw from RTL boundary to RTL boundary
	while (cur < end) {
		bool drawNow = false;
		
		// simplified cursorRight code below which does not
		// descend into insets and which does not go into the
		// next line. Compare the logic with the original cursorRight
		
		// if left of boundary -> just jump to right side
		// but for RTL boundaries don't, because: abc|DDEEFFghi -> abcDDEEF|Fghi
		if (cur.boundary()) {
			cur.boundary(false);
		}	else if (isRTLBoundary(buffer, cur.paragraph(), cur.pos() + 1)) {
			// in front of RTL boundary -> Stay on this side of the boundary because:
			//   ab|cDDEEFFghi -> abc|DDEEFFghi
			++cur.pos();
			cur.boundary(true);
			drawNow = true;
		} else {
			// move right
			++cur.pos();
			
			// line end?
			if (cur.pos() == row.endpos())
				cur.boundary(true);
		}
			
		if (x1 == -1) {
			// the previous segment was just drawn, now the next starts
			x1 = cursorX(bv, cur.top(), cur.boundary());
		}
		
		if (!(cur < end) || drawNow) {
			x2 = cursorX(bv, cur.top(), cur.boundary());
			pi.pain.fillRectangle(x + min(x1,x2), y1, abs(x2 - x1), y2 - y1,
														Color::selection);
			
			// reset x1, so it is set again next round (which will be on the 
			// right side of a boundary or at the selection end)
			x1 = -1;
		}
	}
}



bool Text::isLastRow(pit_type pit, Row const & row) const
{
	return row.endpos() >= pars_[pit].size()
		&& pit + 1 == pit_type(paragraphs().size());
}


bool Text::isFirstRow(pit_type pit, Row const & row) const
{
	return row.pos() == 0 && pit == 0;
}


void Text::getWord(CursorSlice & from, CursorSlice & to,
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
		lyxerr << "Text::getWord: NEXT_WORD not implemented yet"
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


void Text::write(Buffer const & buf, std::ostream & os) const
{
	ParagraphList::const_iterator pit = paragraphs().begin();
	ParagraphList::const_iterator end = paragraphs().end();
	depth_type dth = 0;
	for (; pit != end; ++pit)
		pit->write(buf, os, buf.params(), dth);
}


bool Text::read(Buffer const & buf, Lexer & lex, ErrorList & errorList)
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
			par.setFont(0, Font(Font::ALL_INHERIT, buf.params().language));
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

int Text::cursorX(BufferView const & bv, CursorSlice const & sl,
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
	Bidi bidi;
	bidi.computeTables(par, buffer, row);

	pos_type const row_pos  = row.pos();
	pos_type const end      = row.endpos();
	// Spaces at logical line breaks in bidi text must be skipped during 
	// cursor positioning. However, they may appear visually in the middle
	// of a row; they must be skipped, wherever they are...
	// * logically "abc_[HEBREW_\nHEBREW]"
	// * visually "abc_[_WERBEH\nWERBEH]"
	pos_type skipped_sep_vpos = -1;

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
	Font font;
	FontMetrics const & labelfm = theFontMetrics(
		getLabelFont(buffer, par));

	// If the last logical character is a separator, skip it, unless
	// it's in the last row of a paragraph; see skipped_sep_vpos declaration
	if (end > 0 && end < par.size() && par.isSeparator(end - 1))
		skipped_sep_vpos = bidi.log2vis(end - 1);
	
	for (pos_type vpos = row_pos; vpos < cursor_vpos; ++vpos) {
		// Skip the separator which is at the logical end of the row
		if (vpos == skipped_sep_vpos)
			continue;
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
	if (boundary_correction) {
		if (isRTL(buffer, sl, boundary))
			x -= singleWidth(buffer, par, ppos);
		else
			x += singleWidth(buffer, par, ppos);
	}

	return int(x);
}


int Text::cursorY(BufferView const & bv, CursorSlice const & sl, bool boundary) const
{
	//lyxerr << "Text::cursorY: boundary: " << boundary << std::endl;
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
docstring Text::currentState(Cursor & cur)
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
	Font font = real_current_font;
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
	// FIXME: Why is the check for par.size() needed?
	// We are called with cur.pos() == par.size() quite often.
	if (!par.empty() && cur.pos() < par.size()) {
		// Force output of code point, not character
		size_t const c = par.getChar(cur.pos());
		os << _(", Char: 0x") << std::hex << c;
	}
	os << _(", Boundary: ") << cur.boundary();
//	Row & row = cur.textRow();
//	os << bformat(_(", Row b:%1$d e:%2$d"), row.pos(), row.endpos());
#endif
	return os.str();
}


docstring Text::getPossibleLabel(Cursor & cur) const
{
	pit_type pit = cur.pit();

	Layout_ptr layout = pars_[pit].layout();

	docstring text;
	docstring par_text = pars_[pit].asString(cur.buffer(), false);
	string piece;
	// the return string of math matrices might contain linebreaks
	par_text = subst(par_text, '\n', '-');
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

	// No need for a prefix if the user said so.
	if (lyxrc.label_init_length <= 0)
		return text;

	// Will contain the label type.
	docstring name;

	// For section, subsection, etc...
	if (layout->latextype == LATEX_PARAGRAPH && pit != 0) {
		Layout_ptr const & layout2 = pars_[pit - 1].layout();
		if (layout2->latextype != LATEX_PARAGRAPH) {
			--pit;
			layout = layout2;
		}
	}
	if (layout->latextype != LATEX_PARAGRAPH)
		name = from_ascii(layout->latexname());

	// for captions, we just take the caption type
	Inset * caption_inset = cur.innerInsetOfType(Inset::CAPTION_CODE);
	if (caption_inset)
		name = from_ascii(static_cast<InsetCaption *>(caption_inset)->type());

	// If none of the above worked, we'll see if we're inside various
	// types of insets and take our abbreviation from them.
	if (name.empty()) {
		Inset::Code const codes[] = {
			Inset::FLOAT_CODE,
			Inset::WRAP_CODE,
			Inset::FOOT_CODE
		};
		for (unsigned int i = 0; i < (sizeof codes / sizeof codes[0]); ++i) {
			Inset * float_inset = cur.innerInsetOfType(codes[i]);
			if (float_inset) {
				name = float_inset->name();
				break;
			}
		}
	}

	// Create a correct prefix for prettyref
	if (name == "theorem")
		name = from_ascii("thm");
	else if (name == "Foot")
		name = from_ascii("fn");
	else if (name == "listing")
		name = from_ascii("lst");

	if (!name.empty())
		text = name.substr(0, 3) + ':' + text;

	return text;
}


void Text::setCursorFromCoordinates(Cursor & cur, int const x, int const y)
{
	BOOST_ASSERT(this == cur.text());
	pit_type pit = getPitNearY(cur.bv(), y);

	TextMetrics const & tm = cur.bv().textMetrics(this);
	ParagraphMetrics const & pm = tm.parMetrics(pit);

	int yy = cur.bv().coordCache().get(this, pit).y_ - pm.ascent();
	LYXERR(Debug::DEBUG)
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

	LYXERR(Debug::DEBUG)
		<< BOOST_CURRENT_FUNCTION
		<< ": row " << r
		<< " from pos: " << row.pos()
		<< endl;

	bool bound = false;
	int xx = x;
	pos_type const pos = row.pos()
		+ tm.getColumnNearX(pit, row, xx, bound);

	LYXERR(Debug::DEBUG)
		<< BOOST_CURRENT_FUNCTION
		<< ": setting cursor pit: " << pit
		<< " pos: " << pos
		<< endl;

	setCursor(cur, pit, pos, true, bound);
	// remember new position.
	cur.setTargetX();
}


void Text::charsTranspose(Cursor & cur)
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
	Font const font1 =
		par.getFontSettings(cur.buffer().params(), pos1);

	char_type char2 = par.getChar(pos2);
	Font const font2 =
		par.getFontSettings(cur.buffer().params(), pos2);

	// And finally, we are ready to perform the transposition.
	// Track the changes if Change Tracking is enabled.
	bool const trackChanges = cur.buffer().params().trackChanges;

	recordUndo(cur);

	par.eraseChar(pos2, trackChanges);
	par.eraseChar(pos1, trackChanges);
	par.insertChar(pos1, char2, font2, trackChanges);
	par.insertChar(pos2, char1, font1, trackChanges);

	checkBufferStructure(cur.buffer(), cur);

	// After the transposition, move cursor to after the transposition.
	setCursor(cur, cur.pit(), pos2);
	cur.forwardPos();
}


} // namespace lyx
