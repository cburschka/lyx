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

#include "LayoutFile.h"
#include "Buffer.h"
#include "BufferParams.h"
#include "Changes.h"
#include "Counters.h"
#include "Encoding.h"
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
#include "paragraph_funcs.h"
#include "ParagraphParameters.h"
#include "sgml.h"
#include "TextClass.h"
#include "TexRow.h"
#include "Text.h"
#include "VSpace.h"
#include "WordList.h"

#include "frontends/alert.h"

#include "insets/InsetBibitem.h"
#include "insets/InsetLabel.h"

#include "support/assert.h"
#include "support/convert.h"
#include "support/debug.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/Messages.h"
#include "support/textutils.h"

#include <sstream>
#include <vector>

using namespace std;
using namespace lyx::support;

namespace lyx {

namespace {
/// Inset identifier (above 0x10ffff, for ucs-4)
char_type const META_INSET = 0x200001;
};

/////////////////////////////////////////////////////////////////////
//
// Paragraph::Private
//
/////////////////////////////////////////////////////////////////////

class Paragraph::Private
{
public:
	///
	Private(Paragraph * owner, Layout const & layout);
	/// "Copy constructor"
	Private(Private const &, Paragraph * owner);

	///
	void insertChar(pos_type pos, char_type c, Change const & change);

	/// Output the surrogate pair formed by \p c and \p next to \p os.
	/// \return the number of characters written.
	int latexSurrogatePair(odocstream & os, char_type c, char_type next,
			       Encoding const &);

	/// Output a space in appropriate formatting (or a surrogate pair
	/// if the next character is a combining character).
	/// \return whether a surrogate pair was output.
	bool simpleTeXBlanks(OutputParams const &,
			     odocstream &, TexRow & texrow,
			     pos_type i,
			     unsigned int & column,
			     Font const & font,
			     Layout const & style);

	/// Output consecutive unicode chars, belonging to the same script as
	/// specified by the latex macro \p ltx, to \p os starting from \p i.
	/// \return the number of characters written.
	int writeScriptChars(odocstream & os, docstring const & ltx,
			   Change &, Encoding const &, pos_type & i);

	/// This could go to ParagraphParameters if we want to.
	int startTeXParParams(BufferParams const &, odocstream &, TexRow &,
			      bool) const;

	/// This could go to ParagraphParameters if we want to.
	int endTeXParParams(BufferParams const &, odocstream &, TexRow &,
			    bool) const;

	///
	void latexInset(BufferParams const &,
				   odocstream &,
				   TexRow & texrow, OutputParams &,
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
				   odocstream & os,
				   OutputParams & runparams,
				   Font & running_font,
				   Change & running_change,
				   Layout const & style,
				   pos_type & i,
				   unsigned int & column);

	///
	bool latexSpecialT1(
		char_type const c,
		odocstream & os,
		pos_type & i,
		unsigned int & column);
	///
	bool latexSpecialTypewriter(
		char_type const c,
		odocstream & os,
		pos_type & i,
		unsigned int & column);
	///
	bool latexSpecialPhrase(
		odocstream & os,
		pos_type & i,
		unsigned int & column,
		OutputParams & runparams);

	///
	void validate(LaTeXFeatures & features,
		      Layout const & layout) const;

	/// Checks if the paragraph contains only text and no inset or font change.
	bool onlyText(Buffer const & buf, Font const & outerfont,
		      pos_type initial) const;

	/// match a string against a particular point in the paragraph
	bool isTextAt(string const & str, pos_type pos) const;
	
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

	/// end of label
	pos_type begin_of_body_;

	typedef docstring TextContainer;
	///
	TextContainer text_;
	
