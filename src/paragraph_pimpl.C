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

#ifdef __GNUG__
#pragma implementation
#endif

#include "paragraph_pimpl.h"
#include "LaTeXFeatures.h"
#include "texrow.h"
#include "language.h"
#include "bufferparams.h"
#include "encoding.h"
#include "lyxrc.h"
#include "debug.h"

#include "support/LAssert.h"

using lyx::pos_type;
using std::endl;
using std::ostream;
using std::upper_bound;
using std::lower_bound;

// Initialize static member.
ShareContainer<LyXFont> Paragraph::Pimpl::FontTable::container;
// Initialization of the counter for the paragraph id's,
unsigned int Paragraph::Pimpl::paragraph_id = 0;

namespace {

struct special_phrase {
	string phrase;
	string macro;
	bool builtin;
};

special_phrase special_phrases[] = {
	{ "LyX", "\\LyX{}", false },
	{ "TeX", "\\TeX{}", true },
	{ "LaTeX2e", "\\LaTeXe{}", true },
	{ "LaTeX", "\\LaTeX{}", true },
};

size_t const phrases_nr = sizeof(special_phrases)/sizeof(special_phrase);

} // namespace anon


Paragraph::Pimpl::Pimpl(Paragraph * owner)
	: owner_(owner)
{
	inset_owner = 0;
	id_ = paragraph_id++;
}


Paragraph::Pimpl::Pimpl(Pimpl const & p, Paragraph * owner, bool same_ids)
	: params(p.params), owner_(owner)
{
	inset_owner = p.inset_owner;
	text = p.text;
	fontlist = p.fontlist;
	if (same_ids)
		id_ = p.id_;
	else
		id_ = paragraph_id++;
}


void Paragraph::Pimpl::clear()
{
	text.clear();
}


void Paragraph::Pimpl::setContentsFromPar(Paragraph const * par)
{
	lyx::Assert(par);
	text = par->pimpl_->text;
}


Paragraph::value_type Paragraph::Pimpl::getChar(pos_type pos) const
{
	// This is in the critical path for loading!
	pos_type const siz = size();
	lyx::Assert(pos <= siz);
	// This is stronger, and I belive that this is the assertion
	// that we should really use. (Lgb)
	//Assert(pos < size());

	// Then this has no meaning. (Lgb)
	if (!siz || pos == siz)
		return '\0';

	return text[pos];
}


void Paragraph::Pimpl::setChar(pos_type pos, value_type c)
{
	text[pos] = c;
}


void Paragraph::Pimpl::insertChar(pos_type pos, value_type c,
				  LyXFont const & font)
{
	lyx::Assert(pos <= size());

	// This is actually very common when parsing buffers (and
	// maybe inserting ascii text)
	if (pos == size()) {
		// when appending characters, no need to update tables
		text.push_back(c);
		owner_->setFont(pos, font);
		return;
	}

	text.insert(text.begin() + pos, c);

	// Update the font table.
	FontTable search_font(pos, LyXFont());
	for (FontList::iterator it = lower_bound(fontlist.begin(),
						      fontlist.end(),
						      search_font, matchFT());
	     it != fontlist.end(); ++it)
	{
		it->pos(it->pos() + 1);
	}

	// Update the insets
	owner_->insetlist.increasePosAfterPos(pos);

	owner_->setFont(pos, font);
}


void Paragraph::Pimpl::insertInset(pos_type pos,
				   Inset * inset, LyXFont const & font)
{
	lyx::Assert(inset);
	lyx::Assert(pos <= size());

	insertChar(pos, META_INSET, font);
	lyx::Assert(text[pos] == META_INSET);

	// Add a new entry in the insetlist.
	owner_->insetlist.insert(inset, pos);
	inset->parOwner(owner_);

	if (inset_owner)
		inset->setOwner(inset_owner);
}


