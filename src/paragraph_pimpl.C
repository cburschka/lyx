/**
 * \file paragraph_pimpl.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author John Levon
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "paragraph_pimpl.h"

#include "bufferparams.h"
#include "debug.h"
#include "encoding.h"
#include "language.h"
#include "LaTeXFeatures.h"
#include "LColor.h"
#include "lyxlength.h"
#include "lyxrc.h"
#include "outputparams.h"
#include "texrow.h"


using lyx::pos_type;

using std::endl;
using std::upper_bound;
using std::lower_bound;
using std::string;
using std::ostream;


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


Paragraph::Pimpl::Pimpl(Pimpl const & p, Paragraph * owner)
	: params(p.params), owner_(owner)
{
	inset_owner = p.inset_owner;
	fontlist = p.fontlist;
	id_ = paragraph_id++;

	if (p.tracking())
		changes_.reset(new Changes(*p.changes_.get()));
}


void Paragraph::Pimpl::setContentsFromPar(Paragraph const & par)
{
	owner_->text_ = par.text_;
	if (par.pimpl_->tracking()) {
		changes_.reset(new Changes(*(par.pimpl_->changes_.get())));
	}
}


void Paragraph::Pimpl::trackChanges(Change::Type type)
{
	if (tracking()) {
		lyxerr[Debug::CHANGES] << "already tracking for par " << id_ << endl;
		return;
	}

	lyxerr[Debug::CHANGES] << "track changes for par "
		<< id_ << " type " << type << endl;
	changes_.reset(new Changes(type));
	changes_->set(type, 0, size());
}


void Paragraph::Pimpl::untrackChanges()
{
	changes_.reset(0);
}


void Paragraph::Pimpl::cleanChanges()
{
	// if we're not tracking, we don't want to reset...
	if (!tracking())
		return;

	changes_.reset(new Changes(Change::INSERTED));
	changes_->set(Change::INSERTED, 0, size());
}


bool Paragraph::Pimpl::isChanged(pos_type start, pos_type end) const
{
	if (!tracking())
		return false;

	return changes_->isChange(start, end);
}


bool Paragraph::Pimpl::isChangeEdited(pos_type start, pos_type end) const
{
	if (!tracking())
		return false;

	return changes_->isChangeEdited(start, end);
}


void Paragraph::Pimpl::setChange(pos_type pos, Change::Type type)
{
	if (!tracking())
		return;

	changes_->set(type, pos);
}


Change::Type Paragraph::Pimpl::lookupChange(pos_type pos) const
{
	if (!tracking())
		return Change::UNCHANGED;

	return changes_->lookup(pos);
}


Change const Paragraph::Pimpl::lookupChangeFull(pos_type pos) const
{
	if (!tracking())
		return Change(Change::UNCHANGED);

	return changes_->lookupFull(pos);
}


void Paragraph::Pimpl::markErased()
{
	BOOST_ASSERT(tracking());

	// FIXME: we should actually remove INSERTED chars.
	// difficult because owning insettexts/tabulars need
	// to update themselves when rows etc. change
	changes_->set(Change::DELETED, 0, size());
	changes_->reset(Change::DELETED);
}


void Paragraph::Pimpl::acceptChange(pos_type start, pos_type end)
{
	if (!tracking())
		return;

	if (!size()) {
		changes_.reset(new Changes(Change::UNCHANGED));
		return;
	}

	lyxerr << "acceptchange" << endl;
	pos_type i = start;

	for (; i < end; ++i) {
		switch (lookupChange(i)) {
			case Change::UNCHANGED:
				break;

			case Change::INSERTED:
				changes_->set(Change::UNCHANGED, i);
				break;

			case Change::DELETED:
				eraseIntern(i);
				changes_->erase(i);
				--end;
				--i;
				break;
		}
	}

	lyxerr << "endacceptchange" << endl;
	changes_->reset(Change::UNCHANGED);
}


void Paragraph::Pimpl::rejectChange(pos_type start, pos_type end)
{
	if (!tracking())
		return;

	if (!size()) {
		changes_.reset(new Changes(Change::UNCHANGED));
		return;
	}

	pos_type i = start;

	for (; i < end; ++i) {
		switch (lookupChange(i)) {
			case Change::UNCHANGED:
				break;

			case Change::INSERTED:
				eraseIntern(i);
				changes_->erase(i);
				--end;
				--i;
				break;

			case Change::DELETED:
				changes_->set(Change::UNCHANGED, i);
				break;
		}
	}
	changes_->reset(Change::UNCHANGED);
}


Paragraph::value_type Paragraph::Pimpl::getChar(pos_type pos) const
{
	return owner_->getChar(pos);
}


void Paragraph::Pimpl::insertChar(pos_type pos, value_type c,
				  LyXFont const & font, Change change)
{
	BOOST_ASSERT(pos <= size());

	if (tracking()) {
		changes_->record(change, pos);
	}

	// This is actually very common when parsing buffers (and
	// maybe inserting ascii text)
	if (pos == size()) {
		// when appending characters, no need to update tables
		owner_->text_.push_back(c);
		owner_->setFont(pos, font);
		return;
	}

	owner_->text_.insert(owner_->text_.begin() + pos, c);

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
				   InsetOld * inset, LyXFont const & font, Change change)
{
	BOOST_ASSERT(inset);
	BOOST_ASSERT(pos <= size());

	insertChar(pos, META_INSET, font, change);
	BOOST_ASSERT(owner_->text_[pos] == META_INSET);

	// Add a new entry in the insetlist.
	owner_->insetlist.insert(inset, pos);

	if (inset_owner)
		inset->setOwner(inset_owner);
}


void Paragraph::Pimpl::eraseIntern(pos_type pos)
{
	// if it is an inset, delete the inset entry
	if (owner_->text_[pos] == Paragraph::META_INSET) {
		owner_->insetlist.erase(pos);
	}

	owner_->text_.erase(owner_->text_.begin() + pos);

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


bool Paragraph::Pimpl::erase(pos_type pos)
{
	BOOST_ASSERT(pos < size());

	if (tracking()) {
		Change::Type changetype(changes_->lookup(pos));
		changes_->record(Change(Change::DELETED), pos);

		// only allow the actual removal if it was /new/ text
		if (changetype != Change::INSERTED) {
			if (owner_->text_[pos] == Paragraph::META_INSET) {
				owner_->getInset(pos)->markErased();
			}
			return false;
		}
	}

	eraseIntern(pos);
	return true;
}


int Paragraph::Pimpl::erase(pos_type start, pos_type end)
{
	pos_type i = start;
	for (pos_type count = end - start; count; --count) {
		if (!erase(i))
			++i;
	}
	return end - i;
}


void Paragraph::Pimpl::simpleTeXBlanks(ostream & os, TexRow & texrow,
				       pos_type const i,
				       unsigned int & column,
				       LyXFont const & font,
				       LyXLayout const & style)
{
	if (style.pass_thru)
		return;

	if (column > lyxrc.ascii_linelen
	    && i
	    && getChar(i - 1) != ' '
	    && (i < size() - 1)
	    // same in FreeSpacing mode
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
		texrow.start(owner_->id(), i + 1);
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
		if (str[i] != owner_->text_[pos + i])
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


void Paragraph::Pimpl::simpleTeXSpecialChars(Buffer const & buf,
					     BufferParams const & bparams,
					     ostream & os,
					     TexRow & texrow,
					     OutputParams const & runparams,
					     LyXFont & font,
					     LyXFont & running_font,
					     LyXFont & basefont,
					     LyXFont const & outerfont,
					     bool & open_font,
					     Change::Type & running_change,
					     LyXLayout const & style,
					     pos_type & i,
					     unsigned int & column,
					     value_type const c)
{
	if (style.pass_thru) {
		if (c != Paragraph::META_INSET) {
			if (c != '\0')
				os << c;
		} else {
			InsetOld const * inset = owner_->getInset(i);
			inset->plaintext(buf, os, runparams);
		}
		return;
	}

	// Two major modes:  LaTeX or plain
	// Handle here those cases common to both modes
	// and then split to handle the two modes separately.
	switch (c) {
	case Paragraph::META_INSET: {
		InsetOld * inset = owner_->getInset(i);

		// FIXME: remove this check
		if (!inset)
			break;

		// FIXME: move this to InsetNewline::latex
		if (inset->lyxCode() == InsetOld::NEWLINE_CODE) {
			// newlines are handled differently here than
			// the default in simpleTeXSpecialChars().
			if (!style.newline_allowed) {
				os << '\n';
			} else {
				if (open_font) {
					column += running_font.latexWriteEndChanges(os, basefont, basefont);
					open_font = false;
				}
				basefont = owner_->getLayoutFont(bparams, outerfont);
				running_font = basefont;

				if (font.family() == LyXFont::TYPEWRITER_FAMILY)
					os << '~';

				if (runparams.moving_arg)
					os << "\\protect ";

				os << "\\\\\n";
			}
			texrow.newline();
			texrow.start(owner_->id(), i + 1);
			column = 0;
			break;
		}

		if (inset->isTextInset()) {
			column += Changes::latexMarkChange(os, running_change,
				Change::UNCHANGED);
			running_change = Change::UNCHANGED;
		}

		bool close = false;
		ostream::pos_type const len = os.tellp();

		if ((inset->lyxCode() == InsetOld::GRAPHICS_CODE
		     || inset->lyxCode() == InsetOld::MATH_CODE
		     || inset->lyxCode() == InsetOld::URL_CODE)
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
			basefont = owner_->getLayoutFont(bparams, outerfont);
			running_font = basefont;
		}

		int tmp = inset->latex(buf, os, runparams);

		if (close)
			os << '}';

		if (tmp) {
			for (int j = 0; j < tmp; ++j) {
				texrow.newline();
			}
			texrow.start(owner_->id(), i + 1);
			column = 0;
		} else {
			column += os.tellp() - len;
		}
	}
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


void Paragraph::Pimpl::validate(LaTeXFeatures & features,
				LyXLayout const & layout) const
{
	BufferParams const & bparams = features.bufferParams();

	// check the params.
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
		if (icit->inset) {
			icit->inset->validate(features);
			if (layout.needprotect &&
			    icit->inset->lyxCode() == InsetOld::FOOT_CODE)
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