	typedef std::set<docstring> Words;
	///
	Words words_;
	///
	Layout const * layout_;
};


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


Paragraph::Private::Private(Paragraph * owner, Layout const & layout)
	: owner_(owner), inset_owner_(0), begin_of_body_(0), layout_(&layout)
{
	id_ = paragraph_id++;
	text_.reserve(100);
}


Paragraph::Private::Private(Private const & p, Paragraph * owner)
	: owner_(owner), inset_owner_(p.inset_owner_), fontlist_(p.fontlist_), 
	  params_(p.params_), changes_(p.changes_), insetlist_(p.insetlist_),
	  begin_of_body_(p.begin_of_body_), text_(p.text_), words_(p.words_),
	  layout_(p.layout_)
{
	id_ = paragraph_id++;
}


bool Paragraph::isChanged(pos_type start, pos_type end) const
{
	LASSERT(start >= 0 && start <= size(), /**/);
	LASSERT(end > start && end <= size() + 1, /**/);

	return d->changes_.isChanged(start, end);
}


bool Paragraph::isMergedOnEndOfParDeletion(bool trackChanges) const
{
	// keep the logic here in sync with the logic of eraseChars()
	if (!trackChanges)
		return true;

	Change const change = d->changes_.lookup(size());
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
			if (Inset * inset = getInset(pos))
				inset->setChange(change);
		}
	}
}


void Paragraph::setChange(pos_type pos, Change const & change)
{
	LASSERT(pos >= 0 && pos <= size(), /**/);
	d->changes_.set(change, pos);

	// see comment in setChange(Change const &) above
	if (change.type != Change::DELETED && pos < size())
			if (Inset * inset = getInset(pos))
				inset->setChange(change);
}


Change const & Paragraph::lookupChange(pos_type pos) const
{
	LASSERT(pos >= 0 && pos <= size(), /**/);
	return d->changes_.lookup(pos);
}