void Paragraph::Pimpl::erase(pos_type pos)
{
	lyx::Assert(pos < size());
	// if it is an inset, delete the inset entry
	if (text[pos] == Paragraph::META_INSET) {
		owner_->insetlist.erase(pos);
	}

	text.erase(text.begin() + pos);

	// Erase entries in the tables.
	FontTable search_font(pos, LyXFont());

	FontList::iterator it =
		lower_bound(fontlist.begin(),
			    fontlist.end(),
			    search_font, matchFT());
	if (it != fontlist.end() && it->pos() == pos &&
	    (pos == 0 ||
	     (it != fontlist.begin()
	      && boost::prior(it)->pos() == pos - 1))) {
		// If it is a multi-character font
		// entry, we just make it smaller
		// (see update below), otherwise we
		// should delete it.
		unsigned int const i = it - fontlist.begin();
		fontlist.erase(fontlist.begin() + i);
		it = fontlist.begin() + i;
		if (i > 0 && i < fontlist.size() &&
		    fontlist[i - 1].font() == fontlist[i].font()) {
			fontlist.erase(fontlist.begin() + i - 1);
			it = fontlist.begin() + i - 1;
		}
	}

	// Update all other entries.
	FontList::iterator fend = fontlist.end();
	for (; it != fend; ++it)
		it->pos(it->pos() - 1);

	// Update the insetlist.
	owner_->insetlist.decreasePosAfterPos(pos);
}


void Paragraph::Pimpl::simpleTeXBlanks(ostream & os, TexRow & texrow,
				       pos_type const i,
				       int & column, LyXFont const & font,
				       LyXLayout const & style)
{
	if (style.pass_thru) return;
	if (column > lyxrc.ascii_linelen
	    && i
	    && getChar(i - 1) != ' '
	    && (i < size() - 1)
	    // same in FreeSpacing mode
	    && !style.free_spacing
		&& !owner_->isFreeSpacing()
	    // In typewriter mode, we want to avoid
	    // ! . ? : at the end of a line
	    && !(font.family() == LyXFont::TYPEWRITER_FAMILY
		 && (getChar(i - 1) == '.'
		     || getChar(i - 1) == '?'
		     || getChar(i - 1) == ':'
		     || getChar(i - 1) == '!'))) {
		os << '\n';
		texrow.newline();
		texrow.start(owner_, i + 1);
		column = 0;
	} else if (style.free_spacing) {
		os << '~';
	} else {
		os << ' ';
	}
}


bool Paragraph::Pimpl::isTextAt(string const & str, pos_type pos) const
{
	pos_type const len = str.length();

	// is the paragraph large enough?
	if (pos + len > size())
		return false;

	// does the wanted text start at point?
	for (string::size_type i = 0; i < str.length(); ++i) {
		if (str[i] != text[pos + i])
			return false;
	}

	// is there a font change in middle of the word?
	FontList::const_iterator cit = fontlist.begin();
	FontList::const_iterator end = fontlist.end();
	for (; cit != end; ++cit) {
		if (cit->pos() >= pos)
			break;
	}
	if (cit != end && pos + len - 1 > cit->pos())
		return false;

	return true;
}


