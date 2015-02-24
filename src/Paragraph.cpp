/**
 * \file Paragraph.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup
 * \author Lars Gullik Bjønnes
 * \author Richard Heck (XHTML output)
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

#include "LayoutFile.h"
#include "Buffer.h"
#include "BufferParams.h"
#include "Changes.h"
#include "Counters.h"
#include "BufferEncodings.h"
#include "InsetList.h"
#include "Language.h"
#include "LaTeXFeatures.h"
#include "Layout.h"
#include "Length.h"
#include "Font.h"
#include "FontList.h"
#include "LyXRC.h"
#include "OutputParams.h"
#include "output_latex.h"
#include "output_xhtml.h"
#include "ParagraphParameters.h"
#include "SpellChecker.h"
#include "sgml.h"
#include "TextClass.h"
#include "TexRow.h"
#include "Text.h"
#include "WordLangTuple.h"
#include "WordList.h"

#include "frontends/alert.h"

#include "insets/InsetBibitem.h"
#include "insets/InsetLabel.h"
#include "insets/InsetSpecialChar.h"

#include "support/debug.h"
#include "support/docstring_list.h"
#include "support/ExceptionMessage.h"
#include "support/gettext.h"
#include "support/lassert.h"
#include "support/lstrings.h"
#include "support/textutils.h"

#include <sstream>
#include <vector>

using namespace std;
using namespace lyx::support;

namespace lyx {

namespace {

/// Inset identifier (above 0x10ffff, for ucs-4)
char_type const META_INSET = 0x200001;

}


/////////////////////////////////////////////////////////////////////
//
// SpellResultRange
//
/////////////////////////////////////////////////////////////////////

class SpellResultRange {
public:
	SpellResultRange(FontSpan range, SpellChecker::Result result)
	: range_(range), result_(result)
	{}
	///
	FontSpan const & range() const { return range_; }
	///
	void range(FontSpan const & r) { range_ = r; }
	///
	SpellChecker::Result result() const { return result_; }
	///
	void result(SpellChecker::Result r) { result_ = r; }
	///
	bool contains(pos_type pos) const { return range_.contains(pos); }
	///
	bool covered(FontSpan const & r) const
	{
		// 1. first of new range inside current range or
		// 2. last of new range inside current range or
		// 3. first of current range inside new range or
		// 4. last of current range inside new range
		//FIXME: is this the same as !range_.intersect(r).empty() ?
		return range_.contains(r.first) || range_.contains(r.last) ||
			r.contains(range_.first) || r.contains(range_.last);
	}
	///
	void shift(pos_type pos, int offset)
	{
		if (range_.first > pos) {
			range_.first += offset;
			range_.last += offset;
		} else if (range_.last >= pos) {
			range_.last += offset;
		}
	}
private:
	FontSpan range_ ;
	SpellChecker::Result result_ ;
};


/////////////////////////////////////////////////////////////////////
//
// SpellCheckerState
//
/////////////////////////////////////////////////////////////////////

class SpellCheckerState {
public:
	SpellCheckerState()
	{
		needs_refresh_ = true;
		current_change_number_ = 0;
	}

	void setRange(FontSpan const & fp, SpellChecker::Result state)
	{
		Ranges result;
		RangesIterator et = ranges_.end();
		RangesIterator it = ranges_.begin();
		for (; it != et; ++it) {
			if (!it->covered(fp))
				result.push_back(SpellResultRange(it->range(), it->result()));
			else if (state == SpellChecker::WORD_OK) {
				// trim or split the current misspelled range
				// store misspelled ranges only
				FontSpan range = it->range();
				if (fp.first > range.first) {
					// misspelled area in front of WORD_OK
					range.last = fp.first - 1;
					result.push_back(SpellResultRange(range, it->result()));
					range = it->range();
				}
				if (fp.last < range.last) {
					// misspelled area after WORD_OK range
					range.first = fp.last + 1;
					result.push_back(SpellResultRange(range, it->result()));
				}
			}
		}
		ranges_ = result;
		if (state != SpellChecker::WORD_OK)
			ranges_.push_back(SpellResultRange(fp, state));
	}

	void increasePosAfterPos(pos_type pos)
	{
		correctRangesAfterPos(pos, 1);
		needsRefresh(pos);
	}

	void decreasePosAfterPos(pos_type pos)
	{
		correctRangesAfterPos(pos, -1);
		needsRefresh(pos);
	}

	void refreshLast(pos_type pos)
	{
		if (pos < refresh_.last)
			refresh_.last = pos;
	}

	SpellChecker::Result getState(pos_type pos) const
	{
		SpellChecker::Result result = SpellChecker::WORD_OK;
		RangesIterator et = ranges_.end();
		RangesIterator it = ranges_.begin();
		for (; it != et; ++it) {
			if(it->contains(pos)) {
				return it->result();
			}
		}
		return result;
	}

	FontSpan const & getRange(pos_type pos) const
	{
		/// empty span to indicate mismatch
		static FontSpan empty_;
		RangesIterator et = ranges_.end();
		RangesIterator it = ranges_.begin();
		for (; it != et; ++it) {
			if(it->contains(pos)) {
				return it->range();
			}
		}
		return empty_;
	}

	bool needsRefresh() const
	{
		return needs_refresh_;
	}

	SpellChecker::ChangeNumber currentChangeNumber() const
	{
		return current_change_number_;
	}

	void refreshRange(pos_type & first, pos_type & last) const
	{
		first = refresh_.first;
		last = refresh_.last;
	}

	void needsRefresh(pos_type pos)
	{
		if (needs_refresh_ && pos != -1) {
			if (pos < refresh_.first)
				refresh_.first = pos;
			if (pos > refresh_.last)
				refresh_.last = pos;
		} else if (pos != -1) {
			// init request check for neighbour positions too
			refresh_.first = pos > 0 ? pos - 1 : 0;
			// no need for special end of paragraph check
			refresh_.last = pos + 1;
		}
		needs_refresh_ = pos != -1;
	}

	void needsCompleteRefresh(SpellChecker::ChangeNumber change_number)
	{
		needs_refresh_ = true;
		refresh_.first = 0;
		refresh_.last = -1;
		current_change_number_ = change_number;
	}
private:
	typedef vector<SpellResultRange> Ranges;
	typedef Ranges::const_iterator RangesIterator;
	Ranges ranges_;
	/// the area of the paragraph with pending spell check
	FontSpan refresh_;
	bool needs_refresh_;
	/// spell state cache version number
	SpellChecker::ChangeNumber current_change_number_;


	void correctRangesAfterPos(pos_type pos, int offset)
	{
		RangesIterator et = ranges_.end();
		Ranges::iterator it = ranges_.begin();
		for (; it != et; ++it) {
			it->shift(pos, offset);
		}
	}

};

/////////////////////////////////////////////////////////////////////
//
// Paragraph::Private
//
/////////////////////////////////////////////////////////////////////

class Paragraph::Private
{
	// Enforce our own "copy" constructor by declaring the standard one and
	// the assignment operator private without implementing them.
	Private(Private const &);
	Private & operator=(Private const &);
public:
	///
	Private(Paragraph * owner, Layout const & layout);
	/// "Copy constructor"
	Private(Private const &, Paragraph * owner);
	/// Copy constructor from \p beg  to \p end
	Private(Private const &, Paragraph * owner, pos_type beg, pos_type end);

	///
	void insertChar(pos_type pos, char_type c, Change const & change);

	/// Output the surrogate pair formed by \p c and \p next to \p os.
	/// \return the number of characters written.
	int latexSurrogatePair(otexstream & os, char_type c, char_type next,
			       OutputParams const &);

	/// Output a space in appropriate formatting (or a surrogate pair
	/// if the next character is a combining character).
	/// \return whether a surrogate pair was output.
	bool simpleTeXBlanks(OutputParams const &,
			     otexstream &,
			     pos_type i,
			     unsigned int & column,
			     Font const & font,
			     Layout const & style);

	/// Output consecutive unicode chars, belonging to the same script as
	/// specified by the latex macro \p ltx, to \p os starting from \p i.
	/// \return the number of characters written.
	int writeScriptChars(otexstream & os, docstring const & ltx,
			   Change const &, Encoding const &, pos_type & i);

	/// This could go to ParagraphParameters if we want to.
	int startTeXParParams(BufferParams const &, otexstream &,
			      OutputParams const &) const;

	/// This could go to ParagraphParameters if we want to.
	bool endTeXParParams(BufferParams const &, otexstream &,
			     OutputParams const &) const;

	///
	void latexInset(BufferParams const &,
				   otexstream &,
				   OutputParams &,
				   Font & running_font,
				   Font & basefont,
				   Font const & outerfont,
				   bool & open_font,
				   Change & running_change,
				   Layout const & style,
				   pos_type & i,
				   unsigned int & column);

	///
	void latexSpecialChar(
				   otexstream & os,
				   BufferParams const & bparams,
				   OutputParams const & runparams,
				   Font const & running_font,
				   Change const & running_change,
				   Layout const & style,
				   pos_type & i,
				   pos_type end_pos,
				   unsigned int & column);

	///
	bool latexSpecialT1(
		char_type const c,
		otexstream & os,
		pos_type i,
		unsigned int & column);
	///
	bool latexSpecialT3(
		char_type const c,
		otexstream & os,
		pos_type i,
		unsigned int & column);
	///
	bool latexSpecialPhrase(
		otexstream & os,
		pos_type & i,
		pos_type end_pos,
		unsigned int & column,
		OutputParams const & runparams);

	///
	void validate(LaTeXFeatures & features) const;

	/// Checks if the paragraph contains only text and no inset or font change.
	bool onlyText(Buffer const & buf, Font const & outerfont,
		      pos_type initial) const;

	/// match a string against a particular point in the paragraph
	bool isTextAt(string const & str, pos_type pos) const;

	/// a vector of speller skip positions
	typedef vector<FontSpan> SkipPositions;
	typedef SkipPositions::const_iterator SkipPositionsIterator;

	void appendSkipPosition(SkipPositions & skips, pos_type const pos) const;
	
	Language * getSpellLanguage(pos_type const from) const;

	Language * locateSpellRange(pos_type & from, pos_type & to,
				    SkipPositions & skips) const;

	bool hasSpellerChange() const
	{
		SpellChecker::ChangeNumber speller_change_number = 0;
		if (theSpellChecker())
			speller_change_number = theSpellChecker()->changeNumber();
		return speller_change_number > speller_state_.currentChangeNumber();
	}

	bool ignoreWord(docstring const & word) const ;
	
	void setMisspelled(pos_type from, pos_type to, SpellChecker::Result state)
	{
		pos_type textsize = owner_->size();
		// check for sane arguments
		if (to <= from || from >= textsize)
			return;
		FontSpan fp = FontSpan(from, to - 1);
		speller_state_.setRange(fp, state);
	}

	void requestSpellCheck(pos_type pos)
	{
		if (pos == -1)
			speller_state_.needsCompleteRefresh(speller_state_.currentChangeNumber());
		else
			speller_state_.needsRefresh(pos);
	}

	void readySpellCheck()
	{
		speller_state_.needsRefresh(-1);
	}

	bool needsSpellCheck() const
	{
		return speller_state_.needsRefresh();
	}

	void rangeOfSpellCheck(pos_type & first, pos_type & last) const
	{
		speller_state_.refreshRange(first, last);
		if (last == -1) {
			last = owner_->size();
			return;
		}
		pos_type endpos = last;
		owner_->locateWord(first, endpos, WHOLE_WORD);
		if (endpos < last) {
			endpos = last;
			owner_->locateWord(last, endpos, WHOLE_WORD);
		}
		last = endpos;
	}

	int countSkips(SkipPositionsIterator & it, SkipPositionsIterator const et,
			    int & start) const
	{
		int numskips = 0;
		while (it != et && it->first < start) {
			int skip = it->last - it->first + 1;
			start += skip;
			numskips += skip;
			++it;
		}
		return numskips;
	}

	void markMisspelledWords(pos_type const & first, pos_type const & last,
							 SpellChecker::Result result,
							 docstring const & word,
							 SkipPositions const & skips);

	InsetCode ownerCode() const
	{
		return inset_owner_ ? inset_owner_->lyxCode() : NO_CODE;
	}

	/// Which Paragraph owns us?
	Paragraph * owner_;

	/// In which Inset?
	Inset const * inset_owner_;

	///
	FontList fontlist_;

	///
	int id_;

	///
	ParagraphParameters params_;

	/// for recording and looking up changes
	Changes changes_;

	///
	InsetList insetlist_;

	/// end of label
	pos_type begin_of_body_;

	typedef docstring TextContainer;
	///
	TextContainer text_;

	typedef set<docstring> Words;
	typedef map<string, Words> LangWordsMap;
	///
	LangWordsMap words_;
	///
	Layout const * layout_;
	///
	SpellCheckerState speller_state_;
};


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


Paragraph::Private::Private(Paragraph * owner, Layout const & layout)
	: owner_(owner), inset_owner_(0), id_(-1), begin_of_body_(0), layout_(&layout)
{
	text_.reserve(100);
}


// Initialization of the counter for the paragraph id's,
//
// FIXME: There should be a more intelligent way to generate and use the
// paragraph ids per buffer instead a global static counter for all InsetText
// in the running program.
static int paragraph_id = -1;

Paragraph::Private::Private(Private const & p, Paragraph * owner)
	: owner_(owner), inset_owner_(p.inset_owner_), fontlist_(p.fontlist_),
	  params_(p.params_), changes_(p.changes_), insetlist_(p.insetlist_),
	  begin_of_body_(p.begin_of_body_), text_(p.text_), words_(p.words_),
	  layout_(p.layout_)
{
	id_ = ++paragraph_id;
	requestSpellCheck(p.text_.size());
}


Paragraph::Private::Private(Private const & p, Paragraph * owner,
	pos_type beg, pos_type end)
	: owner_(owner), inset_owner_(p.inset_owner_),
	  params_(p.params_), changes_(p.changes_),
	  insetlist_(p.insetlist_, beg, end),
	  begin_of_body_(p.begin_of_body_), words_(p.words_),
	  layout_(p.layout_)
{
	id_ = ++paragraph_id;
	if (beg >= pos_type(p.text_.size()))
		return;
	text_ = p.text_.substr(beg, end - beg);

	FontList::const_iterator fcit = fontlist_.begin();
	FontList::const_iterator fend = fontlist_.end();
	for (; fcit != fend; ++fcit) {
		if (fcit->pos() < beg)
			continue;
		if (fcit->pos() >= end) {
			// Add last entry in the fontlist_.
			fontlist_.set(text_.size() - 1, fcit->font());
			break;
		}
		// Add a new entry in the fontlist_.
		fontlist_.set(fcit->pos() - beg, fcit->font());
	}
	requestSpellCheck(p.text_.size());
}


void Paragraph::addChangesToToc(DocIterator const & cdit,
	Buffer const & buf, bool output_active) const
{
	d->changes_.addToToc(cdit, buf, output_active);
}


bool Paragraph::isDeleted(pos_type start, pos_type end) const
{
	LASSERT(start >= 0 && start <= size(), return false);
	LASSERT(end > start && end <= size() + 1, return false);

	return d->changes_.isDeleted(start, end);
}


bool Paragraph::isChanged(pos_type start, pos_type end) const
{
	LASSERT(start >= 0 && start <= size(), return false);
	LASSERT(end > start && end <= size() + 1, return false);

	return d->changes_.isChanged(start, end);
}


bool Paragraph::isMergedOnEndOfParDeletion(bool trackChanges) const
{
	// keep the logic here in sync with the logic of eraseChars()
	if (!trackChanges)
		return true;

	Change const & change = d->changes_.lookup(size());
	return change.inserted() && change.currentAuthor();
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

	if (!change.deleted()) {
		for (pos_type pos = 0; pos < size(); ++pos) {
			if (Inset * inset = getInset(pos))
				inset->setChange(change);
		}
	}
}


void Paragraph::setChange(pos_type pos, Change const & change)
{
	LASSERT(pos >= 0 && pos <= size(), return);
	d->changes_.set(change, pos);

	// see comment in setChange(Change const &) above
	if (!change.deleted() && pos < size())
			if (Inset * inset = getInset(pos))
				inset->setChange(change);
}


Change const & Paragraph::lookupChange(pos_type pos) const
{
	LBUFERR(pos >= 0 && pos <= size());
	return d->changes_.lookup(pos);
}


void Paragraph::acceptChanges(pos_type start, pos_type end)
{
	LASSERT(start >= 0 && start <= size(), return);
	LASSERT(end > start && end <= size() + 1, return);

	for (pos_type pos = start; pos < end; ++pos) {
		switch (lookupChange(pos).type) {
			case Change::UNCHANGED:
				// accept changes in nested inset
				if (Inset * inset = getInset(pos))
					inset->acceptChanges();
				break;

			case Change::INSERTED:
				d->changes_.set(Change(Change::UNCHANGED), pos);
				// also accept changes in nested inset
				if (Inset * inset = getInset(pos))
					inset->acceptChanges();
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


void Paragraph::rejectChanges(pos_type start, pos_type end)
{
	LASSERT(start >= 0 && start <= size(), return);
	LASSERT(end > start && end <= size() + 1, return);

	for (pos_type pos = start; pos < end; ++pos) {
		switch (lookupChange(pos).type) {
			case Change::UNCHANGED:
				// reject changes in nested inset
				if (Inset * inset = getInset(pos))
						inset->rejectChanges();
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


void Paragraph::Private::insertChar(pos_type pos, char_type c,
		Change const & change)
{
	LASSERT(pos >= 0 && pos <= int(text_.size()), return);

	// track change
	changes_.insert(change, pos);

	// This is actually very common when parsing buffers (and
	// maybe inserting ascii text)
	if (pos == pos_type(text_.size())) {
		// when appending characters, no need to update tables
		text_.push_back(c);
		// but we want spell checking
		requestSpellCheck(pos);
		return;
	}

	text_.insert(text_.begin() + pos, c);

	// Update the font table.
	fontlist_.increasePosAfterPos(pos);

	// Update the insets
	insetlist_.increasePosAfterPos(pos);

	// Update list of misspelled positions
	speller_state_.increasePosAfterPos(pos);
}


bool Paragraph::insertInset(pos_type pos, Inset * inset,
				   Font const & font, Change const & change)
{
	LASSERT(inset, return false);
	LASSERT(pos >= 0 && pos <= size(), return false);

	// Paragraph::insertInset() can be used in cut/copy/paste operation where
	// d->inset_owner_ is not set yet.
	if (d->inset_owner_ && !d->inset_owner_->insetAllowed(inset->lyxCode()))
		return false;

	d->insertChar(pos, META_INSET, change);
	LASSERT(d->text_[pos] == META_INSET, return false);

	// Add a new entry in the insetlist_.
	d->insetlist_.insert(inset, pos);

	// Some insets require run of spell checker
	requestSpellCheck(pos);
	setFont(pos, font);
	return true;
}


bool Paragraph::eraseChar(pos_type pos, bool trackChanges)
{
	LASSERT(pos >= 0 && pos <= size(), return false);

	// keep the logic here in sync with the logic of isMergedOnEndOfParDeletion()

	if (trackChanges) {
		Change change = d->changes_.lookup(pos);

		// set the character to DELETED if
		//  a) it was previously unchanged or
		//  b) it was inserted by a co-author

		if (!change.changed() ||
		      (change.inserted() && !change.currentAuthor())) {
			setChange(pos, Change(Change::DELETED));
			// request run of spell checker
			requestSpellCheck(pos);
			return false;
		}

		if (change.deleted())
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
	if (d->text_[pos] == META_INSET)
		d->insetlist_.erase(pos);

	d->text_.erase(d->text_.begin() + pos);

	// Update the fontlist_
	d->fontlist_.erase(pos);

	// Update the insetlist_
	d->insetlist_.decreasePosAfterPos(pos);

	// Update list of misspelled positions
	d->speller_state_.decreasePosAfterPos(pos);
	d->speller_state_.refreshLast(size());

	return true;
}


int Paragraph::eraseChars(pos_type start, pos_type end, bool trackChanges)
{
	LASSERT(start >= 0 && start <= size(), return 0);
	LASSERT(end >= start && end <= size() + 1, return 0);

	pos_type i = start;
	for (pos_type count = end - start; count; --count) {
		if (!eraseChar(i, trackChanges))
			++i;
	}
	return end - i;
}


int Paragraph::Private::latexSurrogatePair(otexstream & os, char_type c,
		char_type next, OutputParams const & runparams)
{
	// Writing next here may circumvent a possible font change between
	// c and next. Since next is only output if it forms a surrogate pair
	// with c we can ignore this:
	// A font change inside a surrogate pair does not make sense and is
	// hopefully impossible to input.
	// FIXME: change tracking
	// Is this correct WRT change tracking?
	Encoding const & encoding = *(runparams.encoding);
	docstring latex1 = encoding.latexChar(next).first;
	if (runparams.inIPA) {
		string const tipashortcut = Encodings::TIPAShortcut(next);
		if (!tipashortcut.empty()) {
			latex1 = from_ascii(tipashortcut);
		}
	}
	docstring const latex2 = encoding.latexChar(c).first;
	if (docstring(1, next) == latex1) {
		// the encoding supports the combination
		os << latex2 << latex1;
		return latex1.length() + latex2.length();
	} else if (runparams.local_font &&
		   runparams.local_font->language()->lang() == "polutonikogreek") {
		// polutonikogreek only works without the brackets
		os << latex1 << latex2;
		return latex1.length() + latex2.length();
	} else
		os << latex1 << '{' << latex2 << '}';
	return latex1.length() + latex2.length() + 2;
}


bool Paragraph::Private::simpleTeXBlanks(OutputParams const & runparams,
				       otexstream & os,
				       pos_type i,
				       unsigned int & column,
				       Font const & font,
				       Layout const & style)
{
	if (style.pass_thru || runparams.pass_thru)
		return false;

	if (i + 1 < int(text_.size())) {
		char_type next = text_[i + 1];
		if (Encodings::isCombiningChar(next)) {
			// This space has an accent, so we must always output it.
			column += latexSurrogatePair(os, ' ', next, runparams) - 1;
			return true;
		}
	}

	if (runparams.linelen > 0
	    && column > runparams.linelen
	    && i
	    && text_[i - 1] != ' '
	    && (i + 1 < int(text_.size()))
	    // same in FreeSpacing mode
	    && !owner_->isFreeSpacing()
	    // In typewriter mode, we want to avoid
	    // ! . ? : at the end of a line
	    && !(font.fontInfo().family() == TYPEWRITER_FAMILY
		 && (text_[i - 1] == '.'
		     || text_[i - 1] == '?'
		     || text_[i - 1] == ':'
		     || text_[i - 1] == '!'))) {
		os << '\n';
		os.texrow().start(owner_->id(), i + 1);
		column = 0;
	} else if (style.free_spacing) {
		os << '~';
	} else {
		os << ' ';
	}
	return false;
}


int Paragraph::Private::writeScriptChars(otexstream & os,
					 docstring const & ltx,
					 Change const & runningChange,
					 Encoding const & encoding,
					 pos_type & i)
{
	// FIXME: modifying i here is not very nice...

	// We only arrive here when a proper language for character text_[i] has
	// not been specified (i.e., it could not be translated in the current
	// latex encoding) or its latex translation has been forced, and it
	// belongs to a known script.
	// Parameter ltx contains the latex translation of text_[i] as specified
	// in the unicodesymbols file and is something like "\textXXX{<spec>}".
	// The latex macro name "textXXX" specifies the script to which text_[i]
	// belongs and we use it in order to check whether characters from the
	// same script immediately follow, such that we can collect them in a
	// single "\textXXX" macro. So, we have to retain "\textXXX{<spec>"
	// for the first char but only "<spec>" for all subsequent chars.
	docstring::size_type const brace1 = ltx.find_first_of(from_ascii("{"));
	docstring::size_type const brace2 = ltx.find_last_of(from_ascii("}"));
	string script = to_ascii(ltx.substr(1, brace1 - 1));
	int pos = 0;
	int length = brace2;
	bool closing_brace = true;
	if (script == "textgreek" && encoding.latexName() == "iso-8859-7") {
		// Correct encoding is being used, so we can avoid \textgreek.
		pos = brace1 + 1;
		length -= pos;
		closing_brace = false;
	}
	os << ltx.substr(pos, length);
	int size = text_.size();
	while (i + 1 < size) {
		char_type const next = text_[i + 1];
		// Stop here if next character belongs to another script
		// or there is a change in change tracking status.
		if (!Encodings::isKnownScriptChar(next, script) ||
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
		// Stop here if there is a font attribute or encoding change.
		if (found && cit != end && prev_font != cit->font())
			break;
		docstring const latex = encoding.latexChar(next).first;
		docstring::size_type const b1 =
					latex.find_first_of(from_ascii("{"));
		docstring::size_type const b2 =
					latex.find_last_of(from_ascii("}"));
		int const len = b2 - b1 - 1;
		os << latex.substr(b1 + 1, len);
		length += len;
		++i;
	}
	if (closing_brace) {
		os << '}';
		++length;
	}
	return length;
}


bool Paragraph::Private::isTextAt(string const & str, pos_type pos) const
{
	pos_type const len = str.length();

	// is the paragraph large enough?
	if (pos + len > int(text_.size()))
		return false;

	// does the wanted text start at point?
	for (string::size_type i = 0; i < str.length(); ++i) {
		// Caution: direct comparison of characters works only
		// because str is pure ASCII.
		if (str[i] != text_[pos + i])
			return false;
	}

	return fontlist_.hasChangeInRange(pos, len);
}


void Paragraph::Private::latexInset(BufferParams const & bparams,
				    otexstream & os,
				    OutputParams & runparams,
				    Font & running_font,
				    Font & basefont,
				    Font const & outerfont,
				    bool & open_font,
				    Change & running_change,
				    Layout const & style,
				    pos_type & i,
				    unsigned int & column)
{
	Inset * inset = owner_->getInset(i);
	LBUFERR(inset);

	if (style.pass_thru) {
		odocstringstream ods;
		inset->plaintext(ods, runparams);
		os << ods.str();
		return;
	}

	// FIXME: move this to InsetNewline::latex
	if (inset->lyxCode() == NEWLINE_CODE || inset->lyxCode() == SEPARATOR_CODE) {
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

			if (running_font.fontInfo().family() == TYPEWRITER_FAMILY)
				os << '~';

			basefont = owner_->getLayoutFont(bparams, outerfont);
			running_font = basefont;

			if (runparams.moving_arg)
				os << "\\protect ";

		}
		os.texrow().start(owner_->id(), i + 1);
		column = 0;
	}

	if (owner_->isDeleted(i)) {
		if( ++runparams.inDeletedInset == 1)
			runparams.changeOfDeletedInset = owner_->lookupChange(i);
	}

	if (inset->canTrackChanges()) {
		column += Changes::latexMarkChange(os, bparams, running_change,
			Change(Change::UNCHANGED), runparams);
		running_change = Change(Change::UNCHANGED);
	}

	bool close = false;
	odocstream::pos_type const len = os.os().tellp();

	if (inset->forceLTR()
	    && !runparams.use_polyglossia
	    && running_font.isRightToLeft()
	    // ERT is an exception, it should be output with no
	    // decorations at all
	    && inset->lyxCode() != ERT_CODE) {
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
	bool arabtex = basefont.language()->lang() == "arabic_arabtex"
		|| running_font.language()->lang() == "arabic_arabtex";
	if (open_font && !inset->inheritFont()) {
		bool closeLanguage = arabtex
			|| basefont.isRightToLeft() == running_font.isRightToLeft();
		unsigned int count = running_font.latexWriteEndChanges(os,
			bparams, runparams, basefont, basefont, closeLanguage);
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

	int prev_rows = os.texrow().rows();

	try {
		runparams.lastid = id_;
		runparams.lastpos = i;
		inset->latex(os, runparams);
	} catch (EncodingException & e) {
		// add location information and throw again.
		e.par_id = id_;
		e.pos = i;
		throw(e);
	}

	if (close) {
		if (running_font.language()->lang() == "farsi")
				os << "\\endL{}";
			else
				os << '}';
	}

	if (os.texrow().rows() > prev_rows) {
		os.texrow().start(owner_->id(), i + 1);
		column = 0;
	} else {
		column += (unsigned int)(os.os().tellp() - len);
	}

	if (owner_->isDeleted(i))
		--runparams.inDeletedInset;
}


void Paragraph::Private::latexSpecialChar(otexstream & os,
					  BufferParams const & bparams,
					  OutputParams const & runparams,
					  Font const & running_font,
					  Change const & running_change,
					  Layout const & style,
					  pos_type & i,
					  pos_type end_pos,
					  unsigned int & column)
{
	// With polyglossia, brackets and stuff need not be reversed
	// in RTL scripts (see bug #8251)
	char_type const c = (runparams.use_polyglossia) ?
		owner_->getUChar(bparams, i) : text_[i];

	if (style.pass_thru || runparams.pass_thru) {
		if (c != '\0') {
			Encoding const * const enc = runparams.encoding;
			if (enc && !enc->encodable(c))
				throw EncodingException(c);
			os.put(c);
		}
		return;
	}

	// TIPA uses its own T3 encoding
	if (runparams.inIPA && latexSpecialT3(c, os, i, column))
		return;
	// If T1 font encoding is used, use the special
	// characters it provides.
	// NOTE: some languages reset the font encoding
	// internally
	if (!runparams.inIPA && !running_font.language()->internalFontEncoding()
	    && lyxrc.fontenc == "T1" && latexSpecialT1(c, os, i, column))
		return;

	// Otherwise, we use what LaTeX provides us.
	switch (c) {
	case '\\':
		os << "\\textbackslash{}";
		column += 15;
		break;
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
	case '-':
		os << '-';
		if (i + 1 < end_pos && text_[i+1] == '-') {
			// Prevent "--" becoming an endash and "---" becoming
			// an emdash.
			// Within \ttfamily, "--" is merged to "-" (no endash)
			// so we avoid this rather irritating ligature as well
			os << "{}";
			column += 2;
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

	case '*':
	case '[':
	case ']':
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
		// LyX, LaTeX etc.
		if (latexSpecialPhrase(os, i, end_pos, column, runparams))
			return;

		if (c == '\0')
			return;

		Encoding const & encoding = *(runparams.encoding);
		char_type next = '\0';
		if (i + 1 < int(text_.size())) {
			next = text_[i + 1];
			if (Encodings::isCombiningChar(next)) {
				column += latexSurrogatePair(os, c, next, runparams) - 1;
				++i;
				break;
			}
		}
		string script;
		pair<docstring, bool> latex = encoding.latexChar(c);
		docstring nextlatex;
		bool nexttipas = false;
		string nexttipashortcut;
		if (next != '\0' && next != META_INSET && encoding.encodable(next)) {
			nextlatex = encoding.latexChar(next).first;
			if (runparams.inIPA) {
				nexttipashortcut = Encodings::TIPAShortcut(next);
				nexttipas = !nexttipashortcut.empty();
			}
		}
		bool tipas = false;
		if (runparams.inIPA) {
			string const tipashortcut = Encodings::TIPAShortcut(c);
			if (!tipashortcut.empty()) {
				latex.first = from_ascii(tipashortcut);
				latex.second = false;
				tipas = true;
			}
		}
		if (Encodings::isKnownScriptChar(c, script)
		    && prefixIs(latex.first, from_ascii("\\" + script)))
			column += writeScriptChars(os, latex.first,
					running_change, encoding, i) - 1;
		else if (latex.second
			 && ((!prefixIs(nextlatex, '\\')
			       && !prefixIs(nextlatex, '{')
			       && !prefixIs(nextlatex, '}'))
			     || (nexttipas
			         && !prefixIs(from_ascii(nexttipashortcut), '\\')))
			 && !tipas) {
			// Prevent eating of a following
			// space or command corruption by
			// following characters
			if (next == ' ' || next == '\0') {
				column += latex.first.length() + 1;
				os << latex.first << "{}";
			} else {
				column += latex.first.length();
				os << latex.first << " ";
			}
		} else {
			column += latex.first.length() - 1;
			os << latex.first;
		}
		break;
	}
}


bool Paragraph::Private::latexSpecialT1(char_type const c, otexstream & os,
	pos_type i, unsigned int & column)
{
	switch (c) {
	case '>':
	case '<':
		os.put(c);
		// In T1 encoding, these characters exist
		// but we should avoid ligatures
		if (i + 1 >= int(text_.size()) || text_[i + 1] != c)
			return true;
		os << "\\textcompwordmark{}";
		column += 19;
		return true;
	case '|':
		os.put(c);
		return true;
	case '\"':
		// soul.sty breaks with \char`\"
		os << "\\textquotedbl{}";
		column += 14;
		return true;
	default:
		return false;
	}
}


bool Paragraph::Private::latexSpecialT3(char_type const c, otexstream & os,
	pos_type /*i*/, unsigned int & column)
{
	switch (c) {
	case '*':
	case '[':
	case ']':
	case '\"':
		os.put(c);
		return true;
	case '|':
		os << "\\textvertline{}";
		column += 14;
		return true;
	default:
		return false;
	}
}


