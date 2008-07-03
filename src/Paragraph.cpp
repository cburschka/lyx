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
#include "Counters.h"
#include "Encoding.h"
#include "debug.h"
#include "gettext.h"
#include "Language.h"
#include "LaTeXFeatures.h"
#include "Color.h"
#include "Length.h"
#include "Font.h"
#include "LyXRC.h"
#include "Row.h"
#include "Messages.h"
#include "OutputParams.h"
#include "output_latex.h"
#include "paragraph_funcs.h"
#include "ParagraphParameters.h"
#include "rowpainter.h"
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

#include <boost/bind.hpp>
#include <boost/next_prior.hpp>

#include <algorithm>
#include <sstream>

using std::distance;
using std::endl;
using std::string;
using std::ostream;

namespace lyx {

using support::contains;
using support::prefixIs;
using support::subst;
using support::suffixIs;
using support::rsplit;


/////////////////////////////////////////////////////////////////////
//
// Paragraph::Pimpl
//
/////////////////////////////////////////////////////////////////////

class Encoding;
class Layout;


class Paragraph::Pimpl {
public:
	///
	Pimpl(Paragraph * owner);
	/// "Copy constructor"
	Pimpl(Pimpl const &, Paragraph * owner);

	//
	// Change tracking
	//
	/// look up change at given pos
	Change const & lookupChange(pos_type pos) const;
	/// is there a change within the given range ?
	bool isChanged(pos_type start, pos_type end) const;
	/// will the paragraph be physically merged with the next
	/// one if the imaginary end-of-par character is logically deleted?
	bool isMergedOnEndOfParDeletion(bool trackChanges) const;
	/// set change for the entire par
	void setChange(Change const & change);
	/// set change at given pos
	void setChange(pos_type pos, Change const & change);
	/// accept changes within the given range
	void acceptChanges(BufferParams const & bparams, pos_type start, pos_type end);
	/// reject changes within the given range
	void rejectChanges(BufferParams const & bparams, pos_type start, pos_type end);

	///
	value_type getChar(pos_type pos) const;
	///
	void insertChar(pos_type pos, value_type c, Change const & change);
	///
	void insertInset(pos_type pos, Inset * inset, Change const & change);
	/// (logically) erase the char at pos; return true if it was actually erased
	bool eraseChar(pos_type pos, bool trackChanges);
	/// (logically) erase the given range; return the number of chars actually erased
	int eraseChars(pos_type start, pos_type end, bool trackChanges);
	///
	Inset * inset_owner;

	/** A font entry covers a range of positions. Notice that the
	    entries in the list are inserted in random order.
	    I don't think it's worth the effort to implement a more effective
	    datastructure, because the number of different fonts in a paragraph
	    is limited. (Asger)
	    Nevertheless, I decided to store fontlist using a sorted vector:
	    fontlist = { {pos_1,font_1} , {pos_2,font_2} , ... } where
	    pos_1 < pos_2 < ..., font_{i-1} != font_i for all i,
	    and font_i covers the chars in positions pos_{i-1}+1,...,pos_i
	    (font_1 covers the chars 0,...,pos_1) (Dekel)
	*/
	class FontTable  {
	public:
		///
		FontTable(pos_type p, Font const & f)
			: pos_(p), font_(f)
		{}
		///
		pos_type pos() const { return pos_; }
		///
		void pos(pos_type p) { pos_ = p; }
		///
		Font const & font() const { return font_; }
		///
		void font(Font const & f) { font_ = f;}
	private:
		/// End position of paragraph this font attribute covers
		pos_type pos_;
		/** Font. Interpretation of the font values:
		    If a value is Font::INHERIT_*, it means that the font
		    attribute is inherited from either the layout of this
		    paragraph or, in the case of nested paragraphs, from the
		    layout in the environment one level up until completely
		    resolved.
		    The values Font::IGNORE_* and Font::TOGGLE are NOT
		    allowed in these font tables.
		*/
		Font font_;
	};
	///
	friend class matchFT;
	///
	class matchFT {
	public:
		/// used by lower_bound and upper_bound
		int operator()(FontTable const & a, FontTable const & b) const {
			return a.pos() < b.pos();
		}
	};