void Paragraph::Pimpl::simpleTeXSpecialChars(Buffer const * buf,
					     BufferParams const & bparams,
					     ostream & os,
					     TexRow & texrow,
					     bool moving_arg,
					     LyXFont & font,
					     LyXFont & running_font,
					     LyXFont & basefont,
					     bool & open_font,
					     LyXLayout const & style,
					     pos_type & i,
					     int & column,
					     value_type const c)
{
	if (style.pass_thru) {
		if (c != '\0') os << c;
		return;
	}
	// Two major modes:  LaTeX or plain
	// Handle here those cases common to both modes
	// and then split to handle the two modes separately.
	switch (c) {
	case Paragraph::META_INSET: {
		Inset * inset = owner_->getInset(i);
		if (inset) {
			bool close = false;
			int const len = os.tellp();
			//ostream::pos_type const len = os.tellp();
			if ((inset->lyxCode() == Inset::GRAPHICS_CODE
			     || inset->lyxCode() == Inset::MATH_CODE
			     || inset->lyxCode() == Inset::URL_CODE)
			    && running_font.isRightToLeft()) {
				os << "\\L{";
				close = true;
			}

#ifdef WITH_WARNINGS
#warning Bug: we can have an empty font change here!
// if there has just been a font change, we are going to close it
// right now, which means stupid latex code like \textsf{}. AFAIK,
// this does not harm dvi output. A minor bug, thus (JMarc)
#endif
			// some insets cannot be inside a font change command
			if (open_font && inset->noFontChange()) {
				column +=running_font.
					latexWriteEndChanges(os,
							     basefont,
							     basefont);
				open_font = false;
				basefont = owner_->getLayoutFont(bparams);
				running_font = basefont;
			}

			int tmp = inset->latex(buf, os, moving_arg,
					       style.free_spacing);

			if (close)
				os << "}";

			if (tmp) {
				for (int j = 0; j < tmp; ++j) {
					texrow.newline();
				}
				texrow.start(owner_, i + 1);
				column = 0;
			} else {
				column += int(os.tellp()) - len;
			}
		}
	}
	break;

	case Paragraph::META_NEWLINE:
		if (open_font) {
			column += running_font.latexWriteEndChanges(os,
								    basefont,
								    basefont);
			open_font = false;
		}
		basefont = owner_->getLayoutFont(bparams);
		running_font = basefont;
		break;

	case Paragraph::META_HFILL:
		os << "\\hfill{}";
		column += 7;
		break;

	default:
		// And now for the special cases within each mode

		switch (c) {
		case '\\':
			os << "\\textbackslash{}";
			column += 15;
			break;

		case '±': case '²': case '³':
		case '×': case '÷': case '¹':
		case '¬': case 'µ':
			if ((bparams.inputenc == "latin1" ||
			     bparams.inputenc == "latin9") ||
			    (bparams.inputenc == "auto" &&
			     (font.language()->encoding()->LatexName()
			      == "latin1" ||
			      font.language()->encoding()->LatexName()
			      == "latin9"))) {
				os << "\\ensuremath{"
				   << c
				   << '}';
				column += 13;
			} else {
				os << c;
			}
			break;

		case '|': case '<': case '>':
			// In T1 encoding, these characters exist
			if (lyxrc.fontenc == "T1") {
				os << c;
				//... but we should avoid ligatures
				if ((c == '>' || c == '<')
				    && i <= size() - 2
				    && getChar(i + 1) == c) {
					//os << "\\textcompwordmark{}";
					// Jean-Marc, have a look at
					// this. I think this works
					// equally well:
					os << "\\,{}";
					// Lgb
					column += 19;
				}
				break;
			}
			// Typewriter font also has them
			if (font.family() == LyXFont::TYPEWRITER_FAMILY) {
				os << c;
				break;
			}
			// Otherwise, we use what LaTeX
			// provides us.
			switch (c) {
			case '<':
				os << "\\textless{}";
				column += 10;
				break;
			case '>':
				os << "\\textgreater{}";
				column += 13;
				break;
			case '|':
				os << "\\textbar{}";
				column += 9;
				break;
			}
			break;

		case '-': // "--" in Typewriter mode -> "-{}-"
			if (i <= size() - 2
			    && getChar(i + 1) == '-'
			    && font.family() == LyXFont::TYPEWRITER_FAMILY) {
				os << "-{}";
				column += 2;
			} else {
				os << '-';
			}
			break;

		case '\"':
			os << "\\char`\\\"{}";
			column += 9;
			break;

		case '£':
			if (bparams.inputenc == "default") {
				os << "\\pounds{}";
				column += 8;
			} else {
				os << c;
			}
			break;

		case '$': case '&':
		case '%': case '#': case '{':
		case '}': case '_':
			os << '\\' << c;
			column += 1;
			break;

		case '~':
			os << "\\textasciitilde{}";
			column += 16;
			break;

		case '^':
			os << "\\textasciicircum{}";
			column += 17;
			break;

		case '*': case '[': case ']':
			// avoid being mistaken for optional arguments
			os << '{' << c << '}';
			column += 2;
			break;

		case ' ':
			// Blanks are printed before font switching.
			// Sure? I am not! (try nice-latex)
			// I am sure it's correct. LyX might be smarter
			// in the future, but for now, nothing wrong is
			// written. (Asger)
			break;

		default:

			// I assume this is hack treating typewriter as verbatim
			if (font.family() == LyXFont::TYPEWRITER_FAMILY) {
				if (c != '\0') {
					os << c;
				}
				break;
			}

			// LyX, LaTeX etc.

			// FIXME: if we have "LaTeX" with a font
			// change in the middle (before the 'T', then
			// the "TeX" part is still special cased.
			// Really we should only operate this on
			// "words" for some definition of word

			size_t pnr = 0;

			for (; pnr < phrases_nr; ++pnr) {
				if (isTextAt(special_phrases[pnr].phrase, i)) {
					os << special_phrases[pnr].macro;
					i += special_phrases[pnr].phrase.length() - 1;
					column += special_phrases[pnr].macro.length() - 1;
					break;
				}
			}

			if (pnr == phrases_nr && c != '\0') {
				os << c;
			}
			break;
		}
	}
}