/// \param end_pos
///   If [start_pos, end_pos) does not include entirely the special phrase, then
///   do not apply the macro transformation.
bool Paragraph::Private::latexSpecialPhrase(otexstream & os, pos_type & i, pos_type end_pos,
	unsigned int & column, OutputParams const & runparams)
{
	// FIXME: if we have "LaTeX" with a font
	// change in the middle (before the 'T', then
	// the "TeX" part is still special cased.
	// Really we should only operate this on
	// "words" for some definition of word

	for (size_t pnr = 0; pnr < phrases_nr; ++pnr) {
		if (!isTextAt(special_phrases[pnr].phrase, i)
		    || (end_pos != -1 && i + int(special_phrases[pnr].phrase.size()) > end_pos))
			continue;
		if (runparams.moving_arg)
			os << "\\protect";
		os << special_phrases[pnr].macro;
		i += special_phrases[pnr].phrase.length() - 1;
		column += special_phrases[pnr].macro.length() - 1;
		return true;
	}
	return false;
}


void Paragraph::Private::validate(LaTeXFeatures & features) const
{
	if (layout_->inpreamble && inset_owner_) {
		bool const is_command = layout_->latextype == LATEX_COMMAND;
		Buffer const & buf = inset_owner_->buffer();
		BufferParams const & bp = features.runparams().is_child
			? buf.masterParams() : buf.params();
		Font f;
		TexRow texrow;
		// Using a string stream here circumvents the encoding
		// switching machinery of odocstream. Therefore the
		// output is wrong if this paragraph contains content
		// that needs to switch encoding.
		odocstringstream ods;
		otexstream os(ods, texrow);
		if (is_command) {
			os << '\\' << from_ascii(layout_->latexname());
			// we have to provide all the optional arguments here, even though
			// the last one is the only one we care about.
			// Separate handling of optional argument inset.
			if (!layout_->latexargs().empty()) {
				OutputParams rp = features.runparams();
				rp.local_font = &owner_->getFirstFontSettings(bp);
				latexArgInsets(*owner_, os, rp, layout_->latexargs());
			}
			os << from_ascii(layout_->latexparam());
		}
		docstring::size_type const length = ods.str().length();
		// this will output "{" at the beginning, but not at the end
		owner_->latex(bp, f, os, features.runparams(), 0, -1, true);
		if (ods.str().length() > length) {
			if (is_command) {
				ods << '}';
				if (!layout_->postcommandargs().empty()) {
					OutputParams rp = features.runparams();
					rp.local_font = &owner_->getFirstFontSettings(bp);
					latexArgInsets(*owner_, os, rp, layout_->postcommandargs(), "post:");
				}
			}
			string const snippet = to_utf8(ods.str());
			features.addPreambleSnippet(snippet);
		}
	}

	if (features.runparams().flavor == OutputParams::HTML
	    && layout_->htmltitle()) {
		features.setHTMLTitle(owner_->asString(AS_STR_INSETS | AS_STR_SKIPDELETE));
	}

	// check the params.
	if (!params_.spacing().isDefault())
		features.require("setspace");

	// then the layouts
	features.useLayout(layout_->name());

	// then the fonts
	fontlist_.validate(features);

	// then the indentation
	if (!params_.leftIndent().zero())
		features.require("ParagraphLeftIndent");

	// then the insets
	InsetList::const_iterator icit = insetlist_.begin();
	InsetList::const_iterator iend = insetlist_.end();
	for (; icit != iend; ++icit) {
		if (icit->inset) {
			icit->inset->validate(features);
			if (layout_->needprotect &&
			    icit->inset->lyxCode() == FOOT_CODE)
				features.require("footmisc");
		}
	}

	// then the contents
	for (pos_type i = 0; i < int(text_.size()) ; ++i) {
		for (size_t pnr = 0; pnr < phrases_nr; ++pnr) {
			if (!special_phrases[pnr].builtin
			    && isTextAt(special_phrases[pnr].phrase, i)) {
				features.require(special_phrases[pnr].phrase);
				break;
			}
		}
		BufferEncodings::validate(text_[i], features);
	}
}