	///
	typedef std::vector<FontTable> FontList;
	///
	FontList fontlist;

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
	/// Output consecutive unicode chars, belonging to the same script as
	/// specified by the latex macro \p ltx, to \p os starting from \p c.
	/// \return the number of characters written.
	int writeScriptChars(odocstream & os, value_type c, docstring const & ltx,
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
	unsigned int id_;
	///
	static unsigned int paragraph_id;
	///
	ParagraphParameters params;

//private:
	///
	pos_type size() const { return owner_->size(); }
	/// match a string against a particular point in the paragraph
	bool isTextAt(std::string const & str, pos_type pos) const;

	/// for recording and looking up changes
	Changes changes_;

	/// Who owns us?
	Paragraph * owner_;
};




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


bool Paragraph::Pimpl::isChanged(pos_type start, pos_type end) const
{
	BOOST_ASSERT(start >= 0 && start <= size());
	BOOST_ASSERT(end > start && end <= size() + 1);

	return changes_.isChanged(start, end);
}


bool Paragraph::Pimpl::isMergedOnEndOfParDeletion(bool trackChanges) const {
	// keep the logic here in sync with the logic of eraseChars()

	if (!trackChanges) {
		return true;
	}

	Change change = changes_.lookup(size());

	return change.type == Change::INSERTED && change.author == 0;
}


void Paragraph::Pimpl::setChange(Change const & change)
{
	// beware of the imaginary end-of-par character!
	changes_.set(change, 0, size() + 1);

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
			if (owner_->isInset(pos)) {
				owner_->getInset(pos)->setChange(change);
			}
		}
	}
}


void Paragraph::Pimpl::setChange(pos_type pos, Change const & change)
{
	BOOST_ASSERT(pos >= 0 && pos <= size());

	changes_.set(change, pos);

	// see comment in setChange(Change const &) above

	if (change.type != Change::DELETED &&
	    pos < size() && owner_->isInset(pos)) {
		owner_->getInset(pos)->setChange(change);
	}
}


Change const & Paragraph::Pimpl::lookupChange(pos_type pos) const
{
	BOOST_ASSERT(pos >= 0 && pos <= size());

	return changes_.lookup(pos);
}