Paragraph * Paragraph::Pimpl::TeXDeeper(Buffer const * buf,
					BufferParams const & bparams,
					ostream & os, TexRow & texrow)
{
	lyxerr[Debug::LATEX] << "TeXDeeper...     " << this << endl;
	Paragraph * par = owner_;

	while (par && par->params().depth() == owner_->params().depth()) {
		if (par->layout()->isEnvironment()) {
			par = par->TeXEnvironment(buf, bparams,
						  os, texrow);
		} else {
			par = par->TeXOnePar(buf, bparams,
					     os, texrow, false);
		}
	}
	lyxerr[Debug::LATEX] << "TeXDeeper...done " << par << endl;

	return par;
}


void Paragraph::Pimpl::validate(LaTeXFeatures & features,
				LyXLayout const & layout) const
{
	BufferParams const & bparams = features.bufferParams();

	// check the params.
	if (params.lineTop() || params.lineBottom())
		features.require("lyxline");
	if (!params.spacing().isDefault())
		features.require("setspace");

	// then the layouts
	features.useLayout(layout.name());

	// then the fonts
	Language const * doc_language = bparams.language;

	FontList::const_iterator fcit = fontlist.begin();
	FontList::const_iterator fend = fontlist.end();
	for (; fcit != fend; ++fcit) {
		if (fcit->font().noun() == LyXFont::ON) {
			lyxerr[Debug::LATEX] << "font.noun: "
					     << fcit->font().noun()
					     << endl;
			features.require("noun");
			lyxerr[Debug::LATEX] << "Noun enabled. Font: "
					     << fcit->font().stateText(0)
					     << endl;
		}
		switch (fcit->font().color()) {
		case LColor::none:
		case LColor::inherit:
		case LColor::ignore:
			// probably we should put here all interface colors used for
			// font displaying! For now I just add this ones I know of (Jug)
		case LColor::latex:
		case LColor::note:
			break;
		default:
			features.require("color");
			lyxerr[Debug::LATEX] << "Color enabled. Font: "
					     << fcit->font().stateText(0)
					     << endl;
		}

		Language const * language = fcit->font().language();
		if (language->babel() != doc_language->babel() &&
		    language != ignore_language &&
		    language != latex_language)
		{
			features.useLanguage(language);
			lyxerr[Debug::LATEX] << "Found language "
					     << language->babel() << endl;
		}
	}

	if (!params.leftIndent().zero())
		features.require("ParagraphLeftIndent");

	// then the insets
	InsetList::iterator icit = owner_->insetlist.begin();
	InsetList::iterator iend = owner_->insetlist.end();
	for (; icit != iend; ++icit) {
		if (icit.getInset()) {
			icit.getInset()->validate(features);
			if (layout.needprotect &&
			    icit.getInset()->lyxCode() == Inset::FOOT_CODE)
				features.require("NeedLyXFootnoteCode");
		}
	}

	// then the contents
	for (pos_type i = 0; i < size() ; ++i) {
		for (size_t pnr = 0; pnr < phrases_nr; ++pnr) {
			if (!special_phrases[pnr].builtin
			    && isTextAt(special_phrases[pnr].phrase, i)) {
				features.require(special_phrases[pnr].phrase);
				break;
			}
		}
	}
}


Paragraph * Paragraph::Pimpl::getParFromID(int id) const
{
	InsetList::iterator cit = owner_->insetlist.begin();
	InsetList::iterator lend = owner_->insetlist.end();
	Paragraph * result;
	for (; cit != lend; ++cit) {
		if ((result = cit.getInset()->getParFromID(id)))
			return result;
	}
	return 0;
}


LyXFont const Paragraph::Pimpl::realizeFont(LyXFont const & font,
					    BufferParams const & bparams) const
{
	LyXFont tmpfont(font);

	// check for environment font information
	char par_depth = owner_->getDepth();
	Paragraph const * par = owner_;
	LyXTextClass const & tclass = bparams.getLyXTextClass();

	while (par && par->getDepth() && !tmpfont.resolved()) {
		par = par->outerHook();
		if (par) {
			tmpfont.realize(par->layout()->font);
			par_depth = par->getDepth();
		}
	}

	tmpfont.realize(tclass.defaultfont());
	return tmpfont;
}
