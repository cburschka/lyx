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
#include "paragraph.h"

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

#include <boost/next_prior.hpp>


namespace lyx {

using std::endl;
using std::upper_bound;
using std::lower_bound;
using std::string;


// Initialization of the counter for the paragraph id's,
unsigned int Paragraph::Pimpl::paragraph_id = 0;

namespace {

struct special_phrase {
	string phrase;
	docstring macro;
	bool builtin;
};

special_phrase const special_phrases[] = {
	{ "LyX", from_ascii("\\LyX{}"), false },
	{ "TeX", from_ascii("\\TeX{}"), true },
	{ "LaTeX2e", from_ascii("\\LaTeXe{}"), true },
	{ "LaTeX", from_ascii("\\LaTeX{}"), true },
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
	: params(p.params), changes_(p.changes_), owner_(owner)
{
	inset_owner = p.inset_owner;
	fontlist = p.fontlist;
	id_ = paragraph_id++;
}


void Paragraph::Pimpl::setContentsFromPar(Paragraph const & par)
{
	owner_->text_ = par.text_;
	// FIXME: change tracking (MG)
	changes_ = par.pimpl_->changes_;
}


bool Paragraph::Pimpl::isChanged(pos_type start, pos_type end) const
{
	// FIXME: change tracking (MG)
	return false;

	return changes_.isChanged(start, end);
}


void Paragraph::Pimpl::setChange(Change const & change)
{
	// FIXME: change tracking (MG)

	return;

	changes_.set(change, 0, size());

	if (change.type == Change::UNCHANGED) { // only for UNCHANGED ???
		for (pos_type i = 0; i < size(); ++i) {
			if (owner_->isInset(i)) {
				owner_->getInset(i)->setChange(change);
			}
		}
	}
}


void Paragraph::Pimpl::setChange(pos_type pos, Change const & change)
{
	// FIXME: change tracking (MG)
	return;

	changes_.set(change, pos);
}


Change const Paragraph::Pimpl::lookupChange(pos_type pos) const
{
	// FIXME: change tracking (MG)
	return Change(Change::UNCHANGED);

	return changes_.lookup(pos);
}


void Paragraph::Pimpl::acceptChange(pos_type start, pos_type end)
{
	// FIXME: change tracking (MG)
	return;

	// care for empty pars

	lyxerr[Debug::CHANGES] << "acceptchange" << endl;
	pos_type i = start;

	for (; i < end; ++i) {
		switch (lookupChange(i).type) {
			case Change::UNCHANGED:
				break;

			case Change::INSERTED:
				// FIXME: change tracking (MG)
				changes_.set(Change(Change::UNCHANGED), i);
				break;

			case Change::DELETED:
				// Suppress access to nonexistent
				// "end-of-paragraph char":
				if (i < size()) {
					erase(i);
					--end;
					--i;
				}
				break;
		}
	}

	lyxerr[Debug::CHANGES] << "endacceptchange" << endl;
	// FIXME: change tracking (MG)
	// changes_.reset(Change::UNCHANGED);
}


void Paragraph::Pimpl::rejectChange(pos_type start, pos_type end)
{
	// FIXME: change tracking (MG)
	return;

	// care for empty pars

	pos_type i = start;

	for (; i < end; ++i) {
		switch (lookupChange(i).type) {
			case Change::UNCHANGED:
				break;

			case Change::INSERTED:
				if (i < size()) {
					erase(i);
					--end;
					--i;
				}
				break;

			case Change::DELETED:
				// FIXME: change tracking (MG)
				changes_.set(Change(Change::UNCHANGED), i);
				// No real char at position size():
				if (i < size() && owner_->isInset(i))
					// FIXME: change tracking (MG)
					owner_->getInset(i)->setChange(Change(Change::UNCHANGED));
				break;
		}
	}
	// FIXME: change tracking (MG)
	// changes_.reset(Change::UNCHANGED);
}


Paragraph::value_type Paragraph::Pimpl::getChar(pos_type pos) const
{
	return owner_->getChar(pos);
}


void Paragraph::Pimpl::insertChar(pos_type pos, value_type c, Change const & change)
{
	BOOST_ASSERT(pos <= size());

	// FIXME: change tracking (MG)
	if (false) {
		// changes_.record(change, pos);
	}

	// This is actually very common when parsing buffers (and
	// maybe inserting ascii text)
	if (pos == size()) {
		// when appending characters, no need to update tables
		owner_->text_.push_back(c);
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
}


void Paragraph::Pimpl::insertInset(pos_type pos,
				   InsetBase * inset, Change const & change)
{
	BOOST_ASSERT(inset);
	BOOST_ASSERT(pos <= size());

	insertChar(pos, META_INSET, change);
	BOOST_ASSERT(owner_->text_[pos] == META_INSET);

	// Add a new entry in the insetlist.
	owner_->insetlist.insert(inset, pos);
}


void Paragraph::Pimpl::erase(pos_type pos)
{
	// FIXME: change tracking (MG)
	// do something like changes_.erase(i);
	// in one of the next patches, the two erase functions 
 	// will be merged but I don't want to break too many things at the same time :-)

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

	// Update all other entries
	FontList::iterator fend = fontlist.end();
	for (; it != fend; ++it)
		it->pos(it->pos() - 1);

	// Update the insetlist
	owner_->insetlist.decreasePosAfterPos(pos);
}


bool Paragraph::Pimpl::erase(pos_type pos, bool /*trackChanges*/)
{
	BOOST_ASSERT(pos <= size());

	// FIXME: change tracking (MG)
	if (false) {
		Change::Type changetype(changes_.lookup(pos).type);

		// only allow the actual removal if it was /new/ text
		if (changetype != Change::INSERTED) {
			// changes_.record(Change(Change::DELETED), pos);
			if (pos < size() && owner_->isInset(pos))
				// FIXME: change tracking (MG)
				owner_->getInset(pos)->setChange(Change(Change::DELETED));
			return false;
		}
	}

	// Don't physically access nonexistent end-of-paragraph char
	if (pos < size()) {
		erase(pos);
		return true;
	}

	return false;
}


int Paragraph::Pimpl::erase(pos_type start, pos_type end, bool trackChanges)
{
	pos_type i = start;
	for (pos_type count = end - start; count; --count) {
		if (!erase(i, trackChanges))
			++i;
	}
	return end - i;
}


void Paragraph::Pimpl::simpleTeXBlanks(odocstream & os, TexRow & texrow,
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
					     odocstream & os,
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
				os.put(c);
		} else
			owner_->getInset(i)->plaintext(buf, os, runparams);
		return;
	}

	// Two major modes:  LaTeX or plain
	// Handle here those cases common to both modes
	// and then split to handle the two modes separately.
	switch (c) {
	case Paragraph::META_INSET: {
		InsetBase * inset = owner_->getInset(i);

		// FIXME: remove this check
		if (!inset)
			break;

		// FIXME: move this to InsetNewline::latex
		if (inset->lyxCode() == InsetBase::NEWLINE_CODE) {
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

		// output change tracking marks only if desired,
		// if dvipost is installed,
		// and with dvi/ps (other formats don't work)
		LaTeXFeatures features(buf, bparams, runparams);
		bool const output = bparams.outputChanges
			&& runparams.flavor == OutputParams::LATEX
			&& features.isAvailable("dvipost");

		if (inset->canTrackChanges()) {
			column += Changes::latexMarkChange(os, running_change,
				Change::UNCHANGED, output);
			running_change = Change::UNCHANGED;
		}

		bool close = false;
		odocstream::pos_type const len = os.tellp();

		if ((inset->lyxCode() == InsetBase::GRAPHICS_CODE
		     || inset->lyxCode() == InsetBase::MATH_CODE
		     || inset->lyxCode() == InsetBase::URL_CODE)
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

		// The following characters could be written literally in latin1, but they
		// would be wrongly converted on systems where char is signed, so we give
		// the code points.
		// This also makes us independant from the encoding of this source file.
		case 0xb1:    // ± PLUS-MINUS SIGN
		case 0xb2:    // ² SUPERSCRIPT TWO
		case 0xb3:    // ³ SUPERSCRIPT THREE
		case 0xd7:    // × MULTIPLICATION SIGN
		case 0xf7:    // ÷ DIVISION SIGN
		case 0xb9:    // ¹ SUPERSCRIPT ONE
		case 0xac:    // ¬ NOT SIGN
		case 0xb5:    // µ MICRO SIGN
			if ((bparams.inputenc == "latin1" ||
			     bparams.inputenc == "latin9") ||
			    (bparams.inputenc == "auto" &&
			     (font.language()->encoding()->latexName()
			      == "latin1" ||
			      font.language()->encoding()->latexName()
			      == "latin9"))) {
				os << "\\ensuremath{";
				os.put(c);
				os << '}';
				column += 13;
			} else {
				os.put(c);
			}
			break;

		case '|': case '<': case '>':
			// In T1 encoding, these characters exist
			if (lyxrc.fontenc == "T1") {
				os.put(c);
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
				os.put(c);
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

		case 0xa3:    // £ POUND SIGN
			if (bparams.inputenc == "default") {
				os << "\\pounds{}";
				column += 8;
			} else {
				os.put(c);
			}
			break;

		case '$': case '&':
		case '%': case '#': case '{':
		case '}': case '_':
			os << '\\';
			os.put(c);
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

		case '*': case '[':
			// avoid being mistaken for optional arguments
			os << '{';
			os.put(c);
			os << '}';
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
					os.put(c);
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
				os.put(c);
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
	InsetList::const_iterator icit = owner_->insetlist.begin();
	InsetList::const_iterator iend = owner_->insetlist.end();
	for (; icit != iend; ++icit) {
		if (icit->inset) {
			icit->inset->validate(features);
			if (layout.needprotect &&
			    icit->inset->lyxCode() == InsetBase::FOOT_CODE)
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


} // namespace lyx
