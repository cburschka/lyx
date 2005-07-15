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

#include "support/types.h"

#include <boost/assert.hpp>

#include <string>
#include <utility>

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
		META_INSET = 1
	};
	///
	typedef char value_type;
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
	std::string const asString(Buffer const &,
				   OutputParams const & runparams,
				   bool label) const;
	///
	std::string const asString(Buffer const &, bool label) const;
	///
	std::string const Paragraph::asString(Buffer const & buffer,
					      lyx::pos_type beg,
					      lyx::pos_type end,
					      bool label) const;
	///
	std::string const asString(Buffer const &,
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
	int startTeXParParams(BufferParams const &, std::ostream &, bool) const;

	///
	int endTeXParParams(BufferParams const &, std::ostream &, bool) const;


	///
	bool simpleTeXOnePar(Buffer const &, BufferParams const &,
			     LyXFont const & outerfont, std::ostream &,
			     TexRow & texrow, OutputParams const &) const;

	/// Writes to stream the content of the paragraph for linuxdoc
	void simpleLinuxDocOnePar(Buffer const & buf,
				  std::ostream & os,
				  LyXFont const & outerfont,
				  OutputParams const & runparams,
				  lyx::depth_type depth) const;

	/// Can we drop the standard paragraph wrapper?
	bool emptyTag() const;

	/// Get the id of the paragraph, usefull for docbook and linuxdoc
	std::string getID(Buffer const & buf,
			  OutputParams const & runparams) const;

	// Get the first word of a paragraph, return the position where it left
	lyx::pos_type getFirstWord(Buffer const & buf,
				   std::ostream & os,
				   OutputParams const & runparams) const;

	/// Checks if the paragraph contains only text and no inset or font change.
	bool onlyText(Buffer const & buf, LyXFont const & outerfont,
		      lyx::pos_type initial) const;

	/// Writes to stream the docbook representation
	void simpleDocBookOnePar(Buffer const & buf,
				 std::ostream &,
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

	/// initialise tracking for this par
	void trackChanges(Change::Type = Change::UNCHANGED);

	/// stop tracking
	void untrackChanges();

	/// set entire paragraph to new text for change tracking
	void cleanChanges();

	/// look up change type at given pos
	Change::Type lookupChange(lyx::pos_type pos) const;

	/// look up change at given pos
	Change const lookupChangeFull(lyx::pos_type pos) const;

	/// is there a change within the given range ?
	bool isChanged(lyx::pos_type start, lyx::pos_type end) const;

	/// is there a non-addition in this range ?
	bool isChangeEdited(lyx::pos_type start, lyx::pos_type end) const;

	/// set change at pos
	void setChange(lyx::pos_type pos, Change::Type type);

	/// accept change
	void acceptChange(lyx::pos_type start, lyx::pos_type end);

	/// reject change
	void rejectChange(lyx::pos_type start, lyx::pos_type end);

	/// mark whole par as erased
	void markErased();

	/// Paragraphs can contain "manual labels", for example, Description
	/// environment. The text for this user-editable label is stored in
	/// the paragraph alongside the text of the rest of the paragraph
	/// (the body). This function returns the starting position of the
	/// body of the text in the paragraph.
	lyx::pos_type beginOfBody() const;
	/// recompute this value
	void setBeginOfBody();

	///
	std::string const & getLabelstring() const;

	/// the next two functions are for the manual labels
	std::string const getLabelWidthString() const;
	///
	void setLabelWidthString(std::string const & s);
	///
	char getAlign() const;
	/// The nesting depth of a paragraph
	depth_type getDepth() const;
	/// The maximal possible depth of a paragraph after this one
	depth_type getMaxDepthAfter() const;
	///
	void applyLayout(LyXLayout_ptr const & new_layout);

	/// definite erase
	void eraseIntern(lyx::pos_type pos);
	/// erase the char at the given position
	bool erase(lyx::pos_type pos);
	/// erase the given range. Returns the number of chars actually erased
	int erase(lyx::pos_type start, lyx::pos_type end);

	/** Get uninstantiated font setting. Returns the difference
	    between the characters font and the layoutfont.
	    This is what is stored in the fonttable
	*/
	LyXFont const
	getFontSettings(BufferParams const &, lyx::pos_type pos) const;
	///
	LyXFont const getFirstFontSettings() const;

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
	 * the last last positions in the paragraph for which that
	 * font is the same. This can be used to avoid unnecessary
	 * calls to getFont.
	 */
	std::pair<lyx::pos_type, lyx::pos_type> getFontSpan(lyx::pos_type pos) const;
	///
	/// this is a bottleneck.
	value_type getChar(lyx::pos_type pos) const
	{
		BOOST_ASSERT(pos >= 0);
		BOOST_ASSERT(pos < int(text_.size()));
		return text_[pos];
	}
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
		    LyXFont const & font);
	///
	void insertChar(lyx::pos_type pos, value_type c,
			Change change = Change(Change::INSERTED));
	///
	void insertChar(lyx::pos_type pos, value_type c,
		LyXFont const &, Change change = Change(Change::INSERTED));
	///
	bool checkInsertChar(LyXFont &);
	///
	void insertInset(lyx::pos_type pos, InsetBase * inset,
			 Change change = Change(Change::INSERTED));
	///
	void insertInset(lyx::pos_type pos, InsetBase * inset,
		LyXFont const &, Change change = Change(Change::INSERTED));
	///
	bool insetAllowed(InsetBase_code code);
	///
	InsetBase * getInset(lyx::pos_type pos);
	///
	InsetBase const * getInset(lyx::pos_type pos) const;
	///
	InsetList insetlist;


	///
	bool isHfill(lyx::pos_type pos) const;
	/// hinted by profiler
	bool isInset(lyx::pos_type pos) const { return getChar(pos) == static_cast<value_type>(META_INSET); }
	///
	bool isNewline(lyx::pos_type pos) const;
	///
	bool isSeparator(lyx::pos_type pos) const;
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
	unsigned char transformChar(unsigned char c, lyx::pos_type pos) const;
	///
	ParagraphParameters & params();
	///
	ParagraphParameters const & params() const;

	///
	Row & getRow(lyx::pos_type pos);
	///
	Row const & getRow(lyx::pos_type pos) const;
	///
	size_t pos2row(lyx::pos_type pos) const;

	/// total height of paragraph
	unsigned int height() const { return dim_.height(); }
	/// total width of paragraph, may differ from workwidth
	unsigned int width() const { return dim_.width(); }
	unsigned int ascent() const { return dim_.ascent(); }
	unsigned int descent() const { return dim_.descent(); }
	/// LyXText updates the rows using this access point
	RowList & rows() { return rows_; }
	/// The painter and others use this
	RowList const & rows() const { return rows_; }

	/// LyXText::redoParagraph updates this
	Dimension & dim() { return dim_; }

	/// dump some information to lyxerr
	void dump() const;
private:
	/// cached dimensions of paragraph
	Dimension dim_;

	///
	mutable RowList rows_;
	///
	LyXLayout_ptr layout_;
	/**
	 * Keeping this here instead of in the pimpl makes LyX >10% faster
	 * for average tasks as buffer loading/switching etc.
	 */
	TextContainer text_;
	/// end of label
	lyx::pos_type begin_of_body_;

	class Pimpl;
	///
	friend class Paragraph::Pimpl;
	///
	Pimpl * pimpl_;
};


inline bool isInsertedText(Paragraph const & par, lyx::pos_type pos)
{
	return par.lookupChange(pos) == Change::INSERTED;
}


inline bool isDeletedText(Paragraph const & par, lyx::pos_type pos)
{
	return par.lookupChange(pos) == Change::DELETED;
}

#endif // PARAGRAPH_H