/////////////////////////////////////////////////////////////////////
//
// Paragraph
//
/////////////////////////////////////////////////////////////////////

namespace {
	Layout const emptyParagraphLayout;
}

Paragraph::Paragraph()
	: d(new Paragraph::Private(this, emptyParagraphLayout))
{
	itemdepth = 0;
	d->params_.clear();
}


Paragraph::Paragraph(Paragraph const & par)
	: itemdepth(par.itemdepth),
	d(new Paragraph::Private(*par.d, this))
{
	registerWords();
}


Paragraph::Paragraph(Paragraph const & par, pos_type beg, pos_type end)
	: itemdepth(par.itemdepth),
	d(new Paragraph::Private(*par.d, this, beg, end))
{
	registerWords();
}


Paragraph & Paragraph::operator=(Paragraph const & par)
{
	// needed as we will destroy the private part before copying it
	if (&par != this) {
		itemdepth = par.itemdepth;

		deregisterWords();
		delete d;
		d = new Private(*par.d, this);
		registerWords();
	}
	return *this;
}


Paragraph::~Paragraph()
{
	deregisterWords();
	delete d;
}


namespace {

// this shall be called just before every "os << ..." action.
void flushString(ostream & os, docstring & s)
{
	os << to_utf8(s);
	s.erase();
}

}