void Paragraph::acceptChanges(BufferParams const & bparams, pos_type start,
		pos_type end)
{
	LASSERT(start >= 0 && start <= size(), /**/);
	LASSERT(end > start && end <= size() + 1, /**/);

	for (pos_type pos = start; pos < end; ++pos) {
		switch (lookupChange(pos).type) {
			case Change::UNCHANGED:
				// accept changes in nested inset
				if (Inset * inset = getInset(pos))
					inset->acceptChanges(bparams);
				break;

			case Change::INSERTED:
				d->changes_.set(Change(Change::UNCHANGED), pos);
				// also accept changes in nested inset
				if (Inset * inset = getInset(pos))
					inset->acceptChanges(bparams);
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
	LASSERT(start >= 0 && start <= size(), /**/);
	LASSERT(end > start && end <= size() + 1, /**/);

	for (pos_type pos = start; pos < end; ++pos) {
		switch (lookupChange(pos).type) {
			case Change::UNCHANGED:
				// reject changes in nested inset
				if (Inset * inset = getInset(pos))
						inset->rejectChanges(bparams);
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
	LASSERT(pos >= 0 && pos <= int(text_.size()), /**/);

	// track change
	changes_.insert(change, pos);

	// This is actually very common when parsing buffers (and
	// maybe inserting ascii text)
	if (pos == pos_type(text_.size())) {
		// when appending characters, no need to update tables
		text_.push_back(c);
		return;
	}

	text_.insert(text_.begin() + pos, c);

	// Update the font table.
	fontlist_.increasePosAfterPos(pos);

	// Update the insets
	insetlist_.increasePosAfterPos(pos);
}


void Paragraph::insertInset(pos_type pos, Inset * inset,
				   Change const & change)
{
	LASSERT(inset, /**/);
	LASSERT(pos >= 0 && pos <= size(), /**/);

	d->insertChar(pos, META_INSET, change);
	LASSERT(d->text_[pos] == META_INSET, /**/);

	// Add a new entry in the insetlist_.
	d->insetlist_.insert(inset, pos);
}


bool Paragraph::eraseChar(pos_type pos, bool trackChanges)
{
	LASSERT(pos >= 0 && pos <= size(), /**/);

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
	if (d->text_[pos] == META_INSET)
		d->insetlist_.erase(pos);

	d->text_.erase(d->text_.begin() + pos);

	// Update the fontlist_
	d->fontlist_.erase(pos);

	// Update the insetlist_
	d->insetlist_.decreasePosAfterPos(pos);

	return true;
}


int Paragraph::eraseChars(pos_type start, pos_type end, bool trackChanges)
{
	LASSERT(start >= 0 && start <= size(), /**/);
	LASSERT(end >= start && end <= size() + 1, /**/);

	pos_type i = start;
	for (pos_type count = end - start; count; --count) {
		if (!eraseChar(i, trackChanges))
			++i;
	}
	return end - i;
}


int Paragraph::Private::latexSurrogatePair(odocstream & os, char_type c,
		char_type next, Encoding const & encoding)
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


bool Paragraph::Private::simpleTeXBlanks(OutputParams const & runparams,
				       odocstream & os, TexRow & texrow,
				       pos_type i,
				       unsigned int & column,
				       Font const & font,
				       Layout const & style)
{
	if (style.pass_thru || runparams.verbatim)
		return false;

	if (i + 1 < int(text_.size())) {
		char_type next = text_[i + 1];
		if (Encodings::isCombiningChar(next)) {
			Encoding const & encoding = *(runparams.encoding);
			// This space has an accent, so we must always output it.
			column += latexSurrogatePair(os, ' ', next, encoding) - 1;
			return true;
		}
	}

	if (lyxrc.plaintext_linelen > 0
	    && column > lyxrc.plaintext_linelen
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


int Paragraph::Private::writeScriptChars(odocstream & os,
					 docstring const & ltx,
					 Change & runningChange,
					 Encoding const & encoding,
					 pos_type & i)
{
	// FIXME: modifying i here is not very nice...

	// We only arrive here when a proper language for character text_[i] has
	// not been specified (i.e., it could not be translated in the current
	// latex encoding) and it belongs to a known script.
	// Parameter ltx contains the latex translation of text_[i] as specified in
	// the unicodesymbols file and is something like "\textXXX{<spec>}".
	// The latex macro name "textXXX" specifies the script to which text_[i]
	// belongs and we use it in order to check whether characters from the
	// same script immediately follow, such that we can collect them in a
	// single "\textXXX" macro. So, we have to retain "\textXXX{<spec>"
	// for the first char but only "<spec>" for all subsequent chars.
	docstring::size_type const brace1 = ltx.find_first_of(from_ascii("{"));
	docstring::size_type const brace2 = ltx.find_last_of(from_ascii("}"));
	string script = to_ascii(ltx.substr(1, brace1 - 1));
	int length = ltx.substr(0, brace2).length();
	os << ltx.substr(0, brace2);
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


void Paragraph::Private::latexInset(
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
					     unsigned int & column)
{
	Inset * inset = owner_->getInset(i);
	LASSERT(inset, /**/);

	if (style.pass_thru) {
		inset->plaintext(os, runparams);
		return;
	}

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

			if (running_font.fontInfo().family() == TYPEWRITER_FAMILY)
				os << '~';

			basefont = owner_->getLayoutFont(bparams, outerfont);
			running_font = basefont;

			if (runparams.moving_arg)
				os << "\\protect ";

		}
		texrow.newline();
		texrow.start(owner_->id(), i + 1);
		column = 0;
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

	if (inset->forceLTR() 
	    && running_font.isRightToLeft()
		// ERT is an exception, it should be output with no decorations at all
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
	if (open_font && inset->noFontChange()) {
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

	int tmp = inset->latex(os, runparams);

	if (close) {
    	if (running_font.language()->lang() == "farsi")
			os << "\\endL{}";
		else
			os << '}';
	}

	if (tmp) {
		for (int j = 0; j < tmp; ++j)
			texrow.newline();

		texrow.start(owner_->id(), i + 1);
		column = 0;
	} else {
		column += os.tellp() - len;
	}

	if (owner_->lookupChange(i).type == Change::DELETED)
		--runparams.inDeletedInset;
}


void Paragraph::Private::latexSpecialChar(
					     odocstream & os,
					     OutputParams & runparams,
					     Font & running_font,
					     Change & running_change,
					     Layout const & style,
					     pos_type & i,
					     unsigned int & column)
{
	char_type const c = text_[i];

	if (style.pass_thru) {
		if (c != '\0')
			// FIXME UNICODE: This can fail if c cannot
			// be encoded in the current encoding.
			os.put(c);
		return;
	}

	if (runparams.verbatim) {
		os.put(c);
		return;
	}

	if (lyxrc.fontenc == "T1" && latexSpecialT1(c, os, i, column))
		return;

	if (running_font.fontInfo().family() == TYPEWRITER_FAMILY
		&& latexSpecialTypewriter(c, os, i, column))
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

		// LyX, LaTeX etc.
		if (latexSpecialPhrase(os, i, column, runparams))
			return;

		if (c == '\0')
			return;

		Encoding const & encoding = *(runparams.encoding);
		if (i + 1 < int(text_.size())) {
			char_type next = text_[i + 1];
			if (Encodings::isCombiningChar(next)) {
				column += latexSurrogatePair(os, c, next, encoding) - 1;
				++i;
				break;
			}
		}
		string script;
		docstring const latex = encoding.latexChar(c);
		if (Encodings::isKnownScriptChar(c, script)
		    && prefixIs(latex, from_ascii("\\" + script)))
			column += writeScriptChars(os, latex,
					running_change, encoding, i) - 1;
		else if (latex.length() > 1 && latex[latex.length() - 1] != '}') {
			// Prevent eating of a following
			// space or command corruption by
			// following characters
			column += latex.length() + 1;
			os << latex << "{}";
		} else {
			column += latex.length() - 1;
			os << latex;
		}
		break;
	}
}


bool Paragraph::Private::latexSpecialT1(char_type const c, odocstream & os,
	pos_type & i, unsigned int & column)
{
	switch (c) {
	case '>':
	case '<':
		os.put(c);
		// In T1 encoding, these characters exist
		// but we should avoid ligatures
		if (i + 1 >= int(text_.size()) || text_[i + 1] != c)
			return true;
		os << "\\,{}";
		column += 3;
		// Alternative code:
		//os << "\\textcompwordmark{}";
		//column += 19;
		return true;
	case '|':
		os.put(c);
		return true;
	default:
		return false;
	}
}


bool Paragraph::Private::latexSpecialTypewriter(char_type const c, odocstream & os,
	pos_type & i, unsigned int & column)
{
	switch (c) {
	case '-':
		if (i + 1 < int(text_.size()) && text_[i + 1] == '-') {
			// "--" in Typewriter mode -> "-{}-"
			os << "-{}";
			column += 2;
		} else
			os << '-';
		return true;

	// I assume this is hack treating typewriter as verbatim
	// FIXME UNICODE: This can fail if c cannot be encoded
	// in the current encoding.

	case '\0':
		return true;

	// Those characters are not directly supported.
	case '\\':
	case '\"':
	case '$': case '&':
	case '%': case '#': case '{':
	case '}': case '_':
	case '~':
	case '^':
	case '*': case '[':
	case ' ':
		return false;

	default:
		// With Typewriter font, these characters exist.
		os.put(c);
		return true;
	}
}


bool Paragraph::Private::latexSpecialPhrase(odocstream & os, pos_type & i,
	unsigned int & column, OutputParams & runparams)
{
	// FIXME: if we have "LaTeX" with a font
	// change in the middle (before the 'T', then
	// the "TeX" part is still special cased.
	// Really we should only operate this on
	// "words" for some definition of word

	for (size_t pnr = 0; pnr < phrases_nr; ++pnr) {
		if (!isTextAt(special_phrases[pnr].phrase, i))
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


void Paragraph::Private::validate(LaTeXFeatures & features,
				Layout const & layout) const
{
	// check the params.
	if (!params_.spacing().isDefault())
		features.require("setspace");

	// then the layouts
	features.useLayout(layout.name());

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
			if (layout.needprotect &&
			    icit->inset->lyxCode() == FOOT_CODE)
				features.require("NeedLyXFootnoteCode");
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
		Encodings::validate(text_[i], features);
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

		char_type const c = d->text_[i];
		switch (c) {
		case META_INSET:
			if (Inset const * inset = getInset(i)) {
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
			}
			break;
		case '\\':
			os << "\n\\backslash\n";
			column = 0;
			break;
		case '.':
			if (i + 1 < size() && d->text_[i + 1] == ' ') {
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
			if (c != '\0')
				os << to_utf8(docstring(1, c));
			else
				LYXERR0("NUL char in structure.");
			++column;
			break;
		}
	}

	os << "\n\\end_layout\n";
}


void Paragraph::validate(LaTeXFeatures & features) const
{
	d->validate(features, *d->layout_);
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
	for (pos_type i = 0; i != end; ++i) {
		// track change
		d->changes_.insert(change, i);
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


void Paragraph::resetFonts(Font const & font)
{
	d->fontlist_.clear();
	d->fontlist_.set(0, font);
	d->fontlist_.set(d->text_.size() - 1, font);
}

// Gets uninstantiated font setting at position.
Font const Paragraph::getFontSettings(BufferParams const & bparams,
					 pos_type pos) const
{
	if (pos > size()) {
		LYXERR0("pos: " << pos << " size: " << size());
		LASSERT(pos <= size(), /**/);
	}

	FontList::const_iterator cit = d->fontlist_.fontIterator(pos);
	if (cit != d->fontlist_.end())
		return cit->font();

	if (pos == size() && !empty())
		return getFontSettings(bparams, pos - 1);

	return Font(inherit_font, getParLanguage(bparams));
}


FontSpan Paragraph::fontSpan(pos_type pos) const
{
	LASSERT(pos <= size(), /**/);
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
	// LYXERR0("Paragraph::getEndPosOfFontSpan: This should not happen!");
	return FontSpan(pos, pos);
}


// Gets uninstantiated font setting at position 0
Font const Paragraph::getFirstFontSettings(BufferParams const & bparams) const
{
	if (!empty() && !d->fontlist_.empty())
		return d->fontlist_.begin()->font();

	return Font(inherit_font, bparams.language);
}


// Gets the fully instantiated font at a given position in a paragraph
// This is basically the same function as Text::GetFont() in text2.cpp.
// The difference is that this one is used for generating the LaTeX file,
// and thus cosmetic "improvements" are disallowed: This has to deliver
// the true picture of the buffer. (Asger)
Font const Paragraph::getFont(BufferParams const & bparams, pos_type pos,
				 Font const & outerfont) const
{
	LASSERT(pos >= 0, /**/);

	Font font = getFontSettings(bparams, pos);

	pos_type const body_pos = beginOfBody();
	if (pos < body_pos)
		font.fontInfo().realize(d->layout_->labelfont);
	else
		font.fontInfo().realize(d->layout_->font);

	font.fontInfo().realize(outerfont.fontInfo());
	font.fontInfo().realize(bparams.getFont().fontInfo());

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
	if (!lyxrc.rtl_support)
		return c;

	char_type uc = c;
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
	return c;
}


void Paragraph::setFont(pos_type pos, Font const & font)
{
	LASSERT(pos <= size(), /**/);

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
	if (d->layout_->margintype == MARGIN_MANUAL)
		return d->params_.labelWidthString();
	else
		return _("Senseless with this layout!");
}


void Paragraph::setLabelWidthString(docstring const & s)
{
	d->params_.labelWidthString(s);
}


docstring const Paragraph::translateIfPossible(docstring const & s,
		BufferParams const & bparams) const
{
	if (!isAscii(s) || s.empty()) {
		// This must be a user defined layout. We cannot translate
		// this, since gettext accepts only ascii keys.
		return s;
	}
	// Probably standard layout, try to translate
	Messages & m = getMessages(getParLanguage(bparams)->code());
	return m.get(to_ascii(s));
}


docstring Paragraph::expandLabel(Layout const & layout,
		BufferParams const & bparams, bool process_appendix) const
{
	DocumentClass const & tclass = bparams.documentClass();

	docstring fmt;
	if (process_appendix && d->params_.appendix())
		fmt = translateIfPossible(layout.labelstring_appendix(),
			bparams);
	else
		fmt = translateIfPossible(layout.labelstring(), bparams);

	if (fmt.empty() && layout.labeltype == LABEL_COUNTER 
	    && !layout.counter.empty())
		fmt = "\\the" + layout.counter;

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
	if (i < end && !isNewline(i)) {
		++i;
		char_type previous_char = 0;
		char_type temp = 0;
		if (i < end) {
			previous_char = d->text_[i];
			if (!isNewline(i)) {
				++i;
				while (i < end && previous_char != ' ') {
					temp = d->text_[i];
					if (isNewline(i))
						break;
					++i;
					previous_char = temp;
				}
			}
		}
	}

	d->begin_of_body_ = i;
}


bool Paragraph::forceEmptyLayout() const
{
	Inset const * const inset = inInset();
	if (!inset)
		return true;
	return inset->forceEmptyLayout();
}


bool Paragraph::allowParagraphCustomization() const
{
	Inset const * const inset = inInset();
	if (!inset)
		return true;
	return inset->allowParagraphCustomization();
}


bool Paragraph::useEmptyLayout() const
{
	Inset const * const inset = inInset();
	if (!inset)
		return false;
	return inset->useEmptyLayout();
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


int Paragraph::Private::startTeXParParams(BufferParams const & bparams,
				 odocstream & os, TexRow & texrow,
				 bool moving_arg) const
{
	int column = 0;

	if (params_.noindent()) {
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
		if (owner_->getParLanguage(bparams)->babel() != "hebrew")
			output = corrected_env("\\begin", "flushleft", owner_->ownerCode());
		else
			output = corrected_env("\\begin", "flushright", owner_->ownerCode());
		os << from_ascii(output);
		adjust_row_column(output, texrow, column);
		break;
	} case LYX_ALIGN_RIGHT: {
		string output;
		if (owner_->getParLanguage(bparams)->babel() != "hebrew")
			output = corrected_env("\\begin", "flushright", owner_->ownerCode());
		else
			output = corrected_env("\\begin", "flushleft", owner_->ownerCode());
		os << from_ascii(output);
		adjust_row_column(output, texrow, column);
		break;
	} case LYX_ALIGN_CENTER: {
		string output;
		output = corrected_env("\\begin", "center", owner_->ownerCode());
		os << from_ascii(output);
		adjust_row_column(output, texrow, column);
		break;
	}
	}

	return column;
}


int Paragraph::Private::endTeXParParams(BufferParams const & bparams,
			       odocstream & os, TexRow & texrow,
			       bool moving_arg) const
{
	int column = 0;

	switch (params_.align()) {
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

	switch (params_.align()) {
	case LYX_ALIGN_NONE:
	case LYX_ALIGN_BLOCK:
	case LYX_ALIGN_LAYOUT:
	case LYX_ALIGN_SPECIAL:
		break;
	case LYX_ALIGN_LEFT: {
		string output;
		if (owner_->getParLanguage(bparams)->babel() != "hebrew")
			output = corrected_env("\n\\par\\end", "flushleft", owner_->ownerCode());
		else
			output = corrected_env("\n\\par\\end", "flushright", owner_->ownerCode());
		os << from_ascii(output);
		adjust_row_column(output, texrow, column);
		break;
	} case LYX_ALIGN_RIGHT: {
		string output;
		if (owner_->getParLanguage(bparams)->babel() != "hebrew")
			output = corrected_env("\n\\par\\end", "flushright", owner_->ownerCode());
		else
			output = corrected_env("\n\\par\\end", "flushleft", owner_->ownerCode());
		os << from_ascii(output);
		adjust_row_column(output, texrow, column);
		break;
	} case LYX_ALIGN_CENTER: {
		string output;
		output = corrected_env("\n\\par\\end", "center", owner_->ownerCode());
		os << from_ascii(output);
		adjust_row_column(output, texrow, column);
		break;
	}
	}

	return column;
}


// This one spits out the text of the paragraph
bool Paragraph::latex(BufferParams const & bparams,
				Font const & outerfont,
				odocstream & os, TexRow & texrow,
				OutputParams const & runparams) const
{
	LYXERR(Debug::LATEX, "SimpleTeXOnePar...     " << this);

	bool return_value = false;

	bool asdefault = forceEmptyLayout();

	Layout const & style = asdefault ?
		bparams.documentClass().emptyLayout() :
		*d->layout_;

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
		if (style.isCommand()) {
			os << '{';
			++column;
		}
		if (!asdefault)
			column += d->startTeXParParams(bparams, os, texrow,
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
			if (style.isCommand()) {
				os << '{';
				++column;
			}

			if (!asdefault)
				column += d->startTeXParParams(bparams, os,
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

		// Switch file encoding if necessary (and allowed)
		if (!runparams.verbatim && 
		    runparams.encoding->package() == Encoding::none &&
		    font.language()->encoding()->package() == Encoding::none) {
			pair<bool, int> const enc_switch = switchEncoding(os, bparams,
					runparams, *(font.language()->encoding()));
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
			// check if the fontchange ends with a trailing blank
			// (like "\small " (see bug 3382)
			if (suffixIs(fontchange, ' ') && c == ' ')
				os << fontchange.substr(0, fontchange.size() - 1) 
				   << from_ascii("{}");
			else
				os << fontchange;
		}

		if (c == ' ') {
			// FIXME: integrate this case in latexSpecialChar
			// Do not print the separation of the optional argument
			// if style.pass_thru is false. This works because
			// latexSpecialChar ignores spaces if
			// style.pass_thru is false.
			if (i != body_pos - 1) {
				if (d->simpleTeXBlanks(
						runparams, os, texrow,
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
		if (c == META_INSET)
			d->latexInset(bparams, os,
					texrow, rp, running_font,
					basefont, outerfont, open_font,
					runningChange, style, i, column);
		else {
			try {
				d->latexSpecialChar(os, rp, running_font, runningChange,
					style, i, column);
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
		column += d->endTeXParParams(bparams, os, texrow,
					  runparams.moving_arg);
	}

	LYXERR(Debug::LATEX, "SimpleTeXOnePar...done " << this);
	return return_value;
}


bool Paragraph::emptyTag() const
{
	for (pos_type i = 0; i < size(); ++i) {
		if (Inset const * inset = getInset(i)) {
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


pos_type Paragraph::firstWord(odocstream & os, OutputParams const & runparams)
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


bool Paragraph::isHfill(pos_type pos) const
{
	Inset const * inset = getInset(pos);
	return inset && (inset->lyxCode() == SPACE_CODE &&
			 inset->isStretchableSpace());
}


bool Paragraph::isNewline(pos_type pos) const
{
	Inset const * inset = getInset(pos);
	return inset && inset->lyxCode() == NEWLINE_CODE;
}


bool Paragraph::isLineSeparator(pos_type pos) const
{
	char_type const c = d->text_[pos];
	if (isLineSeparatorChar(c))
		return true;
	Inset const * inset = getInset(pos);
	return inset && inset->isLineSeparator();
}


/// Used by the spellchecker
bool Paragraph::isLetter(pos_type pos) const
{
	if (Inset const * inset = getInset(pos))
		return inset->isLetter();
	char_type const c = d->text_[pos];
	return isLetterChar(c) || isDigit(c);
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


docstring Paragraph::asString(bool label) const
{
	return asString(0, size(), label);
}


docstring Paragraph::asString(pos_type beg, pos_type end, bool label) const
{

	odocstringstream os;

	if (beg == 0 && label && !d->params_.labelString().empty())
		os << d->params_.labelString() << ' ';

	for (pos_type i = beg; i < end; ++i) {
		char_type const c = d->text_[i];
		if (isPrintable(c))
			os.put(c);
		else if (c == META_INSET)
			getInset(i)->textString(os);
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


Layout const & Paragraph::layout() const
{
	return *d->layout_;
}


void Paragraph::setLayout(Layout const & layout)
{
	d->layout_ = &layout;
}


void Paragraph::setEmptyOrDefaultLayout(DocumentClass const & tclass)
{
	if (useEmptyLayout())
		setLayout(tclass.emptyLayout());
	else
		setLayout(tclass.defaultLayout());
}


Inset * Paragraph::inInset() const
{
	return d->inset_owner_;
}


InsetCode Paragraph::ownerCode() const
{
	return d->inset_owner_ ? d->inset_owner_->lyxCode() : NO_CODE;
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


char_type Paragraph::transformChar(char_type c, pos_type pos) const
{
	if (!Encodings::isArabicChar(c))
		return c;

	char_type prev_char = ' ';
	char_type next_char = ' ';

	for (pos_type i = pos - 1; i >= 0; --i) {
		char_type const par_char = d->text_[i];
		if (!Encodings::isArabicComposeChar(par_char)) {
			prev_char = par_char;
			break;
		}
	}

	for (pos_type i = pos + 1, end = size(); i < end; ++i) {
		char_type const par_char = d->text_[i];
		if (!Encodings::isArabicComposeChar(par_char)) {
			next_char = par_char;
			break;
		}
	}

	if (Encodings::isArabicChar(next_char)) {
		if (Encodings::isArabicChar(prev_char) &&
			!Encodings::isArabicSpecialChar(prev_char))
			return Encodings::transformChar(c, Encodings::FORM_MEDIAL);
		else
			return Encodings::transformChar(c, Encodings::FORM_INITIAL);
	} else {
		if (Encodings::isArabicChar(prev_char) &&
			!Encodings::isArabicSpecialChar(prev_char))
			return Encodings::transformChar(c, Encodings::FORM_FINAL);
		else
			return Encodings::transformChar(c, Encodings::FORM_ISOLATED);
	}
}


int Paragraph::checkBiblio(Buffer const & buffer)
{
	// FIXME From JS:
	// This is getting more and more a mess. ...We really should clean
	// up this bibitem issue for 1.6. See also bug 2743.

	// Add bibitem insets if necessary
	if (d->layout_->labeltype != LABEL_BIBLIO)
		return 0;

	bool hasbibitem = !d->insetlist_.empty()
		// Insist on it being in pos 0
		&& d->text_[0] == META_INSET
		&& d->insetlist_.begin()->inset->lyxCode() == BIBITEM_CODE;

	bool track_changes = buffer.params().trackChanges;

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

	// There was an InsetBibitem at the beginning, and we didn't
	// have to erase one.
	if (hasbibitem && erasedInsetPosition < 0)
			return 0;

	// There was an InsetBibitem at the beginning and we did have to
	// erase one. So we give its properties to the beginning inset.
	if (hasbibitem) {
		InsetBibitem * inset =
			static_cast<InsetBibitem *>(d->insetlist_.begin()->inset);
		if (!oldkey.empty())
			inset->setParam("key", oldkey);
		inset->setParam("label", oldlabel);
		return -erasedInsetPosition;
	}

	// There was no inset at the beginning, so we need to create one with
	// the key and label of the one we erased.
	InsetBibitem * inset = new InsetBibitem(InsetCommandParams(BIBITEM_CODE));
	inset->setBuffer(const_cast<Buffer &>(buffer));
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
	docstring changes;

	bool const trackChanges = bparams.trackChanges;

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

		if (!isLetter(pos) || isDeleted(pos)) {
			// permit capitalization again
			capitalize = true;
		}

		if (oldChar != newChar)
			changes += newChar;

		if (oldChar == newChar || pos == right - 1) {
			if (oldChar != newChar) {
				// step behind the changing area
				pos++;
			}
			int erasePos = pos - changes.size();
			for (size_t i = 0; i < changes.size(); i++) {
				insertChar(pos, changes[i],
					getFontSettings(bparams,
					erasePos),
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
}


bool Paragraph::find(docstring const & str, bool cs, bool mw,
		pos_type pos, bool del) const
{
	int const strsize = str.length();
	int i = 0;
	pos_type const parsize = d->text_.size();
	for (i = 0; pos + i < parsize; ++i) {
		if (i >= strsize)
			break;
		if (cs && str[i] != d->text_[pos + i])
			break;
		if (!cs && uppercase(str[i]) != uppercase(d->text_[pos + i]))
			break;
		if (!del && isDeleted(pos + i))
			break;
	}

	if (i != strsize)
		return false;

	// if necessary, check whether string matches word
	if (mw) {
		if (pos > 0 && isLetter(pos - 1))
			return false;
		if (pos + strsize < parsize
			&& isLetter(pos + strsize))
			return false;
	}

	return true;
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
	Private::Words::const_iterator it;
	WordList & wl = theWordList();
	for (it = d->words_.begin(); it != d->words_.end(); ++it)
		wl.remove(*it);
	d->words_.clear();
}


void Paragraph::collectWords(CursorSlice const & sl)
{
	// find new words
	bool inword = false;

	//lyxerr << "Words: ";
	pos_type n = size();
	for (pos_type pos = 0; pos != n; ++pos) {
		if (isDeleted(pos))
			continue;

		if (!isLetter(pos)) {
			inword = false;
			continue;
		}

		if (inword)
			continue;

		inword = true;
		CursorSlice from = sl;
		CursorSlice to = sl;
		from.pos() = pos;
		to.pos() = pos;
		from.text()->getWord(from, to, WHOLE_WORD);
		if (to.pos() - from.pos() < 6)
			continue;
		docstring word = asString(from.pos(), to.pos(), false);
		d->words_.insert(word);
		//lyxerr << word << " ";
	}
	//lyxerr << std::endl;
}


void Paragraph::registerWords()
{
	Private::Words::const_iterator it;
	WordList & wl = theWordList();
	for (it = d->words_.begin(); it != d->words_.end(); ++it)
		wl.insert(*it);
}


void Paragraph::updateWords(CursorSlice const & sl)
{
	LASSERT(&sl.paragraph() == this, /**/);
	deregisterWords();
	collectWords(sl);
	registerWords();
}

} // namespace lyx
