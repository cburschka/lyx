/**
 * \file Paragraph.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author Angus Leeming
 * \author John Levon
 * \author André Pönitz
 * \author Dekel Tsur
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Paragraph.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "Changes.h"
#include "Counters.h"
#include "Encoding.h"
#include "debug.h"
#include "gettext.h"
#include "InsetList.h"
#include "Language.h"
#include "LaTeXFeatures.h"
#include "Color.h"
#include "Layout.h"
#include "Length.h"
#include "Font.h"
#include "FontList.h"
#include "LyXRC.h"
#include "Messages.h"
#include "OutputParams.h"
#include "output_latex.h"
#include "paragraph_funcs.h"
#include "ParagraphParameters.h"
#include "sgml.h"
#include "TexRow.h"
#include "VSpace.h"

#include "frontends/alert.h"
#include "frontends/FontMetrics.h"

#include "insets/InsetBibitem.h"
#include "insets/InsetOptArg.h"

#include "support/lstrings.h"
#include "support/textutils.h"
#include "support/convert.h"
#include "support/unicode.h"

#include <sstream>

using std::endl;
using std::string;
using std::ostream;

namespace lyx {

using support::contains;
using support::prefixIs;
using support::suffixIs;
using support::rsplit;
using support::rtrim;


/////////////////////////////////////////////////////////////////////
//
// Paragraph::Private
//
/////////////////////////////////////////////////////////////////////

class Paragraph::Private
{
public:
	///
	Private(Paragraph * owner);
	/// "Copy constructor"
	Private(Private const &, Paragraph * owner);

	///
	value_type getChar(pos_type pos) const;
	///
	void insertChar(pos_type pos, value_type c, Change const & change);

	/// Output the surrogate pair formed by \p c and \p next to \p os.
	/// \return the number of characters written.
	int latexSurrogatePair(odocstream & os, value_type c, value_type next,
			       Encoding const &);

	/// Output a space in appropriate formatting (or a surrogate pair
	/// if the next character is a combining character).
	/// \return whether a surrogate pair was output.
	bool simpleTeXBlanks(Encoding const &,
			     odocstream &, TexRow & texrow,
			     pos_type & i,
			     unsigned int & column,
			     Font const & font,
			     Layout const & style);

	/// Output consecutive known unicode chars, belonging to the same
	/// language as specified by \p preamble, to \p os starting from \p c.
	/// \return the number of characters written.
	int knownLangChars(odocstream & os, value_type c, string & preamble,
			   Change &, Encoding const &, pos_type &);
	///
	void simpleTeXSpecialChars(Buffer const &, BufferParams const &,
				   odocstream &,
				   TexRow & texrow, OutputParams &,
				   Font & running_font,
				   Font & basefont,
				   Font const & outerfont,
				   bool & open_font,
				   Change & running_change,
				   Layout const & style,
				   pos_type & i,
				   unsigned int & column, value_type const c);

	///
	void validate(LaTeXFeatures & features,
		      Layout const & layout) const;

	///
	pos_type size() const { return owner_->size(); }

	/// match a string against a particular point in the paragraph
	bool isTextAt(std::string const & str, pos_type pos) const;
	
	/// Which Paragraph owns us?
	Paragraph * owner_;

	/// In which Inset?
	Inset * inset_owner_;

	///
	FontList fontlist_;

	///
	unsigned int id_;
	///
	static unsigned int paragraph_id;
	///
	ParagraphParameters params_;

	/// for recording and looking up changes
	Changes changes_;

	///
	InsetList insetlist_;
};




using std::endl;
using std::upper_bound;
using std::lower_bound;
using std::string;


// Initialization of the counter for the paragraph id's,
unsigned int Paragraph::Private::paragraph_id = 0;

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


Paragraph::Private::Private(Paragraph * owner)
	: owner_(owner)
{
	inset_owner_ = 0;
	id_ = paragraph_id++;
}


Paragraph::Private::Private(Private const & p, Paragraph * owner)
	: owner_(owner), inset_owner_(p.inset_owner_), fontlist_(p.fontlist_), 
	  params_(p.params_), changes_(p.changes_), insetlist_(p.insetlist_)
{
	id_ = paragraph_id++;
}


bool Paragraph::isChanged(pos_type start, pos_type end) const
{
	BOOST_ASSERT(start >= 0 && start <= size());
	BOOST_ASSERT(end > start && end <= size() + 1);

	return d->changes_.isChanged(start, end);
}


bool Paragraph::isMergedOnEndOfParDeletion(bool trackChanges) const {
	// keep the logic here in sync with the logic of eraseChars()

	if (!trackChanges) {
		return true;
	}

	Change change = d->changes_.lookup(size());

	return change.type == Change::INSERTED && change.author == 0;
}


void Paragraph::setChange(Change const & change)
{
	// beware of the imaginary end-of-par character!
	d->changes_.set(change, 0, size() + 1);

	/*
	 * Propagate the change recursively - but not in case of DELETED!
	 *
	 * Imagine that your co-author makes changes in an existing inset. He
	 * sends your document to you and you come to the conclusion that the
	 * inset should go completely. If you erase it, LyX must not delete all
	 * text within the inset. Otherwise, the change tracked insertions of
	 * your co-author get lost and there is no way to restore them later.
	 *
	 * Conclusion: An inset's content should remain untouched if you delete it
	 */

	if (change.type != Change::DELETED) {
		for (pos_type pos = 0; pos < size(); ++pos) {
			if (isInset(pos))
				getInset(pos)->setChange(change);
		}
	}
}


void Paragraph::setChange(pos_type pos, Change const & change)
{
	BOOST_ASSERT(pos >= 0 && pos <= size());

	d->changes_.set(change, pos);

	// see comment in setChange(Change const &) above

	if (change.type != Change::DELETED &&
	    pos < size() && isInset(pos)) {
		getInset(pos)->setChange(change);
	}
}


Change const & Paragraph::lookupChange(pos_type pos) const
{
	BOOST_ASSERT(pos >= 0 && pos <= size());

	return d->changes_.lookup(pos);
}


void Paragraph::acceptChanges(BufferParams const & bparams, pos_type start,
		pos_type end)
{
	BOOST_ASSERT(start >= 0 && start <= size());
	BOOST_ASSERT(end > start && end <= size() + 1);

	for (pos_type pos = start; pos < end; ++pos) {
		switch (lookupChange(pos).type) {
			case Change::UNCHANGED:
				// accept changes in nested inset
				if (pos < size() && isInset(pos))
					getInset(pos)->acceptChanges(bparams);

				break;

			case Change::INSERTED:
				d->changes_.set(Change(Change::UNCHANGED), pos);
				// also accept changes in nested inset
				if (pos < size() && isInset(pos)) {
					getInset(pos)->acceptChanges(bparams);
				}
				break;

			case Change::DELETED:
				// Suppress access to non-existent
				// "end-of-paragraph char"
				if (pos < size()) {
					eraseChar(pos, false);
					--end;
					--pos;
				}
				break;
		}

	}
}


void Paragraph::rejectChanges(BufferParams const & bparams,
		pos_type start, pos_type end)
{
	BOOST_ASSERT(start >= 0 && start <= size());
	BOOST_ASSERT(end > start && end <= size() + 1);

	for (pos_type pos = start; pos < end; ++pos) {
		switch (lookupChange(pos).type) {
			case Change::UNCHANGED:
				// reject changes in nested inset
				if (pos < size() && isInset(pos)) {
					getInset(pos)->rejectChanges(bparams);
				}
				break;

			case Change::INSERTED:
				// Suppress access to non-existent
				// "end-of-paragraph char"
				if (pos < size()) {
					eraseChar(pos, false);
					--end;
					--pos;
				}
				break;

			case Change::DELETED:
				d->changes_.set(Change(Change::UNCHANGED), pos);

				// Do NOT reject changes within a deleted inset!
				// There may be insertions of a co-author inside of it!

				break;
		}
	}
}