void Paragraph::write(ostream & os, BufferParams const & bparams,
	depth_type & dth) const
{
	// The beginning or end of a deeper (i.e. nested) area?
	if (dth != d->params_.depth()) {
		if (d->params_.depth() > dth) {
			while (d->params_.depth() > dth) {
				os << "\n\\begin_deeper";
				++dth;
			}
		} else {
			while (d->params_.depth() < dth) {
				os << "\n\\end_deeper";
				--dth;
			}
		}
	}

	// First write the layout
	os << "\n\\begin_layout " << to_utf8(d->layout_->name()) << '\n';

	d->params_.write(os);

	Font font1(inherit_font, bparams.language);

	Change running_change = Change(Change::UNCHANGED);

	// this string is used as a buffer to avoid repetitive calls
	// to to_utf8(), which turn out to be expensive (JMarc)
	docstring write_buffer;

	int column = 0;
	for (pos_type i = 0; i <= size(); ++i) {

		Change const & change = lookupChange(i);
		if (change != running_change)
			flushString(os, write_buffer);
		Changes::lyxMarkChange(os, bparams, column, running_change, change);
		running_change = change;

		if (i == size())
			break;

		// Write font changes
		Font font2 = getFontSettings(bparams, i);
		if (font2 != font1) {
			flushString(os, write_buffer);
			font2.lyxWriteChanges(font1, os);
			column = 0;
			font1 = font2;
		}

		char_type const c = d->text_[i];
		switch (c) {
		case META_INSET:
			if (Inset const * inset = getInset(i)) {
				flushString(os, write_buffer);
				if (inset->directWrite()) {
					// international char, let it write
					// code directly so it's shorter in
					// the file
					inset->write(os);
				} else {
					if (i)
						os << '\n';
					os << "\\begin_inset ";
					inset->write(os);
					os << "\n\\end_inset\n\n";
					column = 0;
				}
				// FIXME This can be removed again once the mystery
				// crash has been resolved.
				os << flush;
			}
			break;
		case '\\':
			flushString(os, write_buffer);
			os << "\n\\backslash\n";
			column = 0;
			break;
		case '.':
			flushString(os, write_buffer);
			if (i + 1 < size() && d->text_[i + 1] == ' ') {
				os << ".\n";
				column = 0;
			} else
				os << '.';
			break;
		default:
			if ((column > 70 && c == ' ')
			    || column > 79) {
				flushString(os, write_buffer);
				os << '\n';
				column = 0;
			}
			// this check is to amend a bug. LyX sometimes
			// inserts '\0' this could cause problems.
			if (c != '\0')
				write_buffer.push_back(c);
			else
				LYXERR0("NUL char in structure.");
			++column;
			break;
		}
	}

	flushString(os, write_buffer);
	os << "\n\\end_layout\n";
	// FIXME This can be removed again once the mystery
	// crash has been resolved.
	os << flush;
}


void Paragraph::validate(LaTeXFeatures & features) const
{
	d->validate(features);
}


void Paragraph::insert(pos_type start, docstring const & str,
		       Font const & font, Change const & change)
{
	for (size_t i = 0, n = str.size(); i != n ; ++i)
		insertChar(start + i, str[i], font, change);
}


void Paragraph::appendChar(char_type c, Font const & font,
		Change const & change)
{
	// track change
	d->changes_.insert(change, d->text_.size());
	// when appending characters, no need to update tables
	d->text_.push_back(c);
	setFont(d->text_.size() - 1, font);
	d->requestSpellCheck(d->text_.size() - 1);
}


void Paragraph::appendString(docstring const & s, Font const & font,
		Change const & change)
{
	pos_type end = s.size();
	size_t oldsize = d->text_.size();
	size_t newsize = oldsize + end;
	size_t capacity = d->text_.capacity();
	if (newsize >= capacity)
		d->text_.reserve(max(capacity + 100, newsize));

	// when appending characters, no need to update tables
	d->text_.append(s);

	// FIXME: Optimize this!
	for (size_t i = oldsize; i != newsize; ++i) {
		// track change
		d->changes_.insert(change, i);
		d->requestSpellCheck(i);
	}
	d->fontlist_.set(oldsize, font);
	d->fontlist_.set(newsize - 1, font);
}


void Paragraph::insertChar(pos_type pos, char_type c,
			   bool trackChanges)
{
	d->insertChar(pos, c, Change(trackChanges ?
			   Change::INSERTED : Change::UNCHANGED));
}


void Paragraph::insertChar(pos_type pos, char_type c,
			   Font const & font, bool trackChanges)
{
	d->insertChar(pos, c, Change(trackChanges ?
			   Change::INSERTED : Change::UNCHANGED));
	setFont(pos, font);
}


void Paragraph::insertChar(pos_type pos, char_type c,
			   Font const & font, Change const & change)
{
	d->insertChar(pos, c, change);
	setFont(pos, font);
}


void Paragraph::resetFonts(Font const & font)
{
	d->fontlist_.clear();
	d->fontlist_.set(0, font);
	d->fontlist_.set(d->text_.size() - 1, font);
}

// Gets uninstantiated font setting at position.
Font const & Paragraph::getFontSettings(BufferParams const & bparams,
					 pos_type pos) const
{
	if (pos > size()) {
		LYXERR0("pos: " << pos << " size: " << size());
		LBUFERR(false);
	}

	FontList::const_iterator cit = d->fontlist_.fontIterator(pos);
	if (cit != d->fontlist_.end())
		return cit->font();

	if (pos == size() && !empty())
		return getFontSettings(bparams, pos - 1);

	// Optimisation: avoid a full font instantiation if there is no
	// language change from previous call.
	static Font previous_font;
	static Language const * previous_lang = 0;
	Language const * lang = getParLanguage(bparams);
	if (lang != previous_lang) {
		previous_lang = lang;
		previous_font = Font(inherit_font, lang);
	}
	return previous_font;
}


FontSpan Paragraph::fontSpan(pos_type pos) const
{
	LBUFERR(pos < size());

	pos_type start = 0;
	FontList::const_iterator cit = d->fontlist_.begin();
	FontList::const_iterator end = d->fontlist_.end();
	for (; cit != end; ++cit) {
		if (cit->pos() >= pos) {
			if (pos >= beginOfBody())
				return FontSpan(max(start, beginOfBody()),
						cit->pos());
			else
				return FontSpan(start,
						min(beginOfBody() - 1,
							 cit->pos()));
		}
		start = cit->pos() + 1;
	}

	// This should not happen, but if so, we take no chances.
	LYXERR0("Paragraph::fontSpan: position not found in fontinfo table!");
	LASSERT(false, return FontSpan(pos, pos));
}


// Gets uninstantiated font setting at position 0
Font const & Paragraph::getFirstFontSettings(BufferParams const & bparams) const
{
	if (!empty() && !d->fontlist_.empty())
		return d->fontlist_.begin()->font();

	// Optimisation: avoid a full font instantiation if there is no
	// language change from previous call.
	static Font previous_font;
	static Language const * previous_lang = 0;
	if (bparams.language != previous_lang) {
		previous_lang = bparams.language;
		previous_font = Font(inherit_font, bparams.language);
	}

	return previous_font;
}


// Gets the fully instantiated font at a given position in a paragraph
// This is basically the same function as Text::GetFont() in text2.cpp.
// The difference is that this one is used for generating the LaTeX file,
// and thus cosmetic "improvements" are disallowed: This has to deliver
// the true picture of the buffer. (Asger)
Font const Paragraph::getFont(BufferParams const & bparams, pos_type pos,
				 Font const & outerfont) const
{
	LBUFERR(pos >= 0);

	Font font = getFontSettings(bparams, pos);

	pos_type const body_pos = beginOfBody();
	FontInfo & fi = font.fontInfo();
	if (pos < body_pos)
		fi.realize(d->layout_->labelfont);
	else
		fi.realize(d->layout_->font);

	fi.realize(outerfont.fontInfo());
	fi.realize(bparams.getFont().fontInfo());

	return font;
}


Font const Paragraph::getLabelFont
	(BufferParams const & bparams, Font const & outerfont) const
{
	FontInfo tmpfont = d->layout_->labelfont;
	tmpfont.realize(outerfont.fontInfo());
	tmpfont.realize(bparams.getFont().fontInfo());
	return Font(tmpfont, getParLanguage(bparams));
}


Font const Paragraph::getLayoutFont
	(BufferParams const & bparams, Font const & outerfont) const
{
	FontInfo tmpfont = d->layout_->font;
	tmpfont.realize(outerfont.fontInfo());
	tmpfont.realize(bparams.getFont().fontInfo());
	return Font(tmpfont, getParLanguage(bparams));
}


/// Returns the height of the highest font in range
FontSize Paragraph::highestFontInRange
	(pos_type startpos, pos_type endpos, FontSize def_size) const
{
	return d->fontlist_.highestInRange(startpos, endpos, def_size);
}