void Paragraph::Pimpl::acceptChanges(BufferParams const & bparams, pos_type start, pos_type end)
{
	BOOST_ASSERT(start >= 0 && start <= size());
	BOOST_ASSERT(end > start && end <= size() + 1);

	for (pos_type pos = start; pos < end; ++pos) {
		switch (lookupChange(pos).type) {
			case Change::UNCHANGED:
				// accept changes in nested inset
				if (pos < size() && owner_->isInset(pos)) {
					owner_->getInset(pos)->acceptChanges(bparams);
				}

				break;

			case Change::INSERTED:
				changes_.set(Change(Change::UNCHANGED), pos);
				// also accept changes in nested inset
				if (pos < size() && owner_->isInset(pos)) {
					owner_->getInset(pos)->acceptChanges(bparams);
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


void Paragraph::Pimpl::rejectChanges(BufferParams const & bparams, pos_type start, pos_type end)
{
	BOOST_ASSERT(start >= 0 && start <= size());
	BOOST_ASSERT(end > start && end <= size() + 1);

	for (pos_type pos = start; pos < end; ++pos) {
		switch (lookupChange(pos).type) {
			case Change::UNCHANGED:
				// reject changes in nested inset
				if (pos < size() && owner_->isInset(pos)) {
					owner_->getInset(pos)->rejectChanges(bparams);
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
				changes_.set(Change(Change::UNCHANGED), pos);

				// Do NOT reject changes within a deleted inset!
				// There may be insertions of a co-author inside of it!

				break;
		}
	}
}


Paragraph::value_type Paragraph::Pimpl::getChar(pos_type pos) const
{
	BOOST_ASSERT(pos >= 0 && pos <= size());

	return owner_->getChar(pos);
}


void Paragraph::Pimpl::insertChar(pos_type pos, value_type c, Change const & change)
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
	FontTable search_font(pos, Font());
	for (FontList::iterator it
	      = lower_bound(fontlist.begin(), fontlist.end(), search_font, matchFT());
	     it != fontlist.end(); ++it)
	{
		it->pos(it->pos() + 1);
	}

	// Update the insets
	owner_->insetlist.increasePosAfterPos(pos);
}


void Paragraph::Pimpl::insertInset(pos_type pos, Inset * inset,
				   Change const & change)
{
	BOOST_ASSERT(inset);
	BOOST_ASSERT(pos >= 0 && pos <= size());

	insertChar(pos, META_INSET, change);
	BOOST_ASSERT(owner_->text_[pos] == META_INSET);

	// Add a new entry in the insetlist.
	owner_->insetlist.insert(inset, pos);
}


bool Paragraph::Pimpl::eraseChar(pos_type pos, bool trackChanges)
{
	BOOST_ASSERT(pos >= 0 && pos <= size());

	// keep the logic here in sync with the logic of isMergedOnEndOfParDeletion()

	if (trackChanges) {
		Change change = changes_.lookup(pos);

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
	changes_.erase(pos);

	// if it is an inset, delete the inset entry
	if (owner_->text_[pos] == Paragraph::META_INSET) {
		owner_->insetlist.erase(pos);
	}

	owner_->text_.erase(owner_->text_.begin() + pos);

	// Erase entries in the tables.
	FontTable search_font(pos, Font());

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

	return true;
}


int Paragraph::Pimpl::eraseChars(pos_type start, pos_type end, bool trackChanges)
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


int Paragraph::Pimpl::latexSurrogatePair(odocstream & os, value_type c,
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
	if (docstring(1, next) == latex1) {
		os << latex2 << latex1;
		return latex1.length() + latex2.length();
	} else
		os << latex1 << '{' << latex2 << '}';
	return latex1.length() + latex2.length() + 2;
}


bool Paragraph::Pimpl::simpleTeXBlanks(Encoding const & encoding,
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


int Paragraph::Pimpl::writeScriptChars(odocstream & os,
				       value_type /*c*/,
				       docstring const & ltx,
				       Change & runningChange,
				       Encoding const & encoding,
				       pos_type & i)
{
	// We only arrive here when a proper language for character c has not
	// been specified (i.e., it could not be translated in the current
	// latex encoding) and it belongs to a known script.
	// Parameter ltx contains the latex translation of c as specified in
	// the unicodesymbols file and is something like "\textXXX{<spec>}".
	// The latex macro name "textXXX" specifies the script to which c
	// belongs and we use it in order to check whether characters from the
	// same script immediately follow, such that we can collect them in a
	// single "\textXXX" macro. So, we have to retain "\textXXX{<spec>"
	// for the first char but only "<spec>" for all subsequent chars.
	docstring::size_type const brace1 = ltx.find_first_of(from_ascii("{"));
	docstring::size_type const brace2 = ltx.find_last_of(from_ascii("}"));
	string script = to_ascii(ltx.substr(1, brace1 - 1));
	int length = ltx.substr(0, brace2).length();
	os << ltx.substr(0, brace2);
	while (i + 1 < size()) {
		char_type const next = getChar(i + 1);
		// Stop here if next character belongs to another script
		// or there is a change in change tracking status.
		if (!Encodings::isKnownScriptChar(next, script) ||
		    runningChange != lookupChange(i + 1))
			break;
		Font prev_font;
		bool found = false;
		FontList::const_iterator cit = fontlist.begin();
		FontList::const_iterator end = fontlist.end();
		for (; cit != end; ++cit) {
			if (cit->pos() >= i && !found) {
				prev_font = cit->font();
				found = true;
			}
			if (cit->pos() >= i + 1)
				break;
		}
		// Stop here if there is a font attribute or encoding change.
		if (found && cit != end && prev_font != cit->font())
			break;
		docstring const latex = encoding.latexChar(next);
		docstring::size_type const b1 =
					latex.find_first_of(from_ascii("{"));
		docstring::size_type const b2 =
					latex.find_last_of(from_ascii("}"));
		int const len = b2 - b1 - 1;
		os << latex.substr(b1 + 1, len);
		length += len;
		++i;
	}
	os << '}';
	++length;
	return length;
}


bool Paragraph::Pimpl::isTextAt(string const & str, pos_type pos) const
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
		if (inset->lyxCode() == Inset::NEWLINE_CODE) {
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

		if (lookupChange(i).type == Change::DELETED) {
			if( ++runparams.inDeletedInset == 1)
				runparams.changeOfDeletedInset = lookupChange(i);
		}

		if (inset->canTrackChanges()) {
			column += Changes::latexMarkChange(os, bparams, running_change,
				Change(Change::UNCHANGED));
			running_change = Change(Change::UNCHANGED);
		}

		bool close = false;
		odocstream::pos_type const len = os.tellp();

		if ((inset->lyxCode() == Inset::GRAPHICS_CODE
		     || inset->lyxCode() == Inset::MATH_CODE
		     || inset->lyxCode() == Inset::URL_CODE)
		    && running_font.isRightToLeft()) {
		    	if (running_font.language()->lang() == "farsi")
				os << "\\beginL{}";
			else
				os << "\\L{";
			close = true;
		}

#ifdef WITH_WARNINGS
#warning Bug: we can have an empty font change here!
// if there has just been a font change, we are going to close it
// right now, which means stupid latex code like \textsf{}. AFAIK,
// this does not harm dvi output. A minor bug, thus (JMarc)
#endif
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

		if (lookupChange(i).type == Change::DELETED) {
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
#ifdef WITH_WARNINGS
#warning Can someone explain this silly method?
// JSpitzm 2008/07/03
// This looks like complete nonsense to me. Cf. the comments
// below. Since this triggers iconv exceptions (see bug 4727),
// I have commented this out for the time being.
#endif
#if 0
			// I assume this is hack treating typewriter as verbatim
			// FIXME UNICODE: This can fail if c cannot be encoded
			// in the current encoding.
			if (running_font.family() == Font::TYPEWRITER_FAMILY) {
				if (c != '\0') {
					os.put(c);
				}
				break;
			}
#endif

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
				string script;
				docstring const latex = encoding.latexChar(c);
				if (Encodings::isKnownScriptChar(c, script) &&
				    prefixIs(latex, from_ascii("\\" + script)))
					column += writeScriptChars(os, c, latex,
							running_change,
							encoding, i) - 1;
				else if (latex.length() > 1 &&
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


void Paragraph::Pimpl::validate(LaTeXFeatures & features,
				Layout const & layout) const
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

	if (!params.leftIndent().zero())
		features.require("ParagraphLeftIndent");

	// then the insets
	InsetList::const_iterator icit = owner_->insetlist.begin();
	InsetList::const_iterator iend = owner_->insetlist.end();
	for (; icit != iend; ++icit) {
		if (icit->inset) {
			icit->inset->validate(features);
			if (layout.needprotect &&
			    icit->inset->lyxCode() == Inset::FOOT_CODE)
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
	: begin_of_body_(0), pimpl_(new Paragraph::Pimpl(this))
{
	itemdepth = 0;
	params().clear();
}


Paragraph::Paragraph(Paragraph const & par)
	: itemdepth(par.itemdepth), insetlist(par.insetlist),
	layout_(par.layout_),
	text_(par.text_), begin_of_body_(par.begin_of_body_),
	pimpl_(new Paragraph::Pimpl(*par.pimpl_, this))
{
	//lyxerr << "Paragraph::Paragraph(Paragraph const&)" << endl;
	InsetList::iterator it = insetlist.begin();
	InsetList::iterator end = insetlist.end();
	for (; it != end; ++it)
		it->inset = it->inset->clone().release();
}


Paragraph & Paragraph::operator=(Paragraph const & par)
{
	// needed as we will destroy the pimpl_ before copying it
	if (&par != this) {
		itemdepth = par.itemdepth;

		insetlist = par.insetlist;
		InsetList::iterator it = insetlist.begin();
		InsetList::iterator end = insetlist.end();
		for (; it != end; ++it)
			it->inset = it->inset->clone().release();

		layout_ = par.layout();
		text_ = par.text_;
		begin_of_body_ = par.begin_of_body_;

		delete pimpl_;
		pimpl_ = new Pimpl(*par.pimpl_, this);
	}
	return *this;
}


Paragraph::~Paragraph()
{
	delete pimpl_;
	//
	//lyxerr << "Paragraph::paragraph_id = "
	//       << Paragraph::paragraph_id << endl;
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

		Change change = pimpl_->lookupChange(i);
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
	pimpl_->validate(features, *layout());
}


bool Paragraph::eraseChar(pos_type pos, bool trackChanges)
{
	return pimpl_->eraseChar(pos, trackChanges);
}


int Paragraph::eraseChars(pos_type start, pos_type end, bool trackChanges)
{
	return pimpl_->eraseChars(start, end, trackChanges);
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
	pimpl_->insertChar(pos, c, Change(trackChanges ?
			   Change::INSERTED : Change::UNCHANGED));
}


void Paragraph::insertChar(pos_type pos, Paragraph::value_type c,
			   Font const & font, bool trackChanges)
{
	pimpl_->insertChar(pos, c, Change(trackChanges ?
			   Change::INSERTED : Change::UNCHANGED));
	setFont(pos, font);
}


void Paragraph::insertChar(pos_type pos, Paragraph::value_type c,
			   Font const & font, Change const & change)
{
	pimpl_->insertChar(pos, c, change);
	setFont(pos, font);
}


void Paragraph::insertInset(pos_type pos, Inset * inset,
			    Change const & change)
{
	pimpl_->insertInset(pos, inset, change);
}


void Paragraph::insertInset(pos_type pos, Inset * inset,
			    Font const & font, Change const & change)
{
	pimpl_->insertInset(pos, inset, change);
	// Set the font/language of the inset...
	setFont(pos, font);
	// ... as well as the font/language of the text inside the inset
	// FIXME: This is far from perfect. It basically overrides work being done
	// in the InsetText constructor. Also, it doesn't work for Tables 
	// (precisely because each cell's font/language is set in the Table's 
	// constructor, so by now it's too late). The long-term solution should
	// be moving current_font into Cursor, and getting rid of all this...
	// (see http://thread.gmane.org/gmane.editors.lyx.devel/88869/focus=88944)
	if (inset->asTextInset()) {
		inset->asTextInset()->text_.current_font = font;
		inset->asTextInset()->text_.real_current_font = font;
	}
}


bool Paragraph::insetAllowed(Inset_code code)
{
	return !pimpl_->inset_owner || pimpl_->inset_owner->insetAllowed(code);
}


// Gets uninstantiated font setting at position.
Font const Paragraph::getFontSettings(BufferParams const & bparams,
					 pos_type pos) const
{
	if (pos > size()) {
		lyxerr << " pos: " << pos << " size: " << size() << endl;
		BOOST_ASSERT(pos <= size());
	}

	Pimpl::FontList::const_iterator cit = pimpl_->fontlist.begin();
	Pimpl::FontList::const_iterator end = pimpl_->fontlist.end();
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

	Pimpl::FontList::const_iterator cit = pimpl_->fontlist.begin();
	Pimpl::FontList::const_iterator end = pimpl_->fontlist.end();
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
	if (!empty() && !pimpl_->fontlist.empty())
		return pimpl_->fontlist[0].font();

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

	Layout_ptr const & lout = layout();

	pos_type const body_pos = beginOfBody();

	Font layoutfont;
	if (pos < body_pos)
		layoutfont = lout->labelfont;
	else
		layoutfont = lout->font;

	Font font = getFontSettings(bparams, pos);
	font.realize(layoutfont);
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
	if (pimpl_->fontlist.empty())
		return def_size;

	Pimpl::FontList::const_iterator end_it = pimpl_->fontlist.begin();
	Pimpl::FontList::const_iterator const end = pimpl_->fontlist.end();
	for (; end_it != end; ++end_it) {
		if (end_it->pos() >= endpos)
			break;
	}

	if (end_it != end)
		++end_it;

	Pimpl::FontList::const_iterator cit = pimpl_->fontlist.begin();
	for (; cit != end; ++cit) {
		if (cit->pos() >= startpos)
			break;
	}

	Font::FONT_SIZE maxsize = Font::SIZE_TINY;
	for (; cit != end_it; ++cit) {
		Font::FONT_SIZE size = cit->font().size();
		if (size == Font::INHERIT_SIZE)
			size = def_size;
		if (size > maxsize && size <= Font::SIZE_HUGER)
			maxsize = size;
	}
	return maxsize;
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
	// No need to simplify this because it will disappear
	// in a new kernel. (Asger)
	// Next search font table

	Pimpl::FontList::iterator beg = pimpl_->fontlist.begin();
	Pimpl::FontList::iterator it = beg;
	Pimpl::FontList::iterator endit = pimpl_->fontlist.end();
	for (; it != endit; ++it) {
		if (it->pos() >= pos)
			break;
	}
	size_t const i = distance(beg, it);
	bool notfound = (it == endit);

	if (!notfound && pimpl_->fontlist[i].font() == font)
		return;

	bool begin = pos == 0 || notfound ||
		(i > 0 && pimpl_->fontlist[i - 1].pos() == pos - 1);
	// Is position pos is a beginning of a font block?
	bool end = !notfound && pimpl_->fontlist[i].pos() == pos;
	// Is position pos is the end of a font block?
	if (begin && end) { // A single char block
		if (i + 1 < pimpl_->fontlist.size() &&
		    pimpl_->fontlist[i + 1].font() == font) {
			// Merge the singleton block with the next block
			pimpl_->fontlist.erase(pimpl_->fontlist.begin() + i);
			if (i > 0 && pimpl_->fontlist[i - 1].font() == font)
				pimpl_->fontlist.erase(pimpl_->fontlist.begin() + i - 1);
		} else if (i > 0 && pimpl_->fontlist[i - 1].font() == font) {
			// Merge the singleton block with the previous block
			pimpl_->fontlist[i - 1].pos(pos);
			pimpl_->fontlist.erase(pimpl_->fontlist.begin() + i);
		} else
			pimpl_->fontlist[i].font(font);
	} else if (begin) {
		if (i > 0 && pimpl_->fontlist[i - 1].font() == font)
			pimpl_->fontlist[i - 1].pos(pos);
		else
			pimpl_->fontlist.insert(pimpl_->fontlist.begin() + i,
					Pimpl::FontTable(pos, font));
	} else if (end) {
		pimpl_->fontlist[i].pos(pos - 1);
		if (!(i + 1 < pimpl_->fontlist.size() &&
		      pimpl_->fontlist[i + 1].font() == font))
			pimpl_->fontlist.insert(pimpl_->fontlist.begin() + i + 1,
					Pimpl::FontTable(pos, font));
	} else { // The general case. The block is splitted into 3 blocks
		pimpl_->fontlist.insert(pimpl_->fontlist.begin() + i,
				Pimpl::FontTable(pos - 1, pimpl_->fontlist[i].font()));
		pimpl_->fontlist.insert(pimpl_->fontlist.begin() + i + 1,
				Pimpl::FontTable(pos, font));
	}
}


void Paragraph::makeSameLayout(Paragraph const & par)
{
	layout(par.layout());
	// move to pimpl?
	params() = par.params();
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
	return par.layout() == layout() && params().sameLayout(par.params());
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


docstring Paragraph::expandLabel(Layout_ptr const & layout,
		BufferParams const & bparams, bool process_appendix) const
{
	TextClass const & tclass = bparams.getTextClass();

	docstring fmt;
	if (process_appendix && params().appendix())
		fmt = translateIfPossible(layout->labelstring_appendix(),
			bparams);
	else
		fmt = translateIfPossible(layout->labelstring(), bparams);

	// handle 'inherited level parts' in 'fmt',
	// i.e. the stuff between '@' in   '@Section@.\arabic{subsection}'
	size_t const i = fmt.find('@', 0);
	if (i != docstring::npos) {
		size_t const j = fmt.find('@', i + 1);
		if (j != docstring::npos) {
			docstring parent(fmt, i + 1, j - i - 1);
			docstring label = from_ascii("??");
			if (tclass.hasLayout(parent))
				label = expandLabel(tclass[parent], bparams);
			fmt = docstring(fmt, 0, i) + label + docstring(fmt, j + 1, docstring::npos);
		}
	}

	return tclass.counters().counterLabel(fmt);
}


void Paragraph::applyLayout(Layout_ptr const & new_layout)
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
	InsetList::const_iterator it = insetlist.begin();
	InsetList::const_iterator end = insetlist.end();
	for (; it != end; ++it)
		if (it->inset == inset)
			return it->pos;
	return -1;
}


InsetBibitem * Paragraph::bibitem() const
{
	if (!insetlist.empty()) {
		Inset * inset = insetlist.begin()->inset;
		if (inset->lyxCode() == Inset::BIBITEM_CODE)
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

bool noTrivlistCentering(Inset::Code code)
{
	return code == Inset::FLOAT_CODE || code == Inset::WRAP_CODE;
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
	Inset::Code code)
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

	Layout_ptr style;

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
		                                                 : pimpl_->lookupChange(i);

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

		// close babel's font environment before opening CJK.
		if (!running_font.language()->babel().empty() &&
		    font.language()->encoding()->package() == Encoding::CJK) {
				string end_tag = subst(lyxrc.language_command_end,
							"$$lang",
							running_font.language()->babel());
				os << from_ascii(end_tag);
				column += end_tag.length();
		}

		// Switch file encoding if necessary
		if (runparams.encoding->package() != Encoding::none &&
		    font.language()->encoding()->package() != Encoding::none) {
			std::pair<bool, int> const enc_switch = switchEncoding(os, bparams,
					runparams, *(font.language()->encoding()));
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
				if (pimpl_->simpleTeXBlanks(
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
		
		try {
			pimpl_->simpleTeXSpecialChars(buf, bparams, os,
					texrow, rp, running_font,
					basefont, outerfont, open_font,
					runningChange, *style, i, column, c);
		} catch (EncodingException & e) {
			if (runparams.dryrun) {
				os << "<" << _("LyX Warning: ")
				   << _("uncodable character") << " '";
				os.put(c);
				os << "'>";
			} else {
				// add location information and throw again.
				e.par_id = id();
				e.pos = i;
				throw(e);
			}
		}

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
#ifdef WITH_WARNINGS
//#warning For now we ALWAYS have to close the foreign font settings if they are
//#warning there as we start another \selectlanguage with the next paragraph if
//#warning we are in need of this. This should be fixed sometime (Jug)
#endif
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
			Inset::Code lyx_code = inset->lyxCode();
			if (lyx_code != Inset::TOC_CODE &&
			    lyx_code != Inset::INCLUDE_CODE &&
			    lyx_code != Inset::GRAPHICS_CODE &&
			    lyx_code != Inset::ERT_CODE &&
			    lyx_code != Inset::LISTINGS_CODE &&
			    lyx_code != Inset::FLOAT_CODE &&
			    lyx_code != Inset::TABULAR_CODE) {
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
			Inset::Code lyx_code = inset->lyxCode();
			if (lyx_code == Inset::LABEL_CODE) {
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

	Layout_ptr const & style = layout();
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


bool Paragraph::isNewline(pos_type pos) const
{
	return isInset(pos)
		&& getInset(pos)->lyxCode() == Inset::NEWLINE_CODE;
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


bool Paragraph::isChar(pos_type pos) const
{
	if (isInset(pos))
		return getInset(pos)->isChar();
	else {
		value_type const c = getChar(pos);
		return !isLetterChar(c) && !isDigit(c) && !lyx::isSpace(c);
	}
}


bool Paragraph::isSpace(pos_type pos) const
{
	if (isInset(pos))
		return getInset(pos)->isSpace();
	else {
		value_type const c = getChar(pos);
		return lyx::isSpace(c);
	}
}


Language const *
Paragraph::getParLanguage(BufferParams const & bparams) const
{
	if (!empty())
		return getFirstFontSettings(bparams).language();
#ifdef WITH_WARNINGS
#warning FIXME we should check the prev par as well (Lgb)
#endif
	return bparams.language;
}


bool Paragraph::isRightToLeftPar(BufferParams const & bparams) const
{
	return lyxrc.rtl_support
		&& getParLanguage(bparams)->rightToLeft()
		&& ownerCode() != Inset::ERT_CODE
		&& ownerCode() != Inset::LISTINGS_CODE;
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
	Pimpl::FontList::const_iterator cit = pimpl_->fontlist.begin();
	Pimpl::FontList::const_iterator end = pimpl_->fontlist.end();

	for (; cit != end; ++cit)
		if (cit->font().language() != ignore_language &&
		    cit->font().language() != latex_language &&
		    cit->font().language() != doc_language)
			return true;
	return false;
}


docstring const Paragraph::printableString(bool label) const
{
	odocstringstream os;
	if (label && !params().labelString().empty())
		os << params().labelString() << ' ';
	pos_type end = size();
	for (pos_type i = 0; i < end; ++i) {
		value_type const c = getChar(i);
		if (isPrintable(c))
			os.put(c);
	}
	return os.str();
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
	pimpl_->inset_owner = inset;
}


Change const & Paragraph::lookupChange(pos_type pos) const
{
	BOOST_ASSERT(pos <= size());
	return pimpl_->lookupChange(pos);
}


bool Paragraph::isChanged(pos_type start, pos_type end) const
{
	return pimpl_->isChanged(start, end);
}


bool Paragraph::isMergedOnEndOfParDeletion(bool trackChanges) const
{
	return pimpl_->isMergedOnEndOfParDeletion(trackChanges);
}


void Paragraph::setChange(Change const & change)
{
	pimpl_->setChange(change);
}


void Paragraph::setChange(pos_type pos, Change const & change)
{
	pimpl_->setChange(pos, change);
}


void Paragraph::acceptChanges(BufferParams const & bparams, pos_type start, pos_type end)
{
	return pimpl_->acceptChanges(bparams, start, end);
}


void Paragraph::rejectChanges(BufferParams const & bparams, pos_type start, pos_type end)
{
	return pimpl_->rejectChanges(bparams, start, end);
}


int Paragraph::id() const
{
	return pimpl_->id_;
}


Layout_ptr const & Paragraph::layout() const
{
	return layout_;
}


void Paragraph::layout(Layout_ptr const & new_layout)
{
	layout_ = new_layout;
}


Inset * Paragraph::inInset() const
{
	return pimpl_->inset_owner;
}


Inset::Code Paragraph::ownerCode() const
{
	return pimpl_->inset_owner
		? pimpl_->inset_owner->lyxCode() : Inset::NO_CODE;
}


ParagraphParameters & Paragraph::params()
{
	return pimpl_->params;
}


ParagraphParameters const & Paragraph::params() const
{
	return pimpl_->params;
}


bool Paragraph::isFreeSpacing() const
{
	if (layout()->free_spacing)
		return true;

	// for now we just need this, later should we need this in some
	// other way we can always add a function to Inset too.
	return ownerCode() == Inset::ERT_CODE || ownerCode() == Inset::LISTINGS_CODE;
}


bool Paragraph::allowEmpty() const
{
	if (layout()->keepempty)
		return true;
	return ownerCode() == Inset::ERT_CODE || ownerCode() == Inset::LISTINGS_CODE;
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


bool Paragraph::hfillExpansion(Row const & row, pos_type pos) const
{
	if (!isHfill(pos))
		return false;

	BOOST_ASSERT(pos >= row.pos() && pos < row.endpos());

	// expand at the end of a row only if there is another hfill on the same row
	if (pos == row.endpos() - 1) {
		for (pos_type i = row.pos(); i < pos; i++) {
			if (isHfill(i))
				return true;
		}
		return false;
	}

	// expand at the beginning of a row only if it is the first row of a paragraph
	if (pos == row.pos()) {
		return pos == 0;
	}

	// do not expand in some labels
	if (layout()->margintype != MARGIN_MANUAL && pos < beginOfBody())
		return false;

	// if there is anything between the first char of the row and
	// the specified position that is neither a newline nor an hfill,
	// the hfill will be expanded, otherwise it won't
	for (pos_type i = row.pos(); i < pos; i++) {
		if (!isNewline(i) && !isHfill(i))
			return true;
	}
	return false;
}


int Paragraph::checkBiblio(bool track_changes)
{
	//FIXME From JS:
	//This is getting more and more a mess. ...We really should clean
	//up this bibitem issue for 1.6. See also bug 2743.

	// Add bibitem insets if necessary
	if (layout()->labeltype != LABEL_BIBLIO)
		return 0;

	bool hasbibitem = !insetlist.empty()
		// Insist on it being in pos 0
		&& getChar(0) == Paragraph::META_INSET
		&& insetlist.begin()->inset->lyxCode() == Inset::BIBITEM_CODE;

	docstring oldkey;
	docstring oldlabel;

	// remove a bibitem in pos != 0
	// restore it later in pos 0 if necessary
	// (e.g. if a user inserts contents _before_ the item)
	// we're assuming there's only one of these, which there
	// should be.
	int erasedInsetPosition = -1;
	InsetList::iterator it = insetlist.begin();
	InsetList::iterator end = insetlist.end();
	for (; it != end; ++it)
		if (it->inset->lyxCode() == Inset::BIBITEM_CODE
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
			static_cast<InsetBibitem *>(insetlist.begin()->inset);
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
	pimpl_->changes_.checkAuthors(authorList);
}

} // namespace lyx
