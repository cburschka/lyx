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

#include "lyxlayout_ptr_fwd.h"
#include "lyxfont.h" // Just for LyXFont::FONT_SIZE
#include "InsetList.h"

#include "insets/inset.h" // Just for InsetOld::Code

#include "support/types.h"
#include "changes.h"
#include "RowList_fwd.h"

#include "support/std_string.h"

class Buffer;
class BufferParams;
class BufferView;
class Counters;
class InsetBibitem;
class Language;
class LaTeXFeatures;
class LatexRunParams;
class ParagraphParameters;
class TexRow;
class UpdatableInset;

/// A Paragraph holds all text, attributes and insets in a text paragraph
class Paragraph  {
public:
	///
	enum META_KIND {
		/// Note that this is 1 right now to avoid
		/// crashes where getChar() is called wrongly
		/// (returning 0) - if this was 0, then we'd
		/// try getInset() and crash. We should fix
		/// all these places.
		META_INSET = 1
	};
	///
	typedef char value_type;
	/// The same as ParameterStruct::depth_type
	typedef unsigned int depth_type;

	///
	Paragraph();

	///
	Paragraph(Paragraph const &);
	///
	void operator=(Paragraph const &);
	/// the destructor removes the new paragraph from the list
	~Paragraph();

	///
	Language const * getParLanguage(BufferParams const &) const;
	///
	bool isRightToLeftPar(BufferParams const &) const;
	///
	void changeLanguage(BufferParams const & bparams,
			    Language const * from, Language const * to);
	///
	bool isMultiLingual(BufferParams const &);

	///
	string const asString(Buffer const &, bool label) const;
	///
	string const asString(Buffer const &, lyx::pos_type beg, lyx::pos_type end,
			      bool label) const;

	///
	void write(Buffer const &, std::ostream &, BufferParams const &,
		       depth_type & depth) const;
	///
	void validate(LaTeXFeatures &) const;

	/// return the unique ID of this paragraph
	int id() const;
	/// Set the Id of this paragraph.
	void id(int);

	///
	int startTeXParParams(BufferParams const &, std::ostream &, bool) const;

	///
	int endTeXParParams(BufferParams const &, std::ostream &, bool) const;


	///
	bool simpleTeXOnePar(Buffer const &, BufferParams const &,
			     LyXFont const & outerfont, std::ostream &,
			     TexRow & texrow, LatexRunParams const &);

	///
	bool hasSameLayout(Paragraph const & par) const;

	///
	void makeSameLayout(Paragraph const & par);

	///
	UpdatableInset * inInset() const;
	///
	void setInsetOwner(UpdatableInset * inset);
	///
	void deleteInsetsLyXText(BufferView *);

	///
	lyx::pos_type size() const;
	///
	bool empty() const;
	///
	void setContentsFromPar(Paragraph const & par);
	///
	void clearContents();

	///
	LyXLayout_ptr const & layout() const;
	///
	void layout(LyXLayout_ptr const & new_layout);

	///
	char enumdepth;

	///
	char itemdepth;

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
	int beginningOfBody() const;

	///
	string const & getLabelstring() const;

	/// the next two functions are for the manual labels
	string const getLabelWidthString() const;
	///
	void setLabelWidthString(string const & s);
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
	 * span of characters. This method will return the last position
	 * in the paragraph for which that font is the same.
	 * This can be used to avoid unnecessary calls to getFont.
	 */
	lyx::pos_type getEndPosOfFontSpan(lyx::pos_type pos) const;
	///
	value_type getChar(lyx::pos_type pos) const;
	///
	value_type getUChar(BufferParams const &, lyx::pos_type pos) const;
	/// The position must already exist.
	void setChar(lyx::pos_type pos, value_type c);
	/// pos <= size() (there is a dummy font change at the end of each par)
	void setFont(lyx::pos_type pos, LyXFont const & font);
	/// Returns the height of the highest font in range
	LyXFont::FONT_SIZE
	highestFontInRange(lyx::pos_type startpos,
			   lyx::pos_type endpos,
			   LyXFont::FONT_SIZE const def_size) const;
	///
	void insertChar(lyx::pos_type pos, value_type c);
	///
	void insertChar(lyx::pos_type pos, value_type c, LyXFont const &, Change change = Change(Change::INSERTED));
	///
	bool checkInsertChar(LyXFont &);
	///
	void insertInset(lyx::pos_type pos, InsetOld * inset);
	///
	void insertInset(lyx::pos_type pos, InsetOld * inset, LyXFont const &, Change change = Change(Change::INSERTED));
	///
	bool insetAllowed(InsetOld::Code code);
	///
	InsetOld * getInset(lyx::pos_type pos);
	///
	InsetOld const * getInset(lyx::pos_type pos) const;

	///
	bool isHfill(lyx::pos_type pos) const;
	///
	bool isInset(lyx::pos_type pos) const;
	///
	bool isNewline(lyx::pos_type pos) const;
	///
	bool isSeparator(lyx::pos_type pos) const;
	///
	bool isLineSeparator(lyx::pos_type pos) const;
	///
	bool isKomma(lyx::pos_type pos) const;
	/// Used by the spellchecker
	bool isLetter(lyx::pos_type pos) const;
	///
	bool isWord(lyx::pos_type pos) const;

	/// returns -1 if inset not found
	int getPositionOfInset(InsetOld const * inset) const;

	///
	int stripLeadingSpaces();

	/// return true if we allow multiple spaces
	bool isFreeSpacing() const;

	/// return true if we allow this par to stay empty
	bool allowEmpty() const;

	///
	ParagraphParameters & params();
	///
	ParagraphParameters const & params() const;
	///
	InsetList insetlist;

	///
	mutable RowList rows;
	/// last draw y position (baseline of top row)
	int y;

private:
	///
	LyXLayout_ptr layout_;

	struct Pimpl;
	///
	friend struct Paragraph::Pimpl;
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