char_type Paragraph::getUChar(BufferParams const & bparams, pos_type pos) const
{
	char_type c = d->text_[pos];
	if (!getFontSettings(bparams, pos).isRightToLeft())
		return c;

	// FIXME: The arabic special casing is due to the difference of arabic
	// round brackets input introduced in r18599. Check if this should be
	// unified with Hebrew or at least if all bracket types should be
	// handled the same (file format change in either case).
	string const & lang = getFontSettings(bparams, pos).language()->lang();
	bool const arabic = lang == "arabic_arabtex" || lang == "arabic_arabi"
		|| lang == "farsi";
	char_type uc = c;
	switch (c) {
	case '(':
		uc = arabic ? c : ')';
		break;
	case ')':
		uc = arabic ? c : '(';
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

	return uc;
}


void Paragraph::setFont(pos_type pos, Font const & font)
{
	LASSERT(pos <= size(), return);

	// First, reduce font against layout/label font
	// Update: The setCharFont() routine in text2.cpp already
	// reduces font, so we don't need to do that here. (Asger)

	d->fontlist_.set(pos, font);
}


void Paragraph::makeSameLayout(Paragraph const & par)
{
	d->layout_ = par.d->layout_;
	d->params_ = par.d->params_;
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
	return par.d->layout_ == d->layout_
		&& d->params_.sameLayout(par.d->params_);
}


depth_type Paragraph::getDepth() const
{
	return d->params_.depth();
}


depth_type Paragraph::getMaxDepthAfter() const
{
	if (d->layout_->isEnvironment())
		return d->params_.depth() + 1;
	else
		return d->params_.depth();
}


char Paragraph::getAlign() const
{
	if (d->params_.align() == LYX_ALIGN_LAYOUT)
		return d->layout_->align;
	else
		return d->params_.align();
}


docstring const & Paragraph::labelString() const
{
	return d->params_.labelString();
}


// the next two functions are for the manual labels
docstring const Paragraph::getLabelWidthString() const
{
	if (d->layout_->margintype == MARGIN_MANUAL
	    || d->layout_->latextype == LATEX_BIB_ENVIRONMENT)
		return d->params_.labelWidthString();
	else
		return _("Senseless with this layout!");
}


void Paragraph::setLabelWidthString(docstring const & s)
{
	d->params_.labelWidthString(s);
}


docstring Paragraph::expandLabel(Layout const & layout,
		BufferParams const & bparams) const
{
	return expandParagraphLabel(layout, bparams, true);
}


docstring Paragraph::expandDocBookLabel(Layout const & layout,
		BufferParams const & bparams) const
{
	return expandParagraphLabel(layout, bparams, false);
}


docstring Paragraph::expandParagraphLabel(Layout const & layout,
		BufferParams const & bparams, bool process_appendix) const
{
	DocumentClass const & tclass = bparams.documentClass();
	string const & lang = getParLanguage(bparams)->code();
	bool const in_appendix = process_appendix && d->params_.appendix();
	docstring fmt = translateIfPossible(layout.labelstring(in_appendix), lang);

	if (fmt.empty() && !layout.counter.empty())
		return tclass.counters().theCounter(layout.counter, lang);

	// handle 'inherited level parts' in 'fmt',
	// i.e. the stuff between '@' in   '@Section@.\arabic{subsection}'
	size_t const i = fmt.find('@', 0);
	if (i != docstring::npos) {
		size_t const j = fmt.find('@', i + 1);
		if (j != docstring::npos) {
			docstring parent(fmt, i + 1, j - i - 1);
			docstring label = from_ascii("??");
			if (tclass.hasLayout(parent))
				docstring label = expandParagraphLabel(tclass[parent], bparams,
						      process_appendix);
			fmt = docstring(fmt, 0, i) + label
				+ docstring(fmt, j + 1, docstring::npos);
		}
	}

	return tclass.counters().counterLabel(fmt, lang);
}


void Paragraph::applyLayout(Layout const & new_layout)
{
	d->layout_ = &new_layout;
	LyXAlignment const oldAlign = d->params_.align();

	if (!(oldAlign & d->layout_->alignpossible)) {
		frontend::Alert::warning(_("Alignment not permitted"),
			_("The new layout does not permit the alignment previously used.\nSetting to default."));
		d->params_.align(LYX_ALIGN_LAYOUT);
	}
}


pos_type Paragraph::beginOfBody() const
{
	return d->begin_of_body_;
}


void Paragraph::setBeginOfBody()
{
	if (d->layout_->labeltype != LABEL_MANUAL) {
		d->begin_of_body_ = 0;
		return;
	}

	// Unroll the first two cycles of the loop
	// and remember the previous character to
	// remove unnecessary getChar() calls
	pos_type i = 0;
	pos_type end = size();
	if (i < end && !(isNewline(i) || isEnvSeparator(i))) {
		++i;
		char_type previous_char = 0;
		char_type temp = 0;
		if (i < end) {
			previous_char = d->text_[i];
			if (!(isNewline(i) || isEnvSeparator(i))) {
				++i;
				while (i < end && previous_char != ' ') {
					temp = d->text_[i];
					if (isNewline(i) || isEnvSeparator(i))
						break;
					++i;
					previous_char = temp;
				}
			}
		}
	}

	d->begin_of_body_ = i;
}


bool Paragraph::allowParagraphCustomization() const
{
	return inInset().allowParagraphCustomization();
}


bool Paragraph::usePlainLayout() const
{
	return inInset().usePlainLayout();
}


bool Paragraph::isPassThru() const
{
	return inInset().isPassThru() || d->layout_->pass_thru;
}

namespace {

// paragraphs inside floats need different alignment tags to avoid
// unwanted space

bool noTrivlistCentering(InsetCode code)
{
	return code == FLOAT_CODE
	       || code == WRAP_CODE
	       || code == CELL_CODE;
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


bool corrected_env(otexstream & os, string const & suffix, string const & env,
	InsetCode code, bool const lastpar, int & col)
{
	string macro = suffix + "{";
	if (noTrivlistCentering(code)) {
		if (lastpar) {
			// the last paragraph in non-trivlist-aligned
			// context is special (to avoid unwanted whitespace)
			if (suffix == "\\begin") {
				macro = "\\" + correction(env) + "{}";
				os << from_ascii(macro);
				col += macro.size();
				return true;
			}
			return false;
		}
		macro += correction(env);
	} else
		macro += env;
	macro += "}";
	if (suffix == "\\par\\end") {
		os << breakln;
		col = 0;
	}
	os << from_ascii(macro);
	col += macro.size();
	if (suffix == "\\begin") {
		os << breakln;
		col = 0;
	}
	return true;
}

} // namespace anon


int Paragraph::Private::startTeXParParams(BufferParams const & bparams,
			otexstream & os, OutputParams const & runparams) const
{
	int column = 0;

	if (params_.noindent() && !layout_->pass_thru
	    && (layout_->toggle_indent != ITOGGLE_NEVER)) {
		os << "\\noindent ";
		column += 10;
	}

	LyXAlignment const curAlign = params_.align();

	if (curAlign == layout_->align)
		return column;

	switch (curAlign) {
	case LYX_ALIGN_NONE:
	case LYX_ALIGN_BLOCK:
	case LYX_ALIGN_LAYOUT:
	case LYX_ALIGN_SPECIAL:
	case LYX_ALIGN_DECIMAL:
		break;
	case LYX_ALIGN_LEFT:
	case LYX_ALIGN_RIGHT:
	case LYX_ALIGN_CENTER:
		if (runparams.moving_arg) {
			os << "\\protect";
			column += 8;
		}
		break;
	}

	string const begin_tag = "\\begin";
	InsetCode code = ownerCode();
	bool const lastpar = runparams.isLastPar;

	switch (curAlign) {
	case LYX_ALIGN_NONE:
	case LYX_ALIGN_BLOCK:
	case LYX_ALIGN_LAYOUT:
	case LYX_ALIGN_SPECIAL:
	case LYX_ALIGN_DECIMAL:
		break;
	case LYX_ALIGN_LEFT: {
		if (owner_->getParLanguage(bparams)->babel() != "hebrew")
			corrected_env(os, begin_tag, "flushleft", code, lastpar, column);
		else
			corrected_env(os, begin_tag, "flushright", code, lastpar, column);
		break;
	} case LYX_ALIGN_RIGHT: {
		string output;
		if (owner_->getParLanguage(bparams)->babel() != "hebrew")
			corrected_env(os, begin_tag, "flushright", code, lastpar, column);
		else
			corrected_env(os, begin_tag, "flushleft", code, lastpar, column);
		break;
	} case LYX_ALIGN_CENTER: {
		corrected_env(os, begin_tag, "center", code, lastpar, column);
		break;
	}
	}

	return column;
}


bool Paragraph::Private::endTeXParParams(BufferParams const & bparams,
			otexstream & os, OutputParams const & runparams) const
{
	LyXAlignment const curAlign = params_.align();

	if (curAlign == layout_->align)
		return false;

	switch (curAlign) {
	case LYX_ALIGN_NONE:
	case LYX_ALIGN_BLOCK:
	case LYX_ALIGN_LAYOUT:
	case LYX_ALIGN_SPECIAL:
	case LYX_ALIGN_DECIMAL:
		break;
	case LYX_ALIGN_LEFT:
	case LYX_ALIGN_RIGHT:
	case LYX_ALIGN_CENTER:
		if (runparams.moving_arg)
			os << "\\protect";
		break;
	}

	bool output = false;
	int col = 0;
	string const end_tag = "\\par\\end";
	InsetCode code = ownerCode();
	bool const lastpar = runparams.isLastPar;

	switch (curAlign) {
	case LYX_ALIGN_NONE:
	case LYX_ALIGN_BLOCK:
	case LYX_ALIGN_LAYOUT:
	case LYX_ALIGN_SPECIAL:
	case LYX_ALIGN_DECIMAL:
		break;
	case LYX_ALIGN_LEFT: {
		if (owner_->getParLanguage(bparams)->babel() != "hebrew")
			output = corrected_env(os, end_tag, "flushleft", code, lastpar, col);
		else
			output = corrected_env(os, end_tag, "flushright", code, lastpar, col);
		break;
	} case LYX_ALIGN_RIGHT: {
		if (owner_->getParLanguage(bparams)->babel() != "hebrew")
			output = corrected_env(os, end_tag, "flushright", code, lastpar, col);
		else
			output = corrected_env(os, end_tag, "flushleft", code, lastpar, col);
		break;
	} case LYX_ALIGN_CENTER: {
		corrected_env(os, end_tag, "center", code, lastpar, col);
		break;
	}
	}

	return output || lastpar;
}


// This one spits out the text of the paragraph
void Paragraph::latex(BufferParams const & bparams,
	Font const & outerfont,
	otexstream & os,
	OutputParams const & runparams,
	int start_pos, int end_pos, bool force) const
{
	LYXERR(Debug::LATEX, "Paragraph::latex...     " << this);

	// FIXME This check should not be needed. Perhaps issue an
	// error if it triggers.
	Layout const & style = inInset().forcePlainLayout() ?
		bparams.documentClass().plainLayout() : *d->layout_;

	if (!force && style.inpreamble)
		return;

	bool const allowcust = allowParagraphCustomization();

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
		// This is not strictly needed, but if this is changed it
		// would be a file format change, and tex2lyx would need
		// to be adjusted, since it unconditionally removes the
		// braces when it parses \item.
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

	Encoding const * const prev_encoding = runparams.encoding;

	os.texrow().start(id(), 0);

	// if the paragraph is empty, the loop will not be entered at all
	if (empty()) {
		if (style.isCommand()) {
			os << '{';
			++column;
		}
		if (!style.leftdelim().empty()) {
			os << style.leftdelim();
			column += style.leftdelim().size();
		}
		if (allowcust)
			column += d->startTeXParParams(bparams, os, runparams);
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
						runningChange, Change(Change::UNCHANGED),
						runparams);
				runningChange = Change(Change::UNCHANGED);

				os << "}] ";
				column +=3;
			}
			if (style.isCommand()) {
				os << '{';
				++column;
			}

			if (!style.leftdelim().empty()) {
				os << style.leftdelim();
				column += style.leftdelim().size();
			}

			if (allowcust)
				column += d->startTeXParParams(bparams, os,
							    runparams);
		}

		Change const & change = runparams.inDeletedInset
			? runparams.changeOfDeletedInset : lookupChange(i);

		if (bparams.output_changes && runningChange != change) {
			if (open_font) {
				column += running_font.latexWriteEndChanges(
						os, bparams, runparams, basefont, basefont);
				open_font = false;
			}
			basefont = getLayoutFont(bparams, outerfont);
			running_font = basefont;

			column += Changes::latexMarkChange(os, bparams, runningChange,
							   change, runparams);
			runningChange = change;
		}

		// do not output text which is marked deleted
		// if change tracking output is disabled
		if (!bparams.output_changes && change.deleted()) {
			continue;
		}

		++column;

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

		string const running_lang = runparams.use_polyglossia ?
			running_font.language()->polyglossia() : running_font.language()->babel();
		// close babel's font environment before opening CJK.
		string const lang_end_command = runparams.use_polyglossia ?
			"\\end{$$lang}" : lyxrc.language_command_end;
		if (!running_lang.empty() &&
		    font.language()->encoding()->package() == Encoding::CJK) {
				string end_tag = subst(lang_end_command,
							"$$lang",
							running_lang);
				os << from_ascii(end_tag);
				column += end_tag.length();
		}

		// Switch file encoding if necessary (and allowed)
		if (!runparams.pass_thru && !style.pass_thru &&
		    runparams.encoding->package() != Encoding::none &&
		    font.language()->encoding()->package() != Encoding::none) {
			pair<bool, int> const enc_switch =
			    	switchEncoding(os.os(), bparams, runparams,
					*(font.language()->encoding()));
			if (enc_switch.first) {
				column += enc_switch.second;
				runparams.encoding = font.language()->encoding();
			}
		}

		char_type const c = d->text_[i];

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
			// check whether the fontchange ends with a \\textcolor
			// modifier and the text starts with a space (bug 4473)
			docstring const last_modifier = rsplit(fontchange, '\\');
			if (prefixIs(last_modifier, from_ascii("textcolor")) && c == ' ')
				os << fontchange << from_ascii("{}");
			// check if the fontchange ends with a trailing blank
			// (like "\small " (see bug 3382)
			else if (suffixIs(fontchange, ' ') && c == ' ')
				os << fontchange.substr(0, fontchange.size() - 1)
				   << from_ascii("{}");
			else
				os << fontchange;
		}

		// FIXME: think about end_pos implementation...
		if (c == ' ' && i >= start_pos && (end_pos == -1 || i < end_pos)) {
			// FIXME: integrate this case in latexSpecialChar
			// Do not print the separation of the optional argument
			// if style.pass_thru is false. This works because
			// latexSpecialChar ignores spaces if
			// style.pass_thru is false.
			if (i != body_pos - 1) {
				if (d->simpleTeXBlanks(runparams, os,
						i, column, font, style)) {
					// A surrogate pair was output. We
					// must not call latexSpecialChar
					// in this iteration, since it would output
					// the combining character again.
					++i;
					continue;
				}
			}
		}

		OutputParams rp = runparams;
		rp.free_spacing = style.free_spacing;
		rp.local_font = &font;
		rp.intitle = style.intitle;

		// Two major modes:  LaTeX or plain
		// Handle here those cases common to both modes
		// and then split to handle the two modes separately.
		if (c == META_INSET) {
			if (i >= start_pos && (end_pos == -1 || i < end_pos)) {
				d->latexInset(bparams, os, rp, running_font,
						basefont, outerfont, open_font,
						runningChange, style, i, column);
			}
		} else {
			if (i >= start_pos && (end_pos == -1 || i < end_pos)) {
				try {
					d->latexSpecialChar(os, bparams, rp, running_font, runningChange,
							    style, i, end_pos, column);
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
		}
		}

		// Set the encoding to that returned from latexSpecialChar (see
		// comment for encoding member in OutputParams.h)
		runparams.encoding = rp.encoding;
	}

	// If we have an open font definition, we have to close it
	if (open_font) {
#ifdef FIXED_LANGUAGE_END_DETECTION
		if (next_) {
			running_font.latexWriteEndChanges(os, bparams,
					runparams, basefont,
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

	column += Changes::latexMarkChange(os, bparams, runningChange,
					   Change(Change::UNCHANGED), runparams);

	// Needed if there is an optional argument but no contents.
	if (body_pos > 0 && body_pos == size()) {
		os << "}]~";
	}

	if (!style.rightdelim().empty()) {
		os << style.rightdelim();
		column += style.rightdelim().size();
	}

	if (allowcust && d->endTeXParParams(bparams, os, runparams)
	    && runparams.encoding != prev_encoding) {
		runparams.encoding = prev_encoding;
		if (!runparams.isFullUnicode())
			os << setEncoding(prev_encoding->iconvName());
	}

	LYXERR(Debug::LATEX, "Paragraph::latex... done " << this);
}


bool Paragraph::emptyTag() const
{
	for (pos_type i = 0; i < size(); ++i) {
		if (Inset const * inset = getInset(i)) {
			InsetCode lyx_code = inset->lyxCode();
			// FIXME testing like that is wrong. What is
			// the intent?
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
			char_type c = d->text_[i];
			if (c != ' ' && c != '\t')
				return false;
		}
	}
	return true;
}


string Paragraph::getID(Buffer const & buf, OutputParams const & runparams)
	const
{
	for (pos_type i = 0; i < size(); ++i) {
		if (Inset const * inset = getInset(i)) {
			InsetCode lyx_code = inset->lyxCode();
			if (lyx_code == LABEL_CODE) {
				InsetLabel const * const il = static_cast<InsetLabel const *>(inset);
				docstring const & id = il->getParam("name");
				return "id='" + to_utf8(sgml::cleanID(buf, runparams, id)) + "'";
			}
		}
	}
	return string();
}


pos_type Paragraph::firstWordDocBook(odocstream & os, OutputParams const & runparams)
	const
{
	pos_type i;
	for (i = 0; i < size(); ++i) {
		if (Inset const * inset = getInset(i)) {
			inset->docbook(os, runparams);
		} else {
			char_type c = d->text_[i];
			if (c == ' ')
				break;
			os << sgml::escapeChar(c);
		}
	}
	return i;
}


pos_type Paragraph::firstWordLyXHTML(XHTMLStream & xs, OutputParams const & runparams)
	const
{
	pos_type i;
	for (i = 0; i < size(); ++i) {
		if (Inset const * inset = getInset(i)) {
			inset->xhtml(xs, runparams);
		} else {
			char_type c = d->text_[i];
			if (c == ' ')
				break;
			xs << c;
		}
	}
	return i;
}


bool Paragraph::Private::onlyText(Buffer const & buf, Font const & outerfont, pos_type initial) const
{
	Font font_old;
	pos_type size = text_.size();
	for (pos_type i = initial; i < size; ++i) {
		Font font = owner_->getFont(buf.params(), i, outerfont);
		if (text_[i] == META_INSET)
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

	Layout const & style = *d->layout_;
	FontInfo font_old =
		style.labeltype == LABEL_MANUAL ? style.labelfont : style.font;

	if (style.pass_thru && !d->onlyText(buf, outerfont, initial))
		os << "]]>";

	// parsing main loop
	for (pos_type i = initial; i < size(); ++i) {
		Font font = getFont(buf.params(), i, outerfont);

		// handle <emphasis> tag
		if (font_old.emph() != font.fontInfo().emph()) {
			if (font.fontInfo().emph() == FONT_ON) {
				os << "<emphasis>";
				emph_flag = true;
			} else if (i != initial) {
				os << "</emphasis>";
				emph_flag = false;
			}
		}

		if (Inset const * inset = getInset(i)) {
			inset->docbook(os, runparams);
		} else {
			char_type c = d->text_[i];

			if (style.pass_thru)
				os.put(c);
			else
				os << sgml::escapeChar(c);
		}
		font_old = font.fontInfo();
	}

	if (emph_flag) {
		os << "</emphasis>";
	}

	if (style.free_spacing)
		os << '\n';
	if (style.pass_thru && !d->onlyText(buf, outerfont, initial))
		os << "<![CDATA[";
}


namespace {
void doFontSwitch(vector<html::FontTag> & tagsToOpen,
                  vector<html::EndFontTag> & tagsToClose,
                  bool & flag, FontState curstate, html::FontTypes type)
{
	if (curstate == FONT_ON) {
		tagsToOpen.push_back(html::FontTag(type));
		flag = true;
	} else if (flag) {
		tagsToClose.push_back(html::EndFontTag(type));
		flag = false;
	}
}
}


docstring Paragraph::simpleLyXHTMLOnePar(Buffer const & buf,
				    XHTMLStream & xs,
				    OutputParams const & runparams,
				    Font const & outerfont,
				    pos_type initial) const
{
	docstring retval;

	// track whether we have opened these tags
	bool emph_flag = false;
	bool bold_flag = false;
	bool noun_flag = false;
	bool ubar_flag = false;
	bool dbar_flag = false;
	bool sout_flag = false;
	bool wave_flag = false;
	// shape tags
	bool shap_flag = false;
	// family tags
	bool faml_flag = false;
	// size tags
	bool size_flag = false;

	Layout const & style = *d->layout_;

	xs.startParagraph(allowEmpty());

	FontInfo font_old =
		style.labeltype == LABEL_MANUAL ? style.labelfont : style.font;

	FontShape  curr_fs   = INHERIT_SHAPE;
	FontFamily curr_fam  = INHERIT_FAMILY;
	FontSize   curr_size = FONT_SIZE_INHERIT;
	
	string const default_family = 
		buf.masterBuffer()->params().fonts_default_family;		

	vector<html::FontTag> tagsToOpen;
	vector<html::EndFontTag> tagsToClose;
	
	// parsing main loop
	for (pos_type i = initial; i < size(); ++i) {
		// let's not show deleted material in the output
		if (isDeleted(i))
			continue;

		Font const font = getFont(buf.masterBuffer()->params(), i, outerfont);

		// emphasis
		FontState curstate = font.fontInfo().emph();
		if (font_old.emph() != curstate)
			doFontSwitch(tagsToOpen, tagsToClose, emph_flag, curstate, html::FT_EMPH);

		// noun
		curstate = font.fontInfo().noun();
		if (font_old.noun() != curstate)
			doFontSwitch(tagsToOpen, tagsToClose, noun_flag, curstate, html::FT_NOUN);

		// underbar
		curstate = font.fontInfo().underbar();
		if (font_old.underbar() != curstate)
			doFontSwitch(tagsToOpen, tagsToClose, ubar_flag, curstate, html::FT_UBAR);
	
		// strikeout
		curstate = font.fontInfo().strikeout();
		if (font_old.strikeout() != curstate)
			doFontSwitch(tagsToOpen, tagsToClose, sout_flag, curstate, html::FT_SOUT);

		// double underbar
		curstate = font.fontInfo().uuline();
		if (font_old.uuline() != curstate)
			doFontSwitch(tagsToOpen, tagsToClose, dbar_flag, curstate, html::FT_DBAR);

		// wavy line
		curstate = font.fontInfo().uwave();
		if (font_old.uwave() != curstate)
			doFontSwitch(tagsToOpen, tagsToClose, wave_flag, curstate, html::FT_WAVE);

		// bold
		// a little hackish, but allows us to reuse what we have.
		curstate = (font.fontInfo().series() == BOLD_SERIES ? FONT_ON : FONT_OFF);
		if (font_old.series() != font.fontInfo().series())
			doFontSwitch(tagsToOpen, tagsToClose, bold_flag, curstate, html::FT_BOLD);

		// Font shape
		curr_fs = font.fontInfo().shape();
		FontShape old_fs = font_old.shape();
		if (old_fs != curr_fs) {
			if (shap_flag) {
				switch (old_fs) {
				case ITALIC_SHAPE:
					tagsToClose.push_back(html::EndFontTag(html::FT_ITALIC));
					break;
				case SLANTED_SHAPE:
					tagsToClose.push_back(html::EndFontTag(html::FT_SLANTED));
					break;
				case SMALLCAPS_SHAPE:
					tagsToClose.push_back(html::EndFontTag(html::FT_SMALLCAPS));
					break;
				case UP_SHAPE:
				case INHERIT_SHAPE:
					break;
				default:
					// the other tags are for internal use
					LATTEST(false);
					break;
				}
				shap_flag = false;
			}
			switch (curr_fs) {
			case ITALIC_SHAPE:
				tagsToOpen.push_back(html::FontTag(html::FT_ITALIC));
				shap_flag = true;
				break;
			case SLANTED_SHAPE:
				tagsToOpen.push_back(html::FontTag(html::FT_SLANTED));
				shap_flag = true;
				break;
			case SMALLCAPS_SHAPE:
				tagsToOpen.push_back(html::FontTag(html::FT_SMALLCAPS));
				shap_flag = true;
				break;
			case UP_SHAPE:
			case INHERIT_SHAPE:
				break;
			default:
				// the other tags are for internal use
				LATTEST(false);
				break;
			}
		}

		// Font family
		curr_fam = font.fontInfo().family();
		FontFamily old_fam = font_old.family();
		if (old_fam != curr_fam) {
			if (faml_flag) {
				switch (old_fam) {
				case ROMAN_FAMILY:
					tagsToClose.push_back(html::EndFontTag(html::FT_ROMAN));
					break;
				case SANS_FAMILY:
					tagsToClose.push_back(html::EndFontTag(html::FT_SANS));
					break;
				case TYPEWRITER_FAMILY:
					tagsToClose.push_back(html::EndFontTag(html::FT_TYPE));
					break;
				case INHERIT_FAMILY:
					break;
				default:
					// the other tags are for internal use
					LATTEST(false);
					break;
				}
				faml_flag = false;
			}
			switch (curr_fam) {
			case ROMAN_FAMILY:
				// we will treat a "default" font family as roman, since we have
				// no other idea what to do.
				if (default_family != "rmdefault" && default_family != "default") {
					tagsToOpen.push_back(html::FontTag(html::FT_ROMAN));
					faml_flag = true;
				}
				break;
			case SANS_FAMILY:
				if (default_family != "sfdefault") {
					tagsToOpen.push_back(html::FontTag(html::FT_SANS));
					faml_flag = true;
				}
				break;
			case TYPEWRITER_FAMILY:
				if (default_family != "ttdefault") {
					tagsToOpen.push_back(html::FontTag(html::FT_TYPE));
					faml_flag = true;
				}
				break;
			case INHERIT_FAMILY:
				break;
			default:
				// the other tags are for internal use
				LATTEST(false);
				break;
			}
		}

		// Font size
		curr_size = font.fontInfo().size();
		FontSize old_size = font_old.size();
		if (old_size != curr_size) {
			if (size_flag) {
				switch (old_size) {
				case FONT_SIZE_TINY:
					tagsToClose.push_back(html::EndFontTag(html::FT_SIZE_TINY));
					break;
				case FONT_SIZE_SCRIPT:
					tagsToClose.push_back(html::EndFontTag(html::FT_SIZE_SCRIPT));
					break;
				case FONT_SIZE_FOOTNOTE:
					tagsToClose.push_back(html::EndFontTag(html::FT_SIZE_FOOTNOTE));
					break;
				case FONT_SIZE_SMALL:
					tagsToClose.push_back(html::EndFontTag(html::FT_SIZE_SMALL));
					break;
				case FONT_SIZE_LARGE:
					tagsToClose.push_back(html::EndFontTag(html::FT_SIZE_LARGE));
					break;
				case FONT_SIZE_LARGER:
					tagsToClose.push_back(html::EndFontTag(html::FT_SIZE_LARGER));
					break;
				case FONT_SIZE_LARGEST:
					tagsToClose.push_back(html::EndFontTag(html::FT_SIZE_LARGEST));
					break;
				case FONT_SIZE_HUGE:
					tagsToClose.push_back(html::EndFontTag(html::FT_SIZE_HUGE));
					break;
				case FONT_SIZE_HUGER:
					tagsToClose.push_back(html::EndFontTag(html::FT_SIZE_HUGER));
					break;
				case FONT_SIZE_INCREASE:
					tagsToClose.push_back(html::EndFontTag(html::FT_SIZE_INCREASE));
					break;
				case FONT_SIZE_DECREASE:
					tagsToClose.push_back(html::EndFontTag(html::FT_SIZE_DECREASE));
					break;
				case FONT_SIZE_INHERIT:
				case FONT_SIZE_NORMAL:
					break;
				default:
					// the other tags are for internal use
					LATTEST(false);
					break;
				}
				size_flag = false;
			}
			switch (curr_size) {
			case FONT_SIZE_TINY:
				tagsToOpen.push_back(html::FontTag(html::FT_SIZE_TINY));
				size_flag = true;
				break;
			case FONT_SIZE_SCRIPT:
				tagsToOpen.push_back(html::FontTag(html::FT_SIZE_SCRIPT));
				size_flag = true;
				break;
			case FONT_SIZE_FOOTNOTE:
				tagsToOpen.push_back(html::FontTag(html::FT_SIZE_FOOTNOTE));
				size_flag = true;
				break;
			case FONT_SIZE_SMALL:
				tagsToOpen.push_back(html::FontTag(html::FT_SIZE_SMALL));
				size_flag = true;
				break;
			case FONT_SIZE_LARGE:
				tagsToOpen.push_back(html::FontTag(html::FT_SIZE_LARGE));
				size_flag = true;
				break;
			case FONT_SIZE_LARGER:
				tagsToOpen.push_back(html::FontTag(html::FT_SIZE_LARGER));
				size_flag = true;
				break;
			case FONT_SIZE_LARGEST:
				tagsToOpen.push_back(html::FontTag(html::FT_SIZE_LARGEST));
				size_flag = true;
				break;
			case FONT_SIZE_HUGE:
				tagsToOpen.push_back(html::FontTag(html::FT_SIZE_HUGE));
				size_flag = true;
				break;
			case FONT_SIZE_HUGER:
				tagsToOpen.push_back(html::FontTag(html::FT_SIZE_HUGER));
				size_flag = true;
				break;
			case FONT_SIZE_INCREASE:
				tagsToOpen.push_back(html::FontTag(html::FT_SIZE_INCREASE));
				size_flag = true;
				break;
			case FONT_SIZE_DECREASE:
				tagsToOpen.push_back(html::FontTag(html::FT_SIZE_DECREASE));
				size_flag = true;
				break;
			case FONT_SIZE_NORMAL:
			case FONT_SIZE_INHERIT:
				break;
			default:
				// the other tags are for internal use
				LATTEST(false);
				break;
			}
		}

		// FIXME XHTML
		// Other such tags? What about the other text ranges?

		vector<html::EndFontTag>::const_iterator cit = tagsToClose.begin();
		vector<html::EndFontTag>::const_iterator cen = tagsToClose.end();
		for (; cit != cen; ++cit)
			xs << *cit;

		vector<html::FontTag>::const_iterator sit = tagsToOpen.begin();
		vector<html::FontTag>::const_iterator sen = tagsToOpen.end();
		for (; sit != sen; ++sit)
			xs << *sit;

		tagsToClose.clear();
		tagsToOpen.clear();

		Inset const * inset = getInset(i);
		if (inset) {
			if (!runparams.for_toc || inset->isInToc()) {
				OutputParams np = runparams;
				np.local_font = &font;
				if (!inset->getLayout().htmlisblock())
					np.html_in_par = true;
				retval += inset->xhtml(xs, np);
			}
		} else {
			char_type c = getUChar(buf.masterBuffer()->params(), i);
			xs << c;
		}
		font_old = font.fontInfo();
	}

	xs.closeFontTags();
	xs.endParagraph();
	return retval;
}


bool Paragraph::isHfill(pos_type pos) const
{
	Inset const * inset = getInset(pos);
	return inset && inset->isHfill();
}


bool Paragraph::isNewline(pos_type pos) const
{
	Inset const * inset = getInset(pos);
	return inset && inset->lyxCode() == NEWLINE_CODE;
}


bool Paragraph::isEnvSeparator(pos_type pos) const
{
	Inset const * inset = getInset(pos);
	return inset && inset->lyxCode() == SEPARATOR_CODE;
}


bool Paragraph::isLineSeparator(pos_type pos) const
{
	char_type const c = d->text_[pos];
	if (isLineSeparatorChar(c))
		return true;
	Inset const * inset = getInset(pos);
	return inset && inset->isLineSeparator();
}


bool Paragraph::isWordSeparator(pos_type pos) const
{
	if (pos == size())
		return true;
	if (Inset const * inset = getInset(pos))
		return !inset->isLetter();
	// if we have a hard hyphen (no en- or emdash) or apostrophe
	// we pass this to the spell checker
	// FIXME: this method is subject to change, visit
	// https://bugzilla.mozilla.org/show_bug.cgi?id=355178
	// to get an impression how complex this is.
	if (isHardHyphenOrApostrophe(pos))
		return false;
	char_type const c = d->text_[pos];
	// We want to pass the escape chars to the spellchecker
	docstring const escape_chars = from_utf8(lyxrc.spellchecker_esc_chars);
	return !isLetterChar(c) && !isDigitASCII(c) && !contains(escape_chars, c);
}


bool Paragraph::isHardHyphenOrApostrophe(pos_type pos) const
{
	pos_type const psize = size();
	if (pos >= psize)
		return false;
	char_type const c = d->text_[pos];
	if (c != '-' && c != '\'')
		return false;
	int nextpos = pos + 1;
	int prevpos = pos > 0 ? pos - 1 : 0;
	if ((nextpos == psize || isSpace(nextpos))
		&& (pos == 0 || isSpace(prevpos)))
		return false;
	return true;
}


bool Paragraph::isSameSpellRange(pos_type pos1, pos_type pos2) const
{
	return pos1 == pos2
		|| d->speller_state_.getRange(pos1) == d->speller_state_.getRange(pos2);
}


bool Paragraph::isChar(pos_type pos) const
{
	if (Inset const * inset = getInset(pos))
		return inset->isChar();
	char_type const c = d->text_[pos];
	return !isLetterChar(c) && !isDigitASCII(c) && !lyx::isSpace(c);
}


bool Paragraph::isSpace(pos_type pos) const
{
	if (Inset const * inset = getInset(pos))
		return inset->isSpace();
	char_type const c = d->text_[pos];
	return lyx::isSpace(c);
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
	return getParLanguage(bparams)->rightToLeft()
		&& !inInset().getLayout().forceLTR();
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
			d->requestSpellCheck(i);
		}
	}
}


bool Paragraph::isMultiLingual(BufferParams const & bparams) const
{
	Language const * doc_language = bparams.language;
	FontList::const_iterator cit = d->fontlist_.begin();
	FontList::const_iterator end = d->fontlist_.end();

	for (; cit != end; ++cit)
		if (cit->font().language() != ignore_language &&
		    cit->font().language() != latex_language &&
		    cit->font().language() != doc_language)
			return true;
	return false;
}


void Paragraph::getLanguages(std::set<Language const *> & languages) const
{
	FontList::const_iterator cit = d->fontlist_.begin();
	FontList::const_iterator end = d->fontlist_.end();

	for (; cit != end; ++cit) {
		Language const * lang = cit->font().language();
		if (lang != ignore_language &&
		    lang != latex_language)
			languages.insert(lang);
	}
}


docstring Paragraph::asString(int options) const
{
	return asString(0, size(), options);
}


docstring Paragraph::asString(pos_type beg, pos_type end, int options, const OutputParams *runparams) const
{
	odocstringstream os;

	if (beg == 0
	    && options & AS_STR_LABEL
	    && !d->params_.labelString().empty())
		os << d->params_.labelString() << ' ';

	for (pos_type i = beg; i < end; ++i) {
		if ((options & AS_STR_SKIPDELETE) && isDeleted(i))
			continue;
		char_type const c = d->text_[i];
		if (isPrintable(c) || c == '\t'
		    || (c == '\n' && (options & AS_STR_NEWLINES)))
			os.put(c);
		else if (c == META_INSET && (options & AS_STR_INSETS)) {
			if (c == META_INSET && (options & AS_STR_PLAINTEXT)) {
				LASSERT(runparams != 0, return docstring());
				getInset(i)->plaintext(os, *runparams);
			} else {
				getInset(i)->toString(os);
			}
		}
	}

	return os.str();
}


void Paragraph::forOutliner(docstring & os, size_t maxlen) const
{
	if (!d->params_.labelString().empty())
		os += d->params_.labelString() + ' ';
	for (pos_type i = 0; i < size() && os.length() < maxlen; ++i) {
		if (isDeleted(i))
			continue;
		char_type const c = d->text_[i];
		if (isPrintable(c))
			os += c;
		else if (c == '\t' || c == '\n')
			os += ' ';
		else if (c == META_INSET)
			getInset(i)->forOutliner(os, maxlen);
	}
}


void Paragraph::setInsetOwner(Inset const * inset)
{
	d->inset_owner_ = inset;
}


int Paragraph::id() const
{
	return d->id_;
}


void Paragraph::setId(int id)
{
	d->id_ = id;
}


Layout const & Paragraph::layout() const
{
	return *d->layout_;
}


void Paragraph::setLayout(Layout const & layout)
{
	d->layout_ = &layout;
}


void Paragraph::setDefaultLayout(DocumentClass const & tc)
{
	setLayout(tc.defaultLayout());
}


void Paragraph::setPlainLayout(DocumentClass const & tc)
{
	setLayout(tc.plainLayout());
}


void Paragraph::setPlainOrDefaultLayout(DocumentClass const & tclass)
{
	if (usePlainLayout())
		setPlainLayout(tclass);
	else
		setDefaultLayout(tclass);
}


Inset const & Paragraph::inInset() const
{
	LBUFERR(d->inset_owner_);
	return *d->inset_owner_;
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
	if (d->layout_->free_spacing)
		return true;
	return d->inset_owner_ && d->inset_owner_->isFreeSpacing();
}


bool Paragraph::allowEmpty() const
{
	if (d->layout_->keepempty)
		return true;
	return d->inset_owner_ && d->inset_owner_->allowEmpty();
}


bool Paragraph::brokenBiblio() const
{
	// there is a problem if there is no bibitem at position 0 or
	// if there is another bibitem in the paragraph.
	return d->layout_->labeltype == LABEL_BIBLIO
		&& (d->insetlist_.find(BIBITEM_CODE) != 0
		    || d->insetlist_.find(BIBITEM_CODE, 1) > 0);
}


int Paragraph::fixBiblio(Buffer const & buffer)
{
	// FIXME: What about the case where paragraph is not BIBLIO
	// but there is an InsetBibitem?
	// FIXME: when there was already an inset at 0, the return value is 1,
	// which does not tell whether another inset has been remove; the
	// cursor cannot be correctly updated.

	if (d->layout_->labeltype != LABEL_BIBLIO)
		return 0;

	bool const track_changes = buffer.params().track_changes;
	int bibitem_pos = d->insetlist_.find(BIBITEM_CODE);
	bool const hasbibitem0 = bibitem_pos == 0;

	if (hasbibitem0) {
		bibitem_pos = d->insetlist_.find(BIBITEM_CODE, 1);
		// There was an InsetBibitem at pos 0, and no other one => OK
		if (bibitem_pos == -1)
			return 0;
		// there is a bibitem at the 0 position, but since
		// there is a second one, we copy the second on the
		// first. We're assuming there are at most two of
		// these, which there should be.
		// FIXME: why does it make sense to do that rather
		// than keep the first? (JMarc)
		Inset * inset = releaseInset(bibitem_pos);
		d->insetlist_.begin()->inset = inset;
		return -bibitem_pos;
	}

	// We need to create an inset at the beginning
	Inset * inset = 0;
	if (bibitem_pos > 0) {
		// there was one somewhere in the paragraph, let's move it
		inset = d->insetlist_.release(bibitem_pos);
		eraseChar(bibitem_pos, track_changes);
	} else
		// make a fresh one
		inset = new InsetBibitem(const_cast<Buffer *>(&buffer),
					 InsetCommandParams(BIBITEM_CODE));

	Font font(inherit_font, buffer.params().language);
	insertInset(0, inset, font, Change(track_changes ? Change::INSERTED 
				                   : Change::UNCHANGED));

	return 1;
}


void Paragraph::checkAuthors(AuthorList const & authorList)
{
	d->changes_.checkAuthors(authorList);
}


bool Paragraph::isChanged(pos_type pos) const
{
	return lookupChange(pos).changed();
}


bool Paragraph::isInserted(pos_type pos) const
{
	return lookupChange(pos).inserted();
}


bool Paragraph::isDeleted(pos_type pos) const
{
	return lookupChange(pos).deleted();
}


InsetList const & Paragraph::insetList() const
{
	return d->insetlist_;
}


void Paragraph::setBuffer(Buffer & b)
{
	d->insetlist_.setBuffer(b);
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
	return (pos < pos_type(d->text_.size()) && d->text_[pos] == META_INSET)
		 ? d->insetlist_.get(pos) : 0;
}


Inset const * Paragraph::getInset(pos_type pos) const
{
	return (pos < pos_type(d->text_.size()) && d->text_[pos] == META_INSET)
		 ? d->insetlist_.get(pos) : 0;
}


void Paragraph::changeCase(BufferParams const & bparams, pos_type pos,
		pos_type & right, TextCase action)
{
	// process sequences of modified characters; in change
	// tracking mode, this approach results in much better
	// usability than changing case on a char-by-char basis
	// We also need to track the current font, since font
	// changes within sequences can occur.
	vector<pair<char_type, Font> > changes;

	bool const trackChanges = bparams.track_changes;

	bool capitalize = true;

	for (; pos < right; ++pos) {
		char_type oldChar = d->text_[pos];
		char_type newChar = oldChar;

		// ignore insets and don't play with deleted text!
		if (oldChar != META_INSET && !isDeleted(pos)) {
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

		if (isWordSeparator(pos) || isDeleted(pos)) {
			// permit capitalization again
			capitalize = true;
		}

		if (oldChar != newChar) {
			changes.push_back(make_pair(newChar, getFontSettings(bparams, pos)));
			if (pos != right - 1)
				continue;
			// step behind the changing area
			pos++;
		}

		int erasePos = pos - changes.size();
		for (size_t i = 0; i < changes.size(); i++) {
			insertChar(pos, changes[i].first,
				   changes[i].second,
				   trackChanges);
			if (!eraseChar(erasePos, trackChanges)) {
				++erasePos;
				++pos; // advance
				++right; // expand selection
			}
		}
		changes.clear();
	}
}


int Paragraph::find(docstring const & str, bool cs, bool mw,
		pos_type start_pos, bool del) const
{
	pos_type pos = start_pos;
	int const strsize = str.length();
	int i = 0;
	pos_type const parsize = d->text_.size();
	for (i = 0; i < strsize && pos < parsize; ++i, ++pos) {
		// Ignore "invisible" letters such as ligature breaks
		// and hyphenation chars while searching
		while (pos < parsize - 1 && isInset(pos)) {
			odocstringstream os;
			getInset(pos)->toString(os);
			if (!getInset(pos)->isLetter() || !os.str().empty())
				break;
			pos++;
		}
		if (cs && str[i] != d->text_[pos])
			break;
		if (!cs && uppercase(str[i]) != uppercase(d->text_[pos]))
			break;
		if (!del && isDeleted(pos))
			break;
	}

	if (i != strsize)
		return 0;

	// if necessary, check whether string matches word
	if (mw) {
		if (start_pos > 0 && !isWordSeparator(start_pos - 1))
			return 0;
		if (pos < parsize
			&& !isWordSeparator(pos))
			return 0;
	}

	return pos - start_pos;
}


char_type Paragraph::getChar(pos_type pos) const
{
	return d->text_[pos];
}


pos_type Paragraph::size() const
{
	return d->text_.size();
}


bool Paragraph::empty() const
{
	return d->text_.empty();
}


bool Paragraph::isInset(pos_type pos) const
{
	return d->text_[pos] == META_INSET;
}


bool Paragraph::isSeparator(pos_type pos) const
{
	//FIXME: Are we sure this can be the only separator?
	return d->text_[pos] == ' ';
}


void Paragraph::deregisterWords()
{
	Private::LangWordsMap::const_iterator itl = d->words_.begin();
	Private::LangWordsMap::const_iterator ite = d->words_.end();
	for (; itl != ite; ++itl) {
		WordList * wl = theWordList(itl->first);
		Private::Words::const_iterator it = (itl->second).begin();
		Private::Words::const_iterator et = (itl->second).end();
		for (; it != et; ++it)
			wl->remove(*it);
	}
	d->words_.clear();
}


void Paragraph::locateWord(pos_type & from, pos_type & to,
	word_location const loc) const
{
	switch (loc) {
	case WHOLE_WORD_STRICT:
		if (from == 0 || from == size()
		    || isWordSeparator(from)
		    || isWordSeparator(from - 1)) {
			to = from;
			return;
		}
		// no break here, we go to the next

	case WHOLE_WORD:
		// If we are already at the beginning of a word, do nothing
		if (!from || isWordSeparator(from - 1))
			break;
		// no break here, we go to the next

	case PREVIOUS_WORD:
		// always move the cursor to the beginning of previous word
		while (from && !isWordSeparator(from - 1))
			--from;
		break;
	case NEXT_WORD:
		LYXERR0("Paragraph::locateWord: NEXT_WORD not implemented yet");
		break;
	case PARTIAL_WORD:
		// no need to move the 'from' cursor
		break;
	}
	to = from;
	while (to < size() && !isWordSeparator(to))
		++to;
}


void Paragraph::collectWords()
{
	for (pos_type pos = 0; pos < size(); ++pos) {
		if (isWordSeparator(pos))
			continue;
		pos_type from = pos;
		locateWord(from, pos, WHOLE_WORD);
		// Work around MSVC warning: The statement
		// if (pos < from + lyxrc.completion_minlength)
		// triggers a signed vs. unsigned warning.
		// I don't know why this happens, it could be a MSVC bug, or
		// related to LLP64 (windows) vs. LP64 (unix) programming
		// model, or the C++ standard might be ambigous in the section
		// defining the "usual arithmetic conversions". However, using
		// a temporary variable is safe and works on all compilers.
		pos_type const endpos = from + lyxrc.completion_minlength;
		if (pos < endpos)
			continue;
		FontList::const_iterator cit = d->fontlist_.fontIterator(from);
		if (cit == d->fontlist_.end())
			return;
		Language const * lang = cit->font().language();
		docstring const word = asString(from, pos, AS_STR_NONE);
		d->words_[lang->lang()].insert(word);
	}
}


void Paragraph::registerWords()
{
	Private::LangWordsMap::const_iterator itl = d->words_.begin();
	Private::LangWordsMap::const_iterator ite = d->words_.end();
	for (; itl != ite; ++itl) {
		WordList * wl = theWordList(itl->first);
		Private::Words::const_iterator it = (itl->second).begin();
		Private::Words::const_iterator et = (itl->second).end();
		for (; it != et; ++it)
			wl->insert(*it);
	}
}


void Paragraph::updateWords()
{
	deregisterWords();
	collectWords();
	registerWords();
}


void Paragraph::Private::appendSkipPosition(SkipPositions & skips, pos_type const pos) const
{
	SkipPositionsIterator begin = skips.begin();
	SkipPositions::iterator end = skips.end();
	if (pos > 0 && begin < end) {
		--end;
		if (end->last == pos - 1) {
			end->last = pos;
			return;
		}
	}
	skips.insert(end, FontSpan(pos, pos));
}


Language * Paragraph::Private::locateSpellRange(
	pos_type & from, pos_type & to,
	SkipPositions & skips) const
{
	// skip leading white space
	while (from < to && owner_->isWordSeparator(from))
		++from;
	// don't check empty range
	if (from >= to)
		return 0;
	// get current language
	Language * lang = getSpellLanguage(from);
	pos_type last = from;
	bool samelang = true;
	bool sameinset = true;
	while (last < to && samelang && sameinset) {
		// hop to end of word
		while (last < to && !owner_->isWordSeparator(last)) {
			if (owner_->getInset(last)) {
				appendSkipPosition(skips, last);
			} else if (owner_->isDeleted(last)) {
				appendSkipPosition(skips, last);
			}
			++last;
		}
		// hop to next word while checking for insets
		while (sameinset && last < to && owner_->isWordSeparator(last)) {
			if (Inset const * inset = owner_->getInset(last))
				sameinset = inset->isChar() && inset->isLetter();
			if (sameinset && owner_->isDeleted(last)) {
				appendSkipPosition(skips, last);
			}
			if (sameinset)
				last++;
		}
		if (sameinset && last < to) {
			// now check for language change
			samelang = lang == getSpellLanguage(last);
		}
	}
	// if language change detected backstep is needed
	if (!samelang)
		--last;
	to = last;
	return lang;
}


Language * Paragraph::Private::getSpellLanguage(pos_type const from) const
{
	Language * lang =
		const_cast<Language *>(owner_->getFontSettings(
			inset_owner_->buffer().params(), from).language());
	if (lang == inset_owner_->buffer().params().language
		&& !lyxrc.spellchecker_alt_lang.empty()) {
		string lang_code;
		string const lang_variety =
			split(lyxrc.spellchecker_alt_lang, lang_code, '-');
		lang->setCode(lang_code);
		lang->setVariety(lang_variety);
	}
	return lang;
}


void Paragraph::requestSpellCheck(pos_type pos)
{
	d->requestSpellCheck(pos);
}


bool Paragraph::needsSpellCheck() const
{
	SpellChecker::ChangeNumber speller_change_number = 0;
	if (theSpellChecker())
		speller_change_number = theSpellChecker()->changeNumber();
	if (speller_change_number > d->speller_state_.currentChangeNumber()) {
		d->speller_state_.needsCompleteRefresh(speller_change_number);
	}
	return d->needsSpellCheck();
}


bool Paragraph::Private::ignoreWord(docstring const & word) const
{
	// Ignore words with digits
	// FIXME: make this customizable
	// (note that some checkers ignore words with digits by default)
	docstring::const_iterator cit = word.begin();
	docstring::const_iterator const end = word.end();
	for (; cit != end; ++cit) {
		if (isNumber((*cit)))
			return true;
	}
	return false;
}


SpellChecker::Result Paragraph::spellCheck(pos_type & from, pos_type & to,
	WordLangTuple & wl, docstring_list & suggestions,
	bool do_suggestion, bool check_learned) const
{
	SpellChecker::Result result = SpellChecker::WORD_OK;
	SpellChecker * speller = theSpellChecker();
	if (!speller)
		return result;

	if (!d->layout_->spellcheck || !inInset().allowSpellCheck())
		return result;

	locateWord(from, to, WHOLE_WORD);
	if (from == to || from >= size())
		return result;

	docstring word = asString(from, to, AS_STR_INSETS | AS_STR_SKIPDELETE);
	Language * lang = d->getSpellLanguage(from);

	wl = WordLangTuple(word, lang);

	if (word.empty())
		return result;

	if (needsSpellCheck() || check_learned) {
		pos_type end = to;
		if (!d->ignoreWord(word)) {
			bool const trailing_dot = to < size() && d->text_[to] == '.';
			result = speller->check(wl);
			if (SpellChecker::misspelled(result) && trailing_dot) {
				wl = WordLangTuple(word.append(from_ascii(".")), lang);
				result = speller->check(wl);
				if (!SpellChecker::misspelled(result)) {
					LYXERR(Debug::GUI, "misspelled word is correct with dot: \"" <<
					   word << "\" [" <<
					   from << ".." << to << "]");
				} else {
					// spell check with dot appended failed too
					// restore original word/lang value
					word = asString(from, to, AS_STR_INSETS | AS_STR_SKIPDELETE);
					wl = WordLangTuple(word, lang);
				}
			}
		}
		if (!SpellChecker::misspelled(result)) {
			// area up to the begin of the next word is not misspelled
			while (end < size() && isWordSeparator(end))
				++end;
		}
		d->setMisspelled(from, end, result);
	} else {
		result = d->speller_state_.getState(from);
	}

	if (do_suggestion)
		suggestions.clear();

	if (SpellChecker::misspelled(result)) {
		LYXERR(Debug::GUI, "misspelled word: \"" <<
			   word << "\" [" <<
			   from << ".." << to << "]");
		if (do_suggestion)
			speller->suggest(wl, suggestions);
	}
	return result;
}


void Paragraph::Private::markMisspelledWords(
	pos_type const & first, pos_type const & last,
	SpellChecker::Result result,
	docstring const & word,
	SkipPositions const & skips)
{
	if (!SpellChecker::misspelled(result)) {
		setMisspelled(first, last, SpellChecker::WORD_OK);
		return;
	}
	int snext = first;
	SpellChecker * speller = theSpellChecker();
	// locate and enumerate the error positions
	int nerrors = speller->numMisspelledWords();
	int numskipped = 0;
	SkipPositionsIterator it = skips.begin();
	SkipPositionsIterator et = skips.end();
	for (int index = 0; index < nerrors; ++index) {
		int wstart;
		int wlen = 0;
		speller->misspelledWord(index, wstart, wlen);
		/// should not happen if speller supports range checks
		if (!wlen) continue;
		docstring const misspelled = word.substr(wstart, wlen);
		wstart += first + numskipped;
		if (snext < wstart) {
			/// mark the range of correct spelling
			numskipped += countSkips(it, et, wstart);
			setMisspelled(snext,
				wstart - 1, SpellChecker::WORD_OK);
		}
		snext = wstart + wlen;
		numskipped += countSkips(it, et, snext);
		/// mark the range of misspelling
		setMisspelled(wstart, snext, result);
		LYXERR(Debug::GUI, "misspelled word: \"" <<
			   misspelled << "\" [" <<
			   wstart << ".." << (snext-1) << "]");
		++snext;
	}
	if (snext <= last) {
		/// mark the range of correct spelling at end
		setMisspelled(snext, last, SpellChecker::WORD_OK);
	}
}


void Paragraph::spellCheck() const
{
	SpellChecker * speller = theSpellChecker();
	if (!speller || empty() ||!needsSpellCheck())
		return;
	pos_type start;
	pos_type endpos;
	d->rangeOfSpellCheck(start, endpos);
	if (speller->canCheckParagraph()) {
		// loop until we leave the range
		for (pos_type first = start; first < endpos; ) {
			pos_type last = endpos;
			Private::SkipPositions skips;
			Language * lang = d->locateSpellRange(first, last, skips);
			if (first >= endpos)
				break;
			// start the spell checker on the unit of meaning
			docstring word = asString(first, last, AS_STR_INSETS + AS_STR_SKIPDELETE);
			WordLangTuple wl = WordLangTuple(word, lang);
			SpellChecker::Result result = word.size() ?
				speller->check(wl) : SpellChecker::WORD_OK;
			d->markMisspelledWords(first, last, result, word, skips);
			first = ++last;
		}
	} else {
		static docstring_list suggestions;
		pos_type to = endpos;
		while (start < endpos) {
			WordLangTuple wl;
			spellCheck(start, to, wl, suggestions, false);
			start = to + 1;
		}
	}
	d->readySpellCheck();
}


bool Paragraph::isMisspelled(pos_type pos, bool check_boundary) const
{
	bool result = SpellChecker::misspelled(d->speller_state_.getState(pos));
	if (result || pos <= 0 || pos > size())
		return result;
	if (check_boundary && (pos == size() || isWordSeparator(pos)))
		result = SpellChecker::misspelled(d->speller_state_.getState(pos - 1));
	return result;
}


string Paragraph::magicLabel() const
{
	stringstream ss;
	ss << "magicparlabel-" << id();
	return ss.str();
}


} // namespace lyx
