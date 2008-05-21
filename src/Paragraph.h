// -*- C++ -*-
/**
 * \file Paragraph.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup
 * \author Lars Gullik Bjønnes
 * \author John Levon
 * \author André Pönitz
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef PARAGRAPH_H
#define PARAGRAPH_H

#include "Changes.h"
#include "Dimension.h"
#include "InsetList.h"
#include "lyxlayout_ptr_fwd.h"
#include "Row.h"

#include "insets/Inset.h" // only for Inset::Code


namespace lyx {


class Buffer;
class BufferParams;
class Counters;
class Inset;
class InsetBibitem;
class LaTeXFeatures;
class Inset_code;
class Language;
class Font;
class Font_size;
class MetricsInfo;
class OutputParams;
class PainterInfo;
class ParagraphParameters;
class TexRow;


class FontSpan {
public:
	/// Invalid font span containing no character
	FontSpan() : first(0), last(-1) {}
	/// Span including first and last
	FontSpan(pos_type f, pos_type l) : first(f), last(l) {}

public:
	/// Range including first and last.
	pos_type first, last;
};


/// A Paragraph holds all text, attributes and insets in a text paragraph
/// \todo FIXME: any reference to ParagraphMetrics (including inheritance)
/// should go in order to complete the Model/View separation of this class.
class Paragraph  {
public:
	///
	enum {
		/// Note that this is 1 right now to avoid
		/// crashes where getChar() is called wrongly
		/// (returning 0) - if this was 0, then we'd
		/// try getInset() and crash. We should fix
		/// all these places.
		//META_INSET = 1 // as in trunk
		META_INSET = 0x200001  // above 0x10ffff, for ucs-4
	};
	///
	typedef char_type value_type;
	///
	typedef std::vector<value_type> TextContainer;

	///
	Paragraph();
	///
	Paragraph(Paragraph const &);
	///
	Paragraph & operator=(Paragraph const &);
	///
	~Paragraph();
	///
	int id() const;


	///
	Language const * getParLanguage(BufferParams const &) const;
	///
	bool isRightToLeftPar(BufferParams const &) const;
	///
	void changeLanguage(BufferParams const & bparams,
			    Language const * from, Language const * to);
	///
	bool isMultiLingual(BufferParams const &) const;

	///
	docstring const printableString(bool label) const;
	///
	docstring const asString(Buffer const &, bool label) const;
	///
	docstring const asString(Buffer const & buffer,
				   pos_type beg,
				   pos_type end,
				   bool label) const;

	///
	void write(Buffer const &, std::ostream &, BufferParams const &,
		   depth_type & depth) const;
	///
	void validate(LaTeXFeatures &) const;

	///
	int startTeXParParams(BufferParams const &, odocstream &, TexRow &,
			      bool) const;

	///
	int endTeXParParams(BufferParams const &, odocstream &, TexRow &,
			    bool) const;


	///
	bool simpleTeXOnePar(Buffer const &, BufferParams const &,
			     Font const & outerfont, odocstream &,
			     TexRow & texrow, OutputParams const &) const;

	/// Can we drop the standard paragraph wrapper?
	bool emptyTag() const;

	/// Get the id of the paragraph, usefull for docbook
	std::string getID(Buffer const & buf,
			  OutputParams const & runparams) const;

	/// Get the first word of a paragraph, return the position where it left
	pos_type getFirstWord(Buffer const & buf,
				   odocstream & os,
				   OutputParams const & runparams) const;

	/// Checks if the paragraph contains only text and no inset or font change.
	bool onlyText(Buffer const & buf, Font const & outerfont,
		      pos_type initial) const;

	/// Writes to stream the docbook representation
	void simpleDocBookOnePar(Buffer const & buf,
				 odocstream &,
				 OutputParams const & runparams,
				 Font const & outerfont,
				 pos_type initial = 0) const;

	///
	bool hasSameLayout(Paragraph const & par) const;

	///
	void makeSameLayout(Paragraph const & par);

	///
	void setInsetOwner(Inset * inset);
	///
	Inset * inInset() const;
	///
	Inset::Code ownerCode() const;
	///
	bool forceDefaultParagraphs() const;

	///
	pos_type size() const { return text_.size(); }
	///
	bool empty() const { return text_.empty(); }

	///
	Layout_ptr const & layout() const;
	///
	void layout(Layout_ptr const & new_layout);

	/// This is the item depth, only used by enumerate and itemize
	signed char itemdepth;

	///
	InsetBibitem * bibitem() const;  // ale970302

	/// look up change at given pos
	Change const & lookupChange(pos_type pos) const;

	/// is there a change within the given range ?
	bool isChanged(pos_type start, pos_type end) const;
	/// is there an unchanged char at the given pos ?
	bool isUnchanged(pos_type pos) const {
		return lookupChange(pos).type == Change::UNCHANGED;
	}
	/// is there an insertion at the given pos ?
	bool isInserted(pos_type pos) const {
		return lookupChange(pos).type == Change::INSERTED;
	}
	/// is there a deletion at the given pos ?
	bool isDeleted(pos_type pos) const {
		return lookupChange(pos).type == Change::DELETED;
	}

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

	/// Paragraphs can contain "manual labels", for example, Description
	/// environment. The text for this user-editable label is stored in
	/// the paragraph alongside the text of the rest of the paragraph
	/// (the body). This function returns the starting position of the
	/// body of the text in the paragraph.
	pos_type beginOfBody() const;
	/// recompute this value
	void setBeginOfBody();

	///
	docstring const & getLabelstring() const;

	/// the next two functions are for the manual labels
	docstring const getLabelWidthString() const;
	/// Set label width string.
	void setLabelWidthString(docstring const & s);
	/// translate \p label to the paragraph language if possible.
	docstring const translateIfPossible(docstring const & label,
		BufferParams const & bparams) const;
	/// Expand the counters for the labelstring of \c layout
	docstring expandLabel(Layout_ptr const &, BufferParams const &,
		bool process_appendix = true) const;
	/// Actual paragraph alignment used
	char getAlign() const;
	/// The nesting depth of a paragraph
	depth_type getDepth() const;
	/// The maximal possible depth of a paragraph after this one
	depth_type getMaxDepthAfter() const;
	///
	void applyLayout(Layout_ptr const & new_layout);

	/// (logically) erase the char at pos; return true if it was actually erased
	bool eraseChar(pos_type pos, bool trackChanges);
	/// (logically) erase the given range; return the number of chars actually erased
	int eraseChars(pos_type start, pos_type end, bool trackChanges);

	/** Get uninstantiated font setting. Returns the difference
	    between the characters font and the layoutfont.
	    This is what is stored in the fonttable
	*/
	Font const
	getFontSettings(BufferParams const &, pos_type pos) const;
	///
	Font const getFirstFontSettings(BufferParams const &) const;

	/** Get fully instantiated font. If pos == -1, use the layout
	    font attached to this paragraph.
	    If pos == -2, use the label font of the layout attached here.
	    In all cases, the font is instantiated, i.e. does not have any
	    attributes with values Font::INHERIT, Font::IGNORE or
	    Font::TOGGLE.
	*/
	Font const getFont(BufferParams const &, pos_type pos,
			      Font const & outerfont) const;
	Font const getLayoutFont(BufferParams const &,
				    Font const & outerfont) const;
	Font const getLabelFont(BufferParams const &,
				   Font const & outerfont) const;
	/**
	 * The font returned by the above functions is the same in a
	 * span of characters. This method will return the first and
	 * the last positions in the paragraph for which that font is
	 * the same. This can be used to avoid unnecessary calls to getFont.
	 */
	FontSpan fontSpan(pos_type pos) const;
	///
	/// this is a bottleneck.
	value_type getChar(pos_type pos) const { return text_[pos]; }
	/// Get the char, but mirror all bracket characters if it is right-to-left
	value_type getUChar(BufferParams const &, pos_type pos) const;
	/// pos <= size() (there is a dummy font change at the end of each par)
	void setFont(pos_type pos, Font const & font);
	/// Returns the height of the highest font in range
	Font_size highestFontInRange(pos_type startpos,
					pos_type endpos, Font_size def_size) const;
	///
	void insert(pos_type pos, docstring const & str,
		    Font const & font, Change const & change);
	///
	void insertChar(pos_type pos, value_type c, bool trackChanges);
	///
	void insertChar(pos_type pos, value_type c,
			Font const &, bool trackChanges);
	///
	void insertChar(pos_type pos, value_type c,
			Font const &, Change const & change);
	///
	void insertInset(pos_type pos, Inset * inset,
			 Change const & change);
	///
	void insertInset(pos_type pos, Inset * inset,
			 Font const &, Change const & change);
	///
	bool insetAllowed(Inset_code code);
	///
	Inset * getInset(pos_type pos) {
		return insetlist.get(pos);
	}
	///
	Inset const * getInset(pos_type pos) const {
		return insetlist.get(pos);
	}

	///
	bool isHfill(pos_type pos) const {
		return isInset(pos)
		       && getInset(pos)->lyxCode() == Inset::HFILL_CODE;
	}
	/// hinted by profiler
	bool isInset(pos_type pos) const {
		return getChar(pos) == static_cast<value_type>(META_INSET);
	}
	///
	bool isNewline(pos_type pos) const;
	/// return true if the char is a word separator
	bool isSeparator(pos_type pos) const { return getChar(pos) == ' '; }
	///
	bool isLineSeparator(pos_type pos) const;
	/// True if the character/inset at this point can be part of a word.
	/// Note that digits in particular are considered as letters
	bool isLetter(pos_type pos) const;

	/// returns -1 if inset not found
	int getPositionOfInset(Inset const * inset) const;

	/// returns true if at least one line break or line separator has been deleted
	/// at the beginning of the paragraph (either physically or logically)
	bool stripLeadingSpaces(bool trackChanges);

	/// return true if we allow multiple spaces
	bool isFreeSpacing() const;

	/// return true if we allow this par to stay empty
	bool allowEmpty() const;
	///
	char_type transformChar(char_type c, pos_type pos) const;
	///
	ParagraphParameters & params();
	///
	ParagraphParameters const & params() const;
	///
	bool hfillExpansion(Row const & row, pos_type pos) const;

	/// Check if we are in a Biblio environment and insert or
	/// delete InsetBibitems as necessary.
	/// \retval int 1, if we had to add an inset, in which case
	/// the cursor will need to move cursor forward; -pos, if we deleted
	/// an inset, in which case pos is the position from which the inset
	/// was deleted, and the cursor will need to be moved back one if it
	/// was previously past that position. Return 0 otherwise.
	int checkBiblio(bool track_changes);

	/// For each author, set 'used' to true if there is a change
	/// by this author in the paragraph.
	void checkAuthors(AuthorList const & authorList);

public:
	///
	InsetList insetlist;

private:

	///
	Layout_ptr layout_;
	/**
	 * Keeping this here instead of in the pimpl makes LyX >10% faster
	 * for average tasks as buffer loading/switching etc.
	 */
	TextContainer text_;
	/// end of label
	pos_type begin_of_body_;

	/// Pimpl away stuff
	class Pimpl;
	///
	friend class Paragraph::Pimpl;
	///
	Pimpl * pimpl_;
};

} // namespace lyx

#endif // PARAGRAPH_H