Paragraph::value_type Paragraph::Private::getChar(pos_type pos) const
{
	BOOST_ASSERT(pos >= 0 && pos <= size());

	return owner_->getChar(pos);
}


void Paragraph::Private::insertChar(pos_type pos, value_type c,
		Change const & change)
{
	BOOST_ASSERT(pos >= 0 && pos <= size());

	// track change
	changes_.insert(change, pos);

	// This is actually very common when parsing buffers (and
	// maybe inserting ascii text)
	if (pos == size()) {
		// when appending characters, no need to update tables
		owner_->text_.push_back(c);
		return;
	}

	owner_->text_.insert(owner_->text_.begin() + pos, c);

	// Update the font table.
	fontlist_.increasePosAfterPos(pos);

	// Update the insets
	insetlist_.increasePosAfterPos(pos);
}


void Paragraph::insertInset(pos_type pos, Inset * inset,
				   Change const & change)
{
	BOOST_ASSERT(inset);
	BOOST_ASSERT(pos >= 0 && pos <= size());

	d->insertChar(pos, META_INSET, change);
	BOOST_ASSERT(text_[pos] == META_INSET);

	// Add a new entry in the insetlist_.
	d->insetlist_.insert(inset, pos);
}


bool Paragraph::eraseChar(pos_type pos, bool trackChanges)
{
	BOOST_ASSERT(pos >= 0 && pos <= size());

	// keep the logic here in sync with the logic of isMergedOnEndOfParDeletion()

	if (trackChanges) {
		Change change = d->changes_.lookup(pos);

		// set the character to DELETED if
		//  a) it was previously unchanged or
		//  b) it was inserted by a co-author

		if (change.type == Change::UNCHANGED ||
		    (change.type == Change::INSERTED && change.author != 0)) {
			setChange(pos, Change(Change::DELETED));
			return false;
		}

		if (change.type == Change::DELETED)
			return false;
	}

	// Don't physically access the imaginary end-of-paragraph character.
	// eraseChar() can only mark it as DELETED. A physical deletion of
	// end-of-par must be handled externally.
	if (pos == size()) {
		return false;
	}

	// track change
	d->changes_.erase(pos);

	// if it is an inset, delete the inset entry
	if (text_[pos] == Paragraph::META_INSET)
		d->insetlist_.erase(pos);

	text_.erase(text_.begin() + pos);

	// Update the fontlist_
	d->fontlist_.erase(pos);

	// Update the insetlist_
	d->insetlist_.decreasePosAfterPos(pos);

	return true;
}


int Paragraph::eraseChars(pos_type start, pos_type end, bool trackChanges)
{
	BOOST_ASSERT(start >= 0 && start <= size());
	BOOST_ASSERT(end >= start && end <= size() + 1);

	pos_type i = start;
	for (pos_type count = end - start; count; --count) {
		if (!eraseChar(i, trackChanges))
			++i;
	}
	return end - i;
}


int Paragraph::Private::latexSurrogatePair(odocstream & os, value_type c,
		value_type next, Encoding const & encoding)
{
	// Writing next here may circumvent a possible font change between
	// c and next. Since next is only output if it forms a surrogate pair
	// with c we can ignore this:
	// A font change inside a surrogate pair does not make sense and is
	// hopefully impossible to input.
	// FIXME: change tracking
	// Is this correct WRT change tracking?
	docstring const latex1 = encoding.latexChar(next);
	docstring const latex2 = encoding.latexChar(c);
	os << latex1 << '{' << latex2 << '}';
	return latex1.length() + latex2.length() + 2;
}


