// -*- C++ -*-
/**
 * \file paragraph.h
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

#include "changes.h"
#include "dimension.h"
#include "InsetList.h"
#include "lyxlayout_ptr_fwd.h"
#include "RowList_fwd.h"

#include "insets/insetbase.h" // only for InsetBase::Code


class Buffer;
class BufferParams;
class BufferView;
class Counters;
class InsetBase;
class InsetBibitem;
class LaTeXFeatures;
class InsetBase_code;
class Language;
class LyXFont;
class LyXFont_size;
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
	FontSpan(lyx::pos_type f, lyx::pos_type l) : first(f), last(l) {}

public:
	/// Range including first and last.
	lyx::pos_type first, last;
};


/// A Paragraph holds all text, attributes and insets in a text paragraph
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
	typedef lyx::char_type value_type;
	///
	typedef lyx::depth_type depth_type;
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
	lyx::docstring const asString(Buffer const &,
				   OutputParams const & runparams,
				   bool label) const;
	///
	lyx::docstring const asString(Buffer const &, bool label) const;
	///
	lyx::docstring const asString(Buffer const & buffer,
				   lyx::pos_type beg,
				   lyx::pos_type end,
				   bool label) const;
	///
	lyx::docstring const asString(Buffer const &,
				   OutputParams const & runparams,
				   lyx::pos_type beg,
				   lyx::pos_type end,
				   bool label) const;

	///
	void write(Buffer const &, std::ostream &, BufferParams const &,
		   depth_type & depth) const;
	///
	void validate(LaTeXFeatures &) const;

	///
	int startTeXParParams(BufferParams const &, lyx::odocstream &, bool) const;

	///
	int endTeXParParams(BufferParams const &, lyx::odocstream &, bool) const;


	///
	bool simpleTeXOnePar(Buffer const &, BufferParams const &,
			     LyXFont const & outerfont, lyx::odocstream &,
			     TexRow & texrow, OutputParams const &) const;

	/// Can we drop the standard paragraph wrapper?
	bool emptyTag() const;

	/// Get the id of the paragraph, usefull for docbook
	std::string getID(Buffer const & buf,
			  OutputParams const & runparams) const;

	// Get the first word of a paragraph, return the position where it left
	lyx::pos_type getFirstWord(Buffer const & buf,
				   lyx::odocstream & os,
				   OutputParams const & runparams) const;

	/// Checks if the paragraph contains only text and no inset or font change.
	bool onlyText(Buffer const & buf, LyXFont const & outerfont,
		      lyx::pos_type initial) const;

	/// Writes to stream the docbook representation
	void simpleDocBookOnePar(Buffer const & buf,
				 lyx::odocstream &,
				 OutputParams const & runparams,
				 LyXFont const & outerfont,
				 lyx::pos_type initial = 0) const;

	///
	bool hasSameLayout(Paragraph const & par) const;

	///
	void makeSameLayout(Paragraph const & par);

	///
	void setInsetOwner(InsetBase * inset);
	///
	InsetBase * inInset() const;
	///
	InsetBase::Code ownerCode() const;
	///
	bool forceDefaultParagraphs() const;

	///
	lyx::pos_type size() const { return text_.size(); }
	///
	bool empty() const { return text_.empty(); }
	///
	void setContentsFromPar(Paragraph const & par);
	///
	void clearContents();

	///
	LyXLayout_ptr const & layout() const;
	///
	void layout(LyXLayout_ptr const & new_layout);

	/// This is the item depth, only used by enumerate and itemize
	signed char itemdepth;

	///
	InsetBibitem * bibitem() const;  // ale970302

	/// look up change at given pos
	Change const lookupChange(lyx::pos_type pos) const;

	/// is there a change within the given range ?
	bool isChanged(lyx::pos_type start, lyx::pos_type end) const;
	/// is there a deletion at the given pos ?
	bool isDeleted(lyx::pos_type pos) const {
		return lookupChange(pos).type == Change::DELETED;
	}

	/// set change for the entire par
	void setChange(Change const & change);

	/// set change at given pos
	void setChange(lyx::pos_type pos, Change const & change);

	/// accept change
	void acceptChange(lyx::pos_type start, lyx::pos_type end);

	/// reject change
	void rejectChange(lyx::pos_type start, lyx::pos_type end);

	/// Paragraphs can contain "manual labels", for example, Description
	/// environment. The text for this user-editable label is stored in
	/// the paragraph alongside the text of the rest of the paragraph
	/// (the body). This function returns the starting position of the
	/// body of the text in the paragraph.
	lyx::pos_type beginOfBody() const;
	/// recompute this value
	void setBeginOfBody();

	///
	lyx::docstring const & getLabelstring() const;

	/// the next two functions are for the manual labels
	lyx::docstring const getLabelWidthString() const;
	///
	void setLabelWidthString(lyx::docstring const & s);
	///
	char getAlign() const;
	/// The nesting depth of a paragraph
	depth_type getDepth() const;
	/// The maximal possible depth of a paragraph after this one
	depth_type getMaxDepthAfter() const;
	///
	void applyLayout(LyXLayout_ptr const & new_layout);

	/// erase the char at the given position
	bool erase(lyx::pos_type pos, bool trackChanges);
	/// erase the given range. Returns the number of chars actually erased
	int erase(lyx::pos_type start, lyx::pos_type end, bool trackChanges);

	/** Get uninstantiated font setting. Returns the difference
	    between the characters font and the layoutfont.
	    This is what is stored in the fonttable
	*/
	LyXFont const
	getFontSettings(BufferParams const &, lyx::pos_type pos) const;
	///
	LyXFont const getFirstFontSettings(BufferParams const &) const;

	/** Get fully instantiated font. If pos == -1, use the layout
	    font attached to this paragraph.
	    If pos == -2, use the label font of the layout attached here.
	    In all cases, the font is instantiated, i.e. does not have any
	    attributes with values LyXFont::INHERIT, LyXFont::IGNORE or
	    LyXFont::TOGGLE.
	*/
	LyXFont const getFont(BufferParams const &, lyx::pos_type pos,
			      LyXFont const & outerfont) const;
	LyXFont const getLayoutFont(BufferParams const &,
				    LyXFont const & outerfont) const;
	LyXFont const getLabelFont(BufferParams const &,
				   LyXFont const & outerfont) const;
	/**
	 * The font returned by the above functions is the same in a
	 * span of characters. This method will return the first and
	 * the last positions in the paragraph for which that font is
	 * the same. This can be used to avoid unnecessary calls to
   * getFont.
	 */
	FontSpan fontSpan(lyx::pos_type pos) const;
	///
	/// this is a bottleneck.
	value_type getChar(lyx::pos_type pos) const { return text_[pos]; }
	/// Get the char, but mirror all bracket characters if it is right-to-left
	value_type getUChar(BufferParams const &, lyx::pos_type pos) const;
	/// The position must already exist.
	void setChar(lyx::pos_type pos, value_type c);
	/// pos <= size() (there is a dummy font change at the end of each par)
	void setFont(lyx::pos_type pos, LyXFont const & font);
	/// Returns the height of the highest font in range
	LyXFont_size highestFontInRange(lyx::pos_type startpos,
					lyx::pos_type endpos,
					LyXFont_size def_size) const;
	///
	void insert(lyx::pos_type pos, std::string const & str,
	            LyXFont const & font, Change const & change);
	///
	void insertChar(lyx::pos_type pos, value_type c, bool trackChanges);
	///
	void insertChar(lyx::pos_type pos, value_type c,
	                LyXFont const &, bool trackChanges);
	///
	void insertChar(lyx::pos_type pos, value_type c,
	                LyXFont const &, Change const & change);
	///
	void insertInset(lyx::pos_type pos, InsetBase * inset,
			 Change const & change);
	///
	void insertInset(lyx::pos_type pos, InsetBase * inset,
		         LyXFont const &, Change const & change);
	///
	bool insetAllowed(InsetBase_code code);
	///
	InsetBase * getInset(lyx::pos_type pos) {
		return insetlist.get(pos);
	}
	///
	InsetBase const * getInset(lyx::pos_type pos) const {
		return insetlist.get(pos);
	}

	///
	bool isHfill(lyx::pos_type pos) const {
		return isInset(pos)
		       && getInset(pos)->lyxCode() == InsetBase::HFILL_CODE;
	}
	/// hinted by profiler
	bool isInset(lyx::pos_type pos) const {
		return getChar(pos) == static_cast<value_type>(META_INSET);
	}
	///
	bool isNewline(lyx::pos_type pos) const;
	/// return true if the char is a word separator
	bool isSeparator(lyx::pos_type pos) const { return getChar(pos) == ' '; }
	///
	bool isLineSeparator(lyx::pos_type pos) const;
	/// True if the character/inset at this point can be part of a word
	// Note that digits in particular are considered as letters
	bool isLetter(lyx::pos_type pos) const;

	/// returns -1 if inset not found
	int getPositionOfInset(InsetBase const * inset) const;

	/// Returns the number of line breaks and white-space stripped at the start
	int stripLeadingSpaces();

	/// return true if we allow multiple spaces
	bool isFreeSpacing() const;

	/// return true if we allow this par to stay empty
	bool allowEmpty() const;
	///
	lyx::char_type transformChar(lyx::char_type c, lyx::pos_type pos) const;
	///
	ParagraphParameters & params();
	///
	ParagraphParameters const & params() const;

	///
	Row & getRow(lyx::pos_type pos, bool boundary);
	///
	Row const & getRow(lyx::pos_type pos, bool boundary) const;
	///
	size_t pos2row(lyx::pos_type pos) const;

	/// total height of paragraph
	unsigned int height() const { return dim_.height(); }
	/// total width of paragraph, may differ from workwidth
	unsigned int width() const { return dim_.width(); }
	/// ascend of paragraph above baseline
	unsigned int ascent() const { return dim_.ascent(); }
	/// descend of paragraph below baseline
	unsigned int descent() const { return dim_.descent(); }
	/// LyXText updates the rows using this access point
	RowList & rows() { return rows_; }
	/// The painter and others use this
	RowList const & rows() const { return rows_; }
	///
	RowSignature & rowSignature() const { return rowSignature_; }

	/// LyXText::redoParagraph updates this
	Dimension & dim() { return dim_; }

	/// dump some information to lyxerr
	void dump() const;

public:
	///
	InsetList insetlist;

private:
	/// cached dimensions of paragraph
	Dimension dim_;

	///
	mutable RowList rows_;
	///
	mutable RowSignature rowSignature_;

	///
	LyXLayout_ptr layout_;
	/**
	 * Keeping this here instead of in the pimpl makes LyX >10% faster
	 * for average tasks as buffer loading/switching etc.
	 */
	TextContainer text_;
	/// end of label
	lyx::pos_type begin_of_body_;

	/// Pimpl away stuff
	class Pimpl;
	///
	friend class Paragraph::Pimpl;
	///
	Pimpl * pimpl_;
};

#endif // PARAGRAPH_H