bool Paragraph::Private::simpleTeXBlanks(Encoding const & encoding,
				       odocstream & os, TexRow & texrow,
				       pos_type & i,
				       unsigned int & column,
				       Font const & font,
				       Layout const & style)
{
	if (style.pass_thru)
		return false;

	if (i + 1 < size()) {
		char_type next = getChar(i + 1);
		if (Encodings::isCombiningChar(next)) {
			// This space has an accent, so we must always output it.
			column += latexSurrogatePair(os, ' ', next, encoding) - 1;
			++i;
			return true;
		}
	}

	if (lyxrc.plaintext_linelen > 0
	    && column > lyxrc.plaintext_linelen
	    && i
	    && getChar(i - 1) != ' '
	    && (i + 1 < size())
	    // same in FreeSpacing mode
	    && !owner_->isFreeSpacing()
	    // In typewriter mode, we want to avoid
	    // ! . ? : at the end of a line
	    && !(font.family() == Font::TYPEWRITER_FAMILY
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
	return false;
}


int Paragraph::Private::knownLangChars(odocstream & os,
				     value_type c,
				     string & preamble,
				     Change & runningChange,
				     Encoding const & encoding,
				     pos_type & i)
{
	// When the character is marked by the proper language, we simply
	// get its code point in some encoding, otherwise we get the
	// translation specified in the unicodesymbols file, which is
	// something like "\textLANG{<spec>}". So, we have to retain
	// "\textLANG{<spec>" for the first char but only "<spec>" for
	// all subsequent chars.
	docstring const latex1 = rtrim(encoding.latexChar(c), "}");
	int length = latex1.length();
	os << latex1;
	while (i + 1 < size()) {
		char_type next = getChar(i + 1);
		// Stop here if next character belongs to another
		// language or there is a change tracking status.
		if (!Encodings::isKnownLangChar(next, preamble) ||
		    runningChange != owner_->lookupChange(i + 1))
			break;
		Font prev_font;
		bool found = false;
		FontList::const_iterator cit = fontlist_.begin();
		FontList::const_iterator end = fontlist_.end();
		for (; cit != end; ++cit) {
			if (cit->pos() >= i && !found) {
				prev_font = cit->font();
				found = true;
			}
			if (cit->pos() >= i + 1)
				break;
		}
		// Stop here if there is a font attribute change.
		if (found && cit != end && prev_font != cit->font())
			break;
		docstring const latex = rtrim(encoding.latexChar(next), "}");
		docstring::size_type const j =
					latex.find_first_of(from_ascii("{"));
		if (j == docstring::npos) {
			os << latex;
			length += latex.length();
		} else {
			os << latex.substr(j + 1);
			length += latex.substr(j + 1).length();
		}
		++i;
	}
	// When the proper language is set, we are simply passed a code
	// point, so we should not try to close the \textLANG command.
	if (prefixIs(latex1, from_ascii("\\" + preamble))) {
		os << '}';
		++length;
	}
	return length;
}


bool Paragraph::Private::isTextAt(string const & str, pos_type pos) const
{
	pos_type const len = str.length();

	// is the paragraph large enough?
	if (pos + len > size())
		return false;

	// does the wanted text start at point?
	for (string::size_type i = 0; i < str.length(); ++i) {
		// Caution: direct comparison of characters works only
		// because str is pure ASCII.
		if (str[i] != owner_->text_[pos + i])
			return false;
	}

	return fontlist_.hasChangeInRange(pos, len);
}


void Paragraph::Private::simpleTeXSpecialChars(Buffer const & buf,
					     BufferParams const & bparams,
					     odocstream & os,
					     TexRow & texrow,
					     OutputParams & runparams,
					     Font & running_font,
					     Font & basefont,
					     Font const & outerfont,
					     bool & open_font,
					     Change & running_change,
					     Layout const & style,
					     pos_type & i,
					     unsigned int & column,
					     value_type const c)
{
	if (style.pass_thru) {
		if (c != Paragraph::META_INSET) {
			if (c != '\0')
				// FIXME UNICODE: This can fail if c cannot
				// be encoded in the current encoding.
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
		Inset * inset = owner_->getInset(i);

		// FIXME: remove this check
		if (!inset)
			break;

		// FIXME: move this to InsetNewline::latex
		if (inset->lyxCode() == NEWLINE_CODE) {
			// newlines are handled differently here than
			// the default in simpleTeXSpecialChars().
			if (!style.newline_allowed) {
				os << '\n';
			} else {
				if (open_font) {
					column += running_font.latexWriteEndChanges(
						os, bparams, runparams,
						basefont, basefont);
					open_font = false;
				}

				if (running_font.family() == Font::TYPEWRITER_FAMILY)
					os << '~';

				basefont = owner_->getLayoutFont(bparams, outerfont);
				running_font = basefont;

				if (runparams.moving_arg)
					os << "\\protect ";

				os << "\\\\\n";
			}
			texrow.newline();
			texrow.start(owner_->id(), i + 1);
			column = 0;
			break;
		}

		if (owner_->lookupChange(i).type == Change::DELETED) {
			if( ++runparams.inDeletedInset == 1)
				runparams.changeOfDeletedInset = owner_->lookupChange(i);
		}

		if (inset->canTrackChanges()) {
			column += Changes::latexMarkChange(os, bparams, running_change,
				Change(Change::UNCHANGED));
			running_change = Change(Change::UNCHANGED);
		}

		bool close = false;
		odocstream::pos_type const len = os.tellp();

		if ((inset->lyxCode() == GRAPHICS_CODE
		     || inset->lyxCode() == MATH_CODE
		     || inset->lyxCode() == HYPERLINK_CODE)
		    && running_font.isRightToLeft()) {
		    	if (running_font.language()->lang() == "farsi")
				os << "\\beginL{}";
			else
				os << "\\L{";
			close = true;
		}

// FIXME: Bug: we can have an empty font change here!
// if there has just been a font change, we are going to close it
// right now, which means stupid latex code like \textsf{}. AFAIK,
// this does not harm dvi output. A minor bug, thus (JMarc)
		// Some insets cannot be inside a font change command.
		// However, even such insets *can* be placed in \L or \R
		// or their equivalents (for RTL language switches), so we don't
		// close the language in those cases.
		// ArabTeX, though, cannot handle this special behavior, it seems.
		bool arabtex = basefont.language()->lang() == "arabic_arabtex" ||
					   running_font.language()->lang() == "arabic_arabtex";
		if (open_font && inset->noFontChange()) {
			bool closeLanguage = arabtex ||
				basefont.isRightToLeft() == running_font.isRightToLeft();
			unsigned int count = running_font.latexWriteEndChanges(
					os, bparams, runparams,
						basefont, basefont, closeLanguage);
			column += count;
			// if any font properties were closed, update the running_font, 
			// making sure, however, to leave the language as it was
			if (count > 0) {
				// FIXME: probably a better way to keep track of the old 
				// language, than copying the entire font?
				Font const copy_font(running_font);
				basefont = owner_->getLayoutFont(bparams, outerfont);
				running_font = basefont;
				if (!closeLanguage)
					running_font.setLanguage(copy_font.language());
				// leave font open if language is still open
				open_font = (running_font.language() == basefont.language());
				if (closeLanguage)
					runparams.local_font = &basefont;
			}
		}

		int tmp = inset->latex(buf, os, runparams);

		if (close) {
		    	if (running_font.language()->lang() == "farsi")
				os << "\\endL{}";
			else
				os << '}';
		}

		if (tmp) {
			for (int j = 0; j < tmp; ++j) {
				texrow.newline();
			}
			texrow.start(owner_->id(), i + 1);
			column = 0;
		} else {
			column += os.tellp() - len;
		}

		if (owner_->lookupChange(i).type == Change::DELETED) {
			--runparams.inDeletedInset;
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

		case '|': case '<': case '>':
			// In T1 encoding, these characters exist
			if (lyxrc.fontenc == "T1") {
				os.put(c);
				//... but we should avoid ligatures
				if ((c == '>' || c == '<')
				    && i <= size() - 2
				    && getChar(i + 1) == c) {
					//os << "\\textcompwordmark{}";
					//column += 19;
					// Jean-Marc, have a look at
					// this. I think this works
					// equally well:
					os << "\\,{}";
					// Lgb
					column += 3;
				}
				break;
			}
			// Typewriter font also has them
			if (running_font.family() == Font::TYPEWRITER_FAMILY) {
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
			if (i <= size() - 2 &&
			    getChar(i + 1) == '-' &&
			    running_font.family() == Font::TYPEWRITER_FAMILY) {
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
			// FIXME UNICODE: This can fail if c cannot be encoded
			// in the current encoding.
			if (running_font.family() == Font::TYPEWRITER_FAMILY) {
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
				Encoding const & encoding = *(runparams.encoding);
				if (i + 1 < size()) {
					char_type next = getChar(i + 1);
					if (Encodings::isCombiningChar(next)) {
						column += latexSurrogatePair(os, c, next, encoding) - 1;
						++i;
						break;
					}
				}
				string preamble;
				if (Encodings::isKnownLangChar(c, preamble)) {
					column +=
						knownLangChars(os, c, preamble,
							running_change,
							encoding, i) - 1;
					break;
				}
				docstring const latex = encoding.latexChar(c);
				if (latex.length() > 1 &&
				    latex[latex.length() - 1] != '}') {
					// Prevent eating of a following
					// space or command corruption by
					// following characters
					column += latex.length() + 1;
					os << latex << "{}";
				} else {
					column += latex.length() - 1;
					os << latex;
				}
			}
			break;
		}
	}
}


void Paragraph::Private::validate(LaTeXFeatures & features,
				Layout const & layout) const
{
	BufferParams const & bparams = features.bufferParams();

	// check the params.
	if (!params_.spacing().isDefault())
		features.require("setspace");

	// then the layouts
	features.useLayout(layout.name());

	// then the fonts
	Language const * doc_language = bparams.language;

	FontList::const_iterator fcit = fontlist_.begin();
	FontList::const_iterator fend = fontlist_.end();
	for (; fcit != fend; ++fcit) {
		if (fcit->font().noun() == Font::ON) {
			LYXERR(Debug::LATEX) << "font.noun: "
					     << fcit->font().noun()
					     << endl;
			features.require("noun");
			LYXERR(Debug::LATEX) << "Noun enabled. Font: "
					     << to_utf8(fcit->font().stateText(0))
					     << endl;
		}
		switch (fcit->font().color()) {
		case Color::none:
		case Color::inherit:
		case Color::ignore:
			// probably we should put here all interface colors used for
			// font displaying! For now I just add this ones I know of (Jug)
		case Color::latex:
		case Color::note:
			break;
		default:
			features.require("color");
			LYXERR(Debug::LATEX) << "Color enabled. Font: "
					     << to_utf8(fcit->font().stateText(0))
					     << endl;
		}

		Language const * language = fcit->font().language();
		if (language->babel() != doc_language->babel() &&
		    language != ignore_language &&
		    language != latex_language)
		{
			features.useLanguage(language);
			LYXERR(Debug::LATEX) << "Found language "
					     << language->lang() << endl;
		}
	}

	if (!params_.leftIndent().zero())
		features.require("ParagraphLeftIndent");

	// then the insets
	InsetList::const_iterator icit = insetlist_.begin();
	InsetList::const_iterator iend = insetlist_.end();
	for (; icit != iend; ++icit) {
		if (icit->inset) {
			icit->inset->validate(features);
			if (layout.needprotect &&
			    icit->inset->lyxCode() == FOOT_CODE)
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
		Encodings::validate(getChar(i), features);
	}
}


} // namespace lyx


/////////////////////////////////////////////////////////////////////
//
// Paragraph
//
/////////////////////////////////////////////////////////////////////

namespace lyx {

Paragraph::Paragraph()
	: begin_of_body_(0), d(new Paragraph::Private(this))
{
	itemdepth = 0;
	d->params_.clear();
}


Paragraph::Paragraph(Paragraph const & par)
	: itemdepth(par.itemdepth),
	layout_(par.layout_),
	text_(par.text_), begin_of_body_(par.begin_of_body_),
	d(new Paragraph::Private(*par.d, this))
{
}


Paragraph & Paragraph::operator=(Paragraph const & par)
{
	// needed as we will destroy the private part before copying it
	if (&par != this) {
		itemdepth = par.itemdepth;
		layout_ = par.layout();
		text_ = par.text_;
		begin_of_body_ = par.begin_of_body_;

		delete d;
		d = new Private(*par.d, this);
	}
	return *this;
}


Paragraph::~Paragraph()
{
	delete d;
}


void Paragraph::write(Buffer const & buf, ostream & os,
			  BufferParams const & bparams,
			  depth_type & dth) const
{
	// The beginning or end of a deeper (i.e. nested) area?
	if (dth != params().depth()) {
		if (params().depth() > dth) {
			while (params().depth() > dth) {
				os << "\n\\begin_deeper";
				++dth;
			}
		} else {
			while (params().depth() < dth) {
				os << "\n\\end_deeper";
				--dth;
			}
		}
	}

	// First write the layout
	os << "\n\\begin_layout " << to_utf8(layout()->name()) << '\n';

	params().write(os);

	Font font1(Font::ALL_INHERIT, bparams.language);

	Change running_change = Change(Change::UNCHANGED);

	int column = 0;
	for (pos_type i = 0; i <= size(); ++i) {

		Change change = lookupChange(i);
		Changes::lyxMarkChange(os, column, running_change, change);
		running_change = change;

		if (i == size())
			break;

		// Write font changes
		Font font2 = getFontSettings(bparams, i);
		if (font2 != font1) {
			font2.lyxWriteChanges(font1, os);
			column = 0;
			font1 = font2;
		}

		value_type const c = getChar(i);
		switch (c) {
		case META_INSET:
		{
			Inset const * inset = getInset(i);
			if (inset)
				if (inset->directWrite()) {
					// international char, let it write
					// code directly so it's shorter in
					// the file
					inset->write(buf, os);
				} else {
					if (i)
						os << '\n';
					os << "\\begin_inset ";
					inset->write(buf, os);
					os << "\n\\end_inset\n\n";
					column = 0;
				}
		}
		break;
		case '\\':
			os << "\n\\backslash\n";
			column = 0;
			break;
		case '.':
			if (i + 1 < size() && getChar(i + 1) == ' ') {
				os << ".\n";
				column = 0;
			} else
				os << '.';
			break;
		default:
			if ((column > 70 && c == ' ')
			    || column > 79) {
				os << '\n';
				column = 0;
			}
			// this check is to amend a bug. LyX sometimes
			// inserts '\0' this could cause problems.
			if (c != '\0') {
				std::vector<char> tmp = ucs4_to_utf8(c);
				tmp.push_back('\0');
				os << &tmp[0];
			} else
				lyxerr << "ERROR (Paragraph::writeFile):"
					" NULL char in structure." << endl;
			++column;
			break;
		}
	}

	os << "\n\\end_layout\n";
}


void Paragraph::validate(LaTeXFeatures & features) const
{
	d->validate(features, *layout());
}


void Paragraph::insert(pos_type start, docstring const & str,
		       Font const & font, Change const & change)
{
	for (size_t i = 0, n = str.size(); i != n ; ++i)
		insertChar(start + i, str[i], font, change);
}


void Paragraph::insertChar(pos_type pos, Paragraph::value_type c,
			   bool trackChanges)
{
	d->insertChar(pos, c, Change(trackChanges ?
			   Change::INSERTED : Change::UNCHANGED));
}


void Paragraph::insertChar(pos_type pos, Paragraph::value_type c,
			   Font const & font, bool trackChanges)
{
	d->insertChar(pos, c, Change(trackChanges ?
			   Change::INSERTED : Change::UNCHANGED));
	setFont(pos, font);
}


void Paragraph::insertChar(pos_type pos, Paragraph::value_type c,
			   Font const & font, Change const & change)
{
	d->insertChar(pos, c, change);
	setFont(pos, font);
}


void Paragraph::insertInset(pos_type pos, Inset * inset,
			    Font const & font, Change const & change)
{
	insertInset(pos, inset, change);
	// Set the font/language of the inset...
	setFont(pos, font);
}


bool Paragraph::insetAllowed(InsetCode code)
{
	return !d->inset_owner_ || d->inset_owner_->insetAllowed(code);
}


// Gets uninstantiated font setting at position.
Font const Paragraph::getFontSettings(BufferParams const & bparams,
					 pos_type pos) const
{
	if (pos > size()) {
		lyxerr << " pos: " << pos << " size: " << size() << endl;
		BOOST_ASSERT(pos <= size());
	}

	FontList::const_iterator cit = d->fontlist_.begin();
	FontList::const_iterator end = d->fontlist_.end();
	for (; cit != end; ++cit)
		if (cit->pos() >= pos)
			break;

	if (cit != end)
		return cit->font();

	if (pos == size() && !empty())
		return getFontSettings(bparams, pos - 1);

	return Font(Font::ALL_INHERIT, getParLanguage(bparams));
}


FontSpan Paragraph::fontSpan(pos_type pos) const
{
	BOOST_ASSERT(pos <= size());
	pos_type start = 0;

	FontList::const_iterator cit = d->fontlist_.begin();
	FontList::const_iterator end = d->fontlist_.end();
	for (; cit != end; ++cit) {
		if (cit->pos() >= pos) {
			if (pos >= beginOfBody())
				return FontSpan(std::max(start, beginOfBody()),
						cit->pos());
			else
				return FontSpan(start,
						std::min(beginOfBody() - 1,
							 cit->pos()));
		}
		start = cit->pos() + 1;
	}

	// This should not happen, but if so, we take no chances.
	//lyxerr << "Paragraph::getEndPosOfFontSpan: This should not happen!"
	//      << endl;
	return FontSpan(pos, pos);
}


// Gets uninstantiated font setting at position 0
Font const Paragraph::getFirstFontSettings(BufferParams const & bparams) const
{
	if (!empty() && !d->fontlist_.empty())
		return d->fontlist_.begin()->font();

	return Font(Font::ALL_INHERIT, bparams.language);
}


// Gets the fully instantiated font at a given position in a paragraph
// This is basically the same function as Text::GetFont() in text2.cpp.
// The difference is that this one is used for generating the LaTeX file,
// and thus cosmetic "improvements" are disallowed: This has to deliver
// the true picture of the buffer. (Asger)
Font const Paragraph::getFont(BufferParams const & bparams, pos_type pos,
				 Font const & outerfont) const
{
	BOOST_ASSERT(pos >= 0);

	Font font = getFontSettings(bparams, pos);

	pos_type const body_pos = beginOfBody();
	if (pos < body_pos)
		font.realize(layout_->labelfont);
	else
		font.realize(layout_->font);

	font.realize(outerfont);
	font.realize(bparams.getFont());

	return font;
}


Font const Paragraph::getLabelFont
	(BufferParams const & bparams, Font const & outerfont) const
{
	Font tmpfont = layout()->labelfont;
	tmpfont.setLanguage(getParLanguage(bparams));
	tmpfont.realize(outerfont);
	tmpfont.realize(bparams.getFont());
	return tmpfont;
}


Font const Paragraph::getLayoutFont
	(BufferParams const & bparams, Font const & outerfont) const
{
	Font tmpfont = layout()->font;
	tmpfont.setLanguage(getParLanguage(bparams));
	tmpfont.realize(outerfont);
	tmpfont.realize(bparams.getFont());
	return tmpfont;
}


/// Returns the height of the highest font in range
Font_size Paragraph::highestFontInRange
	(pos_type startpos, pos_type endpos, Font_size def_size) const
{
	return d->fontlist_.highestInRange(startpos, endpos, def_size);
}


Paragraph::value_type
Paragraph::getUChar(BufferParams const & bparams, pos_type pos) const
{
	value_type c = getChar(pos);
	if (!lyxrc.rtl_support)
		return c;

	value_type uc = c;
	switch (c) {
	case '(':
		uc = ')';
		break;
	case ')':
		uc = '(';
		break;
	case '[':
		uc = ']';
		break;
	case ']':
		uc = '[';
		break;
	case '{':
		uc = '}';
		break;
	case '}':
		uc = '{';
		break;
	case '<':
		uc = '>';
		break;
	case '>':
		uc = '<';
		break;
	}
	if (uc != c && getFontSettings(bparams, pos).isRightToLeft())
		return uc;
	else
		return c;
}


void Paragraph::setFont(pos_type pos, Font const & font)
{
	BOOST_ASSERT(pos <= size());

	// First, reduce font against layout/label font
	// Update: The setCharFont() routine in text2.cpp already
	// reduces font, so we don't need to do that here. (Asger)
	
	d->fontlist_.set(pos, font);
}


void Paragraph::makeSameLayout(Paragraph const & par)
{
	layout(par.layout());
	// move to pimpl?
	d->params_ = par.params();
}


bool Paragraph::stripLeadingSpaces(bool trackChanges)
{
	if (isFreeSpacing())
		return false;

	int pos = 0;
	int count = 0;

	while (pos < size() && (isNewline(pos) || isLineSeparator(pos))) {
		if (eraseChar(pos, trackChanges))
			++count;
		else
			++pos;
	}

	return count > 0 || pos > 0;
}


bool Paragraph::hasSameLayout(Paragraph const & par) const
{
	return par.layout() == layout() && d->params_.sameLayout(par.params());
}


depth_type Paragraph::getDepth() const
{
	return params().depth();
}


depth_type Paragraph::getMaxDepthAfter() const
{
	if (layout()->isEnvironment())
		return params().depth() + 1;
	else
		return params().depth();
}


char Paragraph::getAlign() const
{
	if (params().align() == LYX_ALIGN_LAYOUT)
		return layout()->align;
	else
		return params().align();
}


docstring const & Paragraph::getLabelstring() const
{
	return params().labelString();
}


// the next two functions are for the manual labels
docstring const Paragraph::getLabelWidthString() const
{
	if (layout()->margintype == MARGIN_MANUAL)
		return params().labelWidthString();
	else
		return _("Senseless with this layout!");
}


void Paragraph::setLabelWidthString(docstring const & s)
{
	params().labelWidthString(s);
}


docstring const Paragraph::translateIfPossible(docstring const & s,
		BufferParams const & bparams) const
{
	if (!support::isAscii(s) || s.empty()) {
		// This must be a user defined layout. We cannot translate
		// this, since gettext accepts only ascii keys.
		return s;
	}
	// Probably standard layout, try to translate
	Messages & m = getMessages(getParLanguage(bparams)->code());
	return m.get(to_ascii(s));
}


docstring Paragraph::expandLabel(LayoutPtr const & layout,
		BufferParams const & bparams, bool process_appendix) const
{
	TextClass const & tclass = bparams.getTextClass();

	docstring fmt;
	if (process_appendix && params().appendix())
		fmt = translateIfPossible(layout->labelstring_appendix(),
			bparams);
	else
		fmt = translateIfPossible(layout->labelstring(), bparams);

	if (fmt.empty() && layout->labeltype == LABEL_COUNTER 
	    && !layout->counter.empty())
		fmt = "\\the" + layout->counter;

	// handle 'inherited level parts' in 'fmt',
	// i.e. the stuff between '@' in   '@Section@.\arabic{subsection}'
	size_t const i = fmt.find('@', 0);
	if (i != docstring::npos) {
		size_t const j = fmt.find('@', i + 1);
		if (j != docstring::npos) {
			docstring parent(fmt, i + 1, j - i - 1);
			docstring label = from_ascii("??");
			if (tclass.hasLayout(parent))
				docstring label = expandLabel(tclass[parent], bparams,
						      process_appendix);
			fmt = docstring(fmt, 0, i) + label 
				+ docstring(fmt, j + 1, docstring::npos);
		}
	}

	return tclass.counters().counterLabel(fmt);
}


void Paragraph::applyLayout(LayoutPtr const & new_layout)
{
	layout(new_layout);
	LyXAlignment const oldAlign = params().align();
	
	if (!(oldAlign & layout()->alignpossible)) {
		frontend::Alert::warning(_("Alignment not permitted"), 
			_("The new layout does not permit the alignment previously used.\nSetting to default."));
		params().align(LYX_ALIGN_LAYOUT);
	}
}


pos_type Paragraph::beginOfBody() const
{
	return begin_of_body_;
}


void Paragraph::setBeginOfBody()
{
	if (layout()->labeltype != LABEL_MANUAL) {
		begin_of_body_ = 0;
		return;
	}

	// Unroll the first two cycles of the loop
	// and remember the previous character to
	// remove unnecessary getChar() calls
	pos_type i = 0;
	pos_type end = size();
	if (i < end && !isNewline(i)) {
		++i;
		char_type previous_char = 0;
		char_type temp = 0;
		if (i < end) {
			previous_char = text_[i];
			if (!isNewline(i)) {
				++i;
				while (i < end && previous_char != ' ') {
					temp = text_[i];
					if (isNewline(i))
						break;
					++i;
					previous_char = temp;
				}
			}
		}
	}

	begin_of_body_ = i;
}


// returns -1 if inset not found
int Paragraph::getPositionOfInset(Inset const * inset) const
{
	// Find the entry.
	InsetList::const_iterator it = d->insetlist_.begin();
	InsetList::const_iterator end = d->insetlist_.end();
	for (; it != end; ++it)
		if (it->inset == inset)
			return it->pos;
	return -1;
}


InsetBibitem * Paragraph::bibitem() const
{
	if (!d->insetlist_.empty()) {
		Inset * inset = d->insetlist_.begin()->inset;
		if (inset->lyxCode() == BIBITEM_CODE)
			return static_cast<InsetBibitem *>(inset);
	}
	return 0;
}


bool Paragraph::forceDefaultParagraphs() const
{
	return inInset() && inInset()->forceDefaultParagraphs(0);
}


namespace {

// paragraphs inside floats need different alignment tags to avoid
// unwanted space

bool noTrivlistCentering(InsetCode code)
{
	return code == FLOAT_CODE || code == WRAP_CODE;
}


string correction(string const & orig)
{
	if (orig == "flushleft")
		return "raggedright";
	if (orig == "flushright")
		return "raggedleft";
	if (orig == "center")
		return "centering";
	return orig;
}


string const corrected_env(string const & suffix, string const & env,
	InsetCode code)
{
	string output = suffix + "{";
	if (noTrivlistCentering(code))
		output += correction(env);
	else
		output += env;
	output += "}";
	if (suffix == "\\begin")
		output += "\n";
	return output;
}


void adjust_row_column(string const & str, TexRow & texrow, int & column)
{
	if (!contains(str, "\n"))
		column += str.size();
	else {
		string tmp;
		texrow.newline();
		column = rsplit(str, tmp, '\n').size();
	}
}

} // namespace anon


// This could go to ParagraphParameters if we want to
int Paragraph::startTeXParParams(BufferParams const & bparams,
				 odocstream & os, TexRow & texrow,
				 bool moving_arg) const
{
	int column = 0;

	if (params().noindent()) {
		os << "\\noindent ";
		column += 10;
	}
	
	LyXAlignment const curAlign = params().align();

	if (curAlign == layout()->align)
		return column;

	switch (curAlign) {
	case LYX_ALIGN_NONE:
	case LYX_ALIGN_BLOCK:
	case LYX_ALIGN_LAYOUT:
	case LYX_ALIGN_SPECIAL:
		break;
	case LYX_ALIGN_LEFT:
	case LYX_ALIGN_RIGHT:
	case LYX_ALIGN_CENTER:
		if (moving_arg) {
			os << "\\protect";
			column += 8;
		}
		break;
	}

	switch (curAlign) {
	case LYX_ALIGN_NONE:
	case LYX_ALIGN_BLOCK:
	case LYX_ALIGN_LAYOUT:
	case LYX_ALIGN_SPECIAL:
		break;
	case LYX_ALIGN_LEFT: {
		string output;
		if (getParLanguage(bparams)->babel() != "hebrew")
			output = corrected_env("\\begin", "flushleft", ownerCode());
		else
			output = corrected_env("\\begin", "flushright", ownerCode());
		os << from_ascii(output);
		adjust_row_column(output, texrow, column);
		break;
	} case LYX_ALIGN_RIGHT: {
		string output;
		if (getParLanguage(bparams)->babel() != "hebrew")
			output = corrected_env("\\begin", "flushright", ownerCode());
		else
			output = corrected_env("\\begin", "flushleft", ownerCode());
		os << from_ascii(output);
		adjust_row_column(output, texrow, column);
		break;
	} case LYX_ALIGN_CENTER: {
		string output;
		output = corrected_env("\\begin", "center", ownerCode());
		os << from_ascii(output);
		adjust_row_column(output, texrow, column);
		break;
	}
	}

	return column;
}


// This could go to ParagraphParameters if we want to
int Paragraph::endTeXParParams(BufferParams const & bparams,
			       odocstream & os, TexRow & texrow,
			       bool moving_arg) const
{
	int column = 0;

	switch (params().align()) {
	case LYX_ALIGN_NONE:
	case LYX_ALIGN_BLOCK:
	case LYX_ALIGN_LAYOUT:
	case LYX_ALIGN_SPECIAL:
		break;
	case LYX_ALIGN_LEFT:
	case LYX_ALIGN_RIGHT:
	case LYX_ALIGN_CENTER:
		if (moving_arg) {
			os << "\\protect";
			column = 8;
		}
		break;
	}

	switch (params().align()) {
	case LYX_ALIGN_NONE:
	case LYX_ALIGN_BLOCK:
	case LYX_ALIGN_LAYOUT:
	case LYX_ALIGN_SPECIAL:
		break;
	case LYX_ALIGN_LEFT: {
		string output;
		if (getParLanguage(bparams)->babel() != "hebrew")
			output = corrected_env("\n\\par\\end", "flushleft", ownerCode());
		else
			output = corrected_env("\n\\par\\end", "flushright", ownerCode());
		os << from_ascii(output);
		adjust_row_column(output, texrow, column);
		break;
	} case LYX_ALIGN_RIGHT: {
		string output;
		if (getParLanguage(bparams)->babel() != "hebrew")
			output = corrected_env("\n\\par\\end", "flushright", ownerCode());
		else
			output = corrected_env("\n\\par\\end", "flushleft", ownerCode());
		os << from_ascii(output);
		adjust_row_column(output, texrow, column);
		break;
	} case LYX_ALIGN_CENTER: {
		string output;
		output = corrected_env("\n\\par\\end", "center", ownerCode());
		os << from_ascii(output);
		adjust_row_column(output, texrow, column);
		break;
	}
	}

	return column;
}


// This one spits out the text of the paragraph
bool Paragraph::simpleTeXOnePar(Buffer const & buf,
				BufferParams const & bparams,
				Font const & outerfont,
				odocstream & os, TexRow & texrow,
				OutputParams const & runparams) const
{
	LYXERR(Debug::LATEX) << "SimpleTeXOnePar...     " << this << endl;

	bool return_value = false;

	LayoutPtr style;

	// well we have to check if we are in an inset with unlimited
	// length (all in one row) if that is true then we don't allow
	// any special options in the paragraph and also we don't allow
	// any environment other than the default layout of the text class
	// to be valid!
	bool asdefault = forceDefaultParagraphs();

	if (asdefault) {
		style = bparams.getTextClass().defaultLayout();
	} else {
		style = layout();
	}

	// Current base font for all inherited font changes, without any
	// change caused by an individual character, except for the language:
	// It is set to the language of the first character.
	// As long as we are in the label, this font is the base font of the
	// label. Before the first body character it is set to the base font
	// of the body.
	Font basefont;

	// Maybe we have to create a optional argument.
	pos_type body_pos = beginOfBody();
	unsigned int column = 0;

	if (body_pos > 0) {
		// the optional argument is kept in curly brackets in
		// case it contains a ']'
		os << "[{";
		column += 2;
		basefont = getLabelFont(bparams, outerfont);
	} else {
		basefont = getLayoutFont(bparams, outerfont);
	}

	// Which font is currently active?
	Font running_font(basefont);
	// Do we have an open font change?
	bool open_font = false;

	Change runningChange = Change(Change::UNCHANGED);

	texrow.start(id(), 0);

	// if the paragraph is empty, the loop will not be entered at all
	if (empty()) {
		if (style->isCommand()) {
			os << '{';
			++column;
		}
		if (!asdefault)
			column += startTeXParParams(bparams, os, texrow,
						    runparams.moving_arg);
	}

	for (pos_type i = 0; i < size(); ++i) {
		// First char in paragraph or after label?
		if (i == body_pos) {
			if (body_pos > 0) {
				if (open_font) {
					column += running_font.latexWriteEndChanges(
						os, bparams, runparams,
						basefont, basefont);
					open_font = false;
				}
				basefont = getLayoutFont(bparams, outerfont);
				running_font = basefont;

				column += Changes::latexMarkChange(os, bparams,
						runningChange, Change(Change::UNCHANGED));
				runningChange = Change(Change::UNCHANGED);

				os << "}] ";
				column +=3;
			}
			if (style->isCommand()) {
				os << '{';
				++column;
			}

			if (!asdefault)
				column += startTeXParParams(bparams, os,
							    texrow,
							    runparams.moving_arg);
		}

		Change const & change = runparams.inDeletedInset ? runparams.changeOfDeletedInset
		                                                 : lookupChange(i);

		if (bparams.outputChanges && runningChange != change) {
			if (open_font) {
				column += running_font.latexWriteEndChanges(
						os, bparams, runparams, basefont, basefont);
				open_font = false;
			}
			basefont = getLayoutFont(bparams, outerfont);
			running_font = basefont;

			column += Changes::latexMarkChange(os, bparams, runningChange, change);
			runningChange = change;
		}

		// do not output text which is marked deleted
		// if change tracking output is disabled
		if (!bparams.outputChanges && change.type == Change::DELETED) {
			continue;
		}

		++column;

		value_type const c = getChar(i);

		// Fully instantiated font
		Font const font = getFont(bparams, i, outerfont);

		Font const last_font = running_font;

		// Do we need to close the previous font?
		if (open_font &&
		    (font != running_font ||
		     font.language() != running_font.language()))
		{
			column += running_font.latexWriteEndChanges(
					os, bparams, runparams, basefont,
					(i == body_pos-1) ? basefont : font);
			running_font = basefont;
			open_font = false;
		}

		// Switch file encoding if necessary
		if (runparams.encoding->package() == Encoding::inputenc &&
		    font.language()->encoding()->package() == Encoding::inputenc) {
			std::pair<bool, int> const enc_switch = switchEncoding(os, bparams,
					runparams.moving_arg, *(runparams.encoding),
					*(font.language()->encoding()));
			if (enc_switch.first) {
				column += enc_switch.second;
				runparams.encoding = font.language()->encoding();
			}
		}

		// Do we need to change font?
		if ((font != running_font ||
		     font.language() != running_font.language()) &&
			i != body_pos - 1)
		{
			odocstringstream ods;
			column += font.latexWriteStartChanges(ods, bparams,
							      runparams, basefont,
							      last_font);
			running_font = font;
			open_font = true;
			docstring fontchange = ods.str();
			// check if the fontchange ends with a trailing blank
			// (like "\small " (see bug 3382)
			if (suffixIs(fontchange, ' ') && c == ' ')
				os << fontchange.substr(0, fontchange.size() - 1) 
				   << from_ascii("{}");
			else
				os << fontchange;
		}

		if (c == ' ') {
			// Do not print the separation of the optional argument
			// if style->pass_thru is false. This works because
			// simpleTeXSpecialChars ignores spaces if
			// style->pass_thru is false.
			if (i != body_pos - 1) {
				if (d->simpleTeXBlanks(
						*(runparams.encoding), os, texrow,
						i, column, font, *style))
					// A surrogate pair was output. We
					// must not call simpleTeXSpecialChars
					// in this iteration, since
					// simpleTeXBlanks incremented i, and
					// simpleTeXSpecialChars would output
					// the combining character again.
					continue;
			}
		}

		OutputParams rp = runparams;
		rp.free_spacing = style->free_spacing;
		rp.local_font = &font;
		rp.intitle = style->intitle;
		d->simpleTeXSpecialChars(buf, bparams, os,
					texrow, rp, running_font,
					basefont, outerfont, open_font,
					runningChange, *style, i, column, c);

		// Set the encoding to that returned from simpleTeXSpecialChars (see
		// comment for encoding member in OutputParams.h)
		runparams.encoding = rp.encoding;
	}

	// If we have an open font definition, we have to close it
	if (open_font) {
#ifdef FIXED_LANGUAGE_END_DETECTION
		if (next_) {
			running_font
				.latexWriteEndChanges(os, bparams, runparams,
					basefont,
					next_->getFont(bparams, 0, outerfont));
		} else {
			running_font.latexWriteEndChanges(os, bparams,
					runparams, basefont, basefont);
		}
#else
//FIXME: For now we ALWAYS have to close the foreign font settings if they are
//FIXME: there as we start another \selectlanguage with the next paragraph if
//FIXME: we are in need of this. This should be fixed sometime (Jug)
		running_font.latexWriteEndChanges(os, bparams, runparams,
				basefont, basefont);
#endif
	}

	column += Changes::latexMarkChange(os, bparams, runningChange, Change(Change::UNCHANGED));

	// Needed if there is an optional argument but no contents.
	if (body_pos > 0 && body_pos == size()) {
		os << "}]~";
		return_value = false;
	}

	if (!asdefault) {
		column += endTeXParParams(bparams, os, texrow,
					  runparams.moving_arg);
	}

	LYXERR(Debug::LATEX) << "SimpleTeXOnePar...done " << this << endl;
	return return_value;
}


namespace {

enum PAR_TAG {
	PAR_NONE=0,
	TT = 1,
	SF = 2,
	BF = 4,
	IT = 8,
	SL = 16,
	EM = 32
};


string tag_name(PAR_TAG const & pt) {
	switch (pt) {
	case PAR_NONE: return "!-- --";
	case TT: return "tt";
	case SF: return "sf";
	case BF: return "bf";
	case IT: return "it";
	case SL: return "sl";
	case EM: return "em";
	}
	return "";
}


inline
void operator|=(PAR_TAG & p1, PAR_TAG const & p2)
{
	p1 = static_cast<PAR_TAG>(p1 | p2);
}


inline
void reset(PAR_TAG & p1, PAR_TAG const & p2)
{
	p1 = static_cast<PAR_TAG>(p1 & ~p2);
}

} // anon


bool Paragraph::emptyTag() const
{
	for (pos_type i = 0; i < size(); ++i) {
		if (isInset(i)) {
			Inset const * inset = getInset(i);
			InsetCode lyx_code = inset->lyxCode();
			if (lyx_code != TOC_CODE &&
			    lyx_code != INCLUDE_CODE &&
			    lyx_code != GRAPHICS_CODE &&
			    lyx_code != ERT_CODE &&
			    lyx_code != LISTINGS_CODE &&
			    lyx_code != FLOAT_CODE &&
			    lyx_code != TABULAR_CODE) {
				return false;
			}
		} else {
			value_type c = getChar(i);
			if (c != ' ' && c != '\t')
				return false;
		}
	}
	return true;
}


string Paragraph::getID(Buffer const & buf, OutputParams const & runparams) const
{
	for (pos_type i = 0; i < size(); ++i) {
		if (isInset(i)) {
			Inset const * inset = getInset(i);
			InsetCode lyx_code = inset->lyxCode();
			if (lyx_code == LABEL_CODE) {
				string const id = static_cast<InsetCommand const *>(inset)->getContents();
				return "id='" + to_utf8(sgml::cleanID(buf, runparams, from_utf8(id))) + "'";
			}
		}

	}
	return string();
}


pos_type Paragraph::getFirstWord(Buffer const & buf, odocstream & os, OutputParams const & runparams) const
{
	pos_type i;
	for (i = 0; i < size(); ++i) {
		if (isInset(i)) {
			Inset const * inset = getInset(i);
			inset->docbook(buf, os, runparams);
		} else {
			value_type c = getChar(i);
			if (c == ' ')
				break;
			os << sgml::escapeChar(c);
		}
	}
	return i;
}


bool Paragraph::onlyText(Buffer const & buf, Font const & outerfont, pos_type initial) const
{
	Font font_old;

	for (pos_type i = initial; i < size(); ++i) {
		Font font = getFont(buf.params(), i, outerfont);
		if (isInset(i))
			return false;
		if (i != initial && font != font_old)
			return false;
		font_old = font;
	}

	return true;
}


void Paragraph::simpleDocBookOnePar(Buffer const & buf,
				    odocstream & os,
				    OutputParams const & runparams,
				    Font const & outerfont,
				    pos_type initial) const
{
	bool emph_flag = false;

	LayoutPtr const & style = layout();
	Font font_old =
		style->labeltype == LABEL_MANUAL ? style->labelfont : style->font;

	if (style->pass_thru && !onlyText(buf, outerfont, initial))
		os << "]]>";

	// parsing main loop
	for (pos_type i = initial; i < size(); ++i) {
		Font font = getFont(buf.params(), i, outerfont);

		// handle <emphasis> tag
		if (font_old.emph() != font.emph()) {
			if (font.emph() == Font::ON) {
				os << "<emphasis>";
				emph_flag = true;
			} else if (i != initial) {
				os << "</emphasis>";
				emph_flag = false;
			}
		}

		if (isInset(i)) {
			Inset const * inset = getInset(i);
			inset->docbook(buf, os, runparams);
		} else {
			value_type c = getChar(i);

			if (style->pass_thru)
				os.put(c);
			else
				os << sgml::escapeChar(c);
		}
		font_old = font;
	}

	if (emph_flag) {
		os << "</emphasis>";
	}

	if (style->free_spacing)
		os << '\n';
	if (style->pass_thru && !onlyText(buf, outerfont, initial))
		os << "<![CDATA[";
}


bool Paragraph::isHfill(pos_type pos) const
{
	return isInset(pos)
		&& getInset(pos)->lyxCode() == HFILL_CODE;
}


bool Paragraph::isNewline(pos_type pos) const
{
	return isInset(pos)
		&& getInset(pos)->lyxCode() == NEWLINE_CODE;
}


bool Paragraph::isLineSeparator(pos_type pos) const
{
	value_type const c = getChar(pos);
	return isLineSeparatorChar(c)
		|| (c == Paragraph::META_INSET && getInset(pos) &&
		getInset(pos)->isLineSeparator());
}


/// Used by the spellchecker
bool Paragraph::isLetter(pos_type pos) const
{
	if (isInset(pos))
		return getInset(pos)->isLetter();
	else {
		value_type const c = getChar(pos);
		return isLetterChar(c) || isDigit(c);
	}
}


Language const *
Paragraph::getParLanguage(BufferParams const & bparams) const
{
	if (!empty())
		return getFirstFontSettings(bparams).language();
	// FIXME: we should check the prev par as well (Lgb)
	return bparams.language;
}


bool Paragraph::isRTL(BufferParams const & bparams) const
{
	return lyxrc.rtl_support
		&& getParLanguage(bparams)->rightToLeft()
		&& ownerCode() != ERT_CODE
		&& ownerCode() != LISTINGS_CODE;
}


void Paragraph::changeLanguage(BufferParams const & bparams,
			       Language const * from, Language const * to)
{
	// change language including dummy font change at the end
	for (pos_type i = 0; i <= size(); ++i) {
		Font font = getFontSettings(bparams, i);
		if (font.language() == from) {
			font.setLanguage(to);
			setFont(i, font);
		}
	}
}


bool Paragraph::isMultiLingual(BufferParams const & bparams) const
{
	Language const * doc_language =	bparams.language;
	FontList::const_iterator cit = d->fontlist_.begin();
	FontList::const_iterator end = d->fontlist_.end();

	for (; cit != end; ++cit)
		if (cit->font().language() != ignore_language &&
		    cit->font().language() != latex_language &&
		    cit->font().language() != doc_language)
			return true;
	return false;
}


// Convert the paragraph to a string.
// Used for building the table of contents
docstring const Paragraph::asString(Buffer const & buffer, bool label) const
{
	return asString(buffer, 0, size(), label);
}


docstring const Paragraph::asString(Buffer const & buffer,
				 pos_type beg, pos_type end, bool label) const
{

	odocstringstream os;

	if (beg == 0 && label && !params().labelString().empty())
		os << params().labelString() << ' ';

	for (pos_type i = beg; i < end; ++i) {
		value_type const c = getChar(i);
		if (isPrintable(c))
			os.put(c);
		else if (c == META_INSET)
			getInset(i)->textString(buffer, os);
	}

	return os.str();
}


void Paragraph::setInsetOwner(Inset * inset)
{
	d->inset_owner_ = inset;
}


int Paragraph::id() const
{
	return d->id_;
}


LayoutPtr const & Paragraph::layout() const
{
	return layout_;
}


void Paragraph::layout(LayoutPtr const & new_layout)
{
	layout_ = new_layout;
}


Inset * Paragraph::inInset() const
{
	return d->inset_owner_;
}


InsetCode Paragraph::ownerCode() const
{
	return d->inset_owner_ ?
		d->inset_owner_->lyxCode() : NO_CODE;
}


ParagraphParameters & Paragraph::params()
{
	return d->params_;
}


ParagraphParameters const & Paragraph::params() const
{
	return d->params_;
}


bool Paragraph::isFreeSpacing() const
{
	if (layout()->free_spacing)
		return true;

	// for now we just need this, later should we need this in some
	// other way we can always add a function to Inset too.
	return ownerCode() == ERT_CODE || ownerCode() == LISTINGS_CODE;
}


bool Paragraph::allowEmpty() const
{
	if (layout()->keepempty)
		return true;
	return ownerCode() == ERT_CODE || ownerCode() == LISTINGS_CODE;
}


char_type Paragraph::transformChar(char_type c, pos_type pos) const
{
	if (!Encodings::is_arabic(c))
		return c;

	value_type prev_char = ' ';
	value_type next_char = ' ';

	for (pos_type i = pos - 1; i >= 0; --i) {
		value_type const par_char = getChar(i);
		if (!Encodings::isComposeChar_arabic(par_char)) {
			prev_char = par_char;
			break;
		}
	}

	for (pos_type i = pos + 1, end = size(); i < end; ++i) {
		value_type const par_char = getChar(i);
		if (!Encodings::isComposeChar_arabic(par_char)) {
			next_char = par_char;
			break;
		}
	}

	if (Encodings::is_arabic(next_char)) {
		if (Encodings::is_arabic(prev_char) &&
			!Encodings::is_arabic_special(prev_char))
			return Encodings::transformChar(c, Encodings::FORM_MEDIAL);
		else
			return Encodings::transformChar(c, Encodings::FORM_INITIAL);
	} else {
		if (Encodings::is_arabic(prev_char) &&
			!Encodings::is_arabic_special(prev_char))
			return Encodings::transformChar(c, Encodings::FORM_FINAL);
		else
			return Encodings::transformChar(c, Encodings::FORM_ISOLATED);
	}
}


int Paragraph::checkBiblio(bool track_changes)
{
	//FIXME From JS:
	//This is getting more and more a mess. ...We really should clean
	//up this bibitem issue for 1.6. See also bug 2743.

	// Add bibitem insets if necessary
	if (layout()->labeltype != LABEL_BIBLIO)
		return 0;

	bool hasbibitem = !d->insetlist_.empty()
		// Insist on it being in pos 0
		&& getChar(0) == Paragraph::META_INSET
		&& d->insetlist_.begin()->inset->lyxCode() == BIBITEM_CODE;

	docstring oldkey;
	docstring oldlabel;

	// remove a bibitem in pos != 0
	// restore it later in pos 0 if necessary
	// (e.g. if a user inserts contents _before_ the item)
	// we're assuming there's only one of these, which there
	// should be.
	int erasedInsetPosition = -1;
	InsetList::iterator it = d->insetlist_.begin();
	InsetList::iterator end = d->insetlist_.end();
	for (; it != end; ++it)
		if (it->inset->lyxCode() == BIBITEM_CODE
		    && it->pos > 0) {
			InsetBibitem * olditem = static_cast<InsetBibitem *>(it->inset);
			oldkey = olditem->getParam("key");
			oldlabel = olditem->getParam("label");
			erasedInsetPosition = it->pos;
			eraseChar(erasedInsetPosition, track_changes);
			break;
	}

	//There was an InsetBibitem at the beginning, and we didn't
	//have to erase one.
	if (hasbibitem && erasedInsetPosition < 0)
			return 0;

	//There was an InsetBibitem at the beginning and we did have to
	//erase one. So we give its properties to the beginning inset.
	if (hasbibitem) {
		InsetBibitem * inset =
			static_cast<InsetBibitem *>(d->insetlist_.begin()->inset);
		if (!oldkey.empty())
			inset->setParam("key", oldkey);
		inset->setParam("label", oldlabel);
		return -erasedInsetPosition;
	}

	//There was no inset at the beginning, so we need to create one with
	//the key and label of the one we erased.
	InsetBibitem * inset(new InsetBibitem(InsetCommandParams("bibitem")));
	// restore values of previously deleted item in this par.
	if (!oldkey.empty())
		inset->setParam("key", oldkey);
	inset->setParam("label", oldlabel);
	insertInset(0, static_cast<Inset *>(inset),
		    Change(track_changes ? Change::INSERTED : Change::UNCHANGED));

	return 1;
}


void Paragraph::checkAuthors(AuthorList const & authorList)
{
	d->changes_.checkAuthors(authorList);
}


bool Paragraph::isUnchanged(pos_type pos) const
{
	return lookupChange(pos).type == Change::UNCHANGED;
}


bool Paragraph::isInserted(pos_type pos) const
{
	return lookupChange(pos).type == Change::INSERTED;
}


bool Paragraph::isDeleted(pos_type pos) const
{
	return lookupChange(pos).type == Change::DELETED;
}


InsetList const & Paragraph::insetList() const
{
	return d->insetlist_;
}


Inset * Paragraph::releaseInset(pos_type pos)
{
	Inset * inset = d->insetlist_.release(pos);
	/// does not honour change tracking!
	eraseChar(pos, false);
	return inset;
}


Inset * Paragraph::getInset(pos_type pos)
{
	return d->insetlist_.get(pos);
}


Inset const * Paragraph::getInset(pos_type pos) const
{
	return d->insetlist_.get(pos);
}

} // namespace lyx
