// -*- C++ -*-
/**
 * \file lyxtext.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author Lars Gullik Bjønnes
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYXTEXT_H
#define LYXTEXT_H

#include "bufferview_funcs.h"
#include "layout.h"
#include "lyxfont.h"
#include "ParagraphList_fwd.h"
#include "RowList_fwd.h"
#include "textcursor.h"

#include "insets/inset.h"

class Buffer;
class BufferParams;
class BufferView;
class Dimension;
class LColor_color;
class InsetText;
class LyXCursor;
class MetricsInfo;
class Paragraph;
class Row;
class Spacing;
class UpdatableInset;
class VSpace;
class WordLangTuple;


/**
  This class used to hold the mapping between buffer paragraphs and
	screen rows. Nowadays, the Paragraphs take care of their rows
  themselves and this contains just most of the code for manipulating
  them and interaction with the Cursor.
  */

// The inheritance from TextCursor should go. It's just there to ease
// transition...
class LyXText : public TextCursor {
public:
	/// Constructor
	LyXText(BufferView *, InsetText *, bool ininset,
		ParagraphList & paragraphs);

	void init(BufferView *);
	///
	int height;
	///
	unsigned int width;
	/// the current font settings
	LyXFont current_font;
	/// the current font
	LyXFont real_current_font;
	/// our buffer's default layout font
	LyXFont defaultfont_;
private:
	/// offset of dran area to document start.
	int anchor_y_;
public:
	/// update all cached row positions
	void updateRowPositions();
	///
	InsetText * inset_owner;
	///
	UpdatableInset * the_locking_inset;

	///
	int getRealCursorX() const;
	///
	LyXFont getFont(ParagraphList::iterator pit, lyx::pos_type pos) const;
	///
	LyXFont getLayoutFont(ParagraphList::iterator pit) const;
	///
	LyXFont getLabelFont(ParagraphList::iterator pit) const;
	///
	void setCharFont(ParagraphList::iterator pit,
			 lyx::pos_type pos, LyXFont const & font);
	void setCharFont(ParagraphList::iterator pit,
			 lyx::pos_type pos,
			 LyXFont const & font, bool toggleall);

	/// what you expect when pressing <enter> at cursor position
	void breakParagraph(ParagraphList & paragraphs, char keep_layout = 0);

	/** set layout over selection and make a total rebreak of
	  those paragraphs
	  */
	ParagraphList::iterator
	setLayout(LyXCursor & actual_cursor,
		  LyXCursor & selection_start,
		  LyXCursor & selection_end,
		  std::string const & layout);
	///
	void setLayout(std::string const & layout);

	/**
	 * Increase or decrease the nesting depth of the selected paragraph(s)
	 * if test_only, don't change any depths. Returns whether something
	 * (would have) changed
	 */
	bool changeDepth(bv_funcs::DEPTH_CHANGE type, bool test_only);

	/// get the depth at current cursor position
	int getDepth() const;

	/** set font over selection and make a total rebreak of those
	  paragraphs.
	  toggleall defaults to false.
	  */
	void setFont(LyXFont const &, bool toggleall = false);

	/// rebreaks all paragaphs between the given pars.
	int redoParagraphs(ParagraphList::iterator begin,
			    ParagraphList::iterator end);
	/// rebreaks the given par
	void redoParagraph(ParagraphList::iterator pit);

	/// rebreaks the cursor par
	void redoParagraph();
private:
	/// rebreaks the given par, return max row width
	int redoParagraphInternal(ParagraphList::iterator pit);
public:

	///
	void toggleFree(LyXFont const &, bool toggleall = false);

	///
	std::string getStringToIndex();

	/** insert a character, moves all the following breaks in the
	  same Paragraph one to the right and make a little rebreak
	  */
	void insertChar(char c);
	///
	void insertInset(InsetOld * inset);

	/// a full rebreak of the whole text
	void fullRebreak();
	/// compute text metrics
	void metrics(MetricsInfo & mi, Dimension & dim);

	///
	InsetOld::RESULT dispatch(FuncRequest const & cmd);

	BufferView * bv();

	BufferView * bv() const;

	friend class LyXScreen;

public:
	/// only the top-level LyXText has this non-zero
	BufferView * bv_owner;

private:
	/// returns a pointer to a specified row.
	RowList::iterator
	getRow(ParagraphList::iterator pit, lyx::pos_type pos) const;
public:
	/// returns an iterator pointing to a cursor row
	RowList::iterator getRow(LyXCursor const & cursor) const;
	/// convenience
	RowList::iterator cursorRow() const;
	/// returns an iterator pointing to a cursor paragraph
	ParagraphList::iterator getPar(LyXCursor const & cursor) const;
	///
	ParagraphList::iterator getPar(lyx::paroffset_type par) const;
	///
	int parOffset(ParagraphList::iterator pit) const;
	/// convenience
	ParagraphList::iterator cursorPar() const;
	/**
	 * Return the next row, when cursor is at the end of the
	 * previous row, for insets that take a full row.
	 *
	 * FIXME: explain why we need this ? especially for y...
	 */
	RowList::iterator cursorIRow() const;

	/** returns a pointer to the row near the specified y-coordinate
	  (relative to the whole text). y is set to the real beginning
	  of this row
	  */
	RowList::iterator getRowNearY(int y,
		ParagraphList::iterator & pit) const;

	/** returns the column near the specified x-coordinate of the row
	 x is set to the real beginning of this column
	 */
	lyx::pos_type getColumnNearX(ParagraphList::iterator pit,
		RowList::iterator rit, int & x, bool & boundary) const;

	/// need the selection cursor:
	void setSelection();
	///
	void clearSelection();

	/// select the word we need depending on word_location
	void getWord(LyXCursor & from, LyXCursor & to, lyx::word_location const);
	/// just selects the word the cursor is in
	void selectWord(lyx::word_location loc);
	/// returns the inset at cursor (if it exists), 0 otherwise
	InsetOld * getInset() const;

	/// accept selected change
	void acceptChange();

	/// reject selected change
	void rejectChange();

	/** 'selects" the next word, where the cursor is not in
	 and returns this word as string. THe cursor will be moved
	 to the beginning of this word.
	 With SelectSelectedWord can this be highlighted really
	 */
	WordLangTuple const selectNextWordToSpellcheck(float & value);
	///
	void selectSelectedWord();
	/// re-computes the cached coordinates in the cursor
	void redoCursor();
	///
	void setCursor(ParagraphList::iterator pit, lyx::pos_type pos);
	/// returns true if par was empty and was removed
	bool setCursor(lyx::paroffset_type par,
		       lyx::pos_type pos,
		       bool setfont = true,
		       bool boundary = false);
	///
	void setCursor(LyXCursor &, lyx::paroffset_type par,
		       lyx::pos_type pos,
		       bool boundary = false);
	///
	void setCursorIntern(lyx::paroffset_type par,
			     lyx::pos_type pos,
			     bool setfont = true,
			     bool boundary = false);
	///
	void setCurrentFont();

	///
	bool isBoundary(Buffer const &, Paragraph const & par,
			lyx::pos_type pos) const;
	///
	bool isBoundary(Buffer const &, Paragraph const & par,
			 lyx::pos_type pos,
			 LyXFont const & font) const;

	///
	void setCursorFromCoordinates(int x, int y);
	///
	void setCursorFromCoordinates(LyXCursor &,
				      int x, int y);
	///
	void cursorUp(bool selecting = false);
	///
	void cursorDown(bool selecting = false);
	///
	void cursorLeft(bool internal = true);
	///
	void cursorRight(bool internal = true);
	///
	void cursorLeftOneWord();
	///
	void cursorRightOneWord();
	///
	void cursorUpParagraph();
	///
	void cursorDownParagraph();
	///
	void cursorHome();
	///
	void cursorEnd();
	///
	void cursorPrevious();
	///
	void cursorNext();
	///
	void cursorTop();
	///
	void cursorBottom();
	///
	void Delete();
	///
	void backspace();
	///
	bool selectWordWhenUnderCursor(lyx::word_location);
	///
	enum TextCase {
		///
		text_lowercase = 0,
		///
		text_capitalization = 1,
		///
		text_uppercase = 2
	};
	/// Change the case of the word at cursor position.
	void changeCase(TextCase action);

	///
	void toggleInset();
	///
	void cutSelection(bool doclear = true, bool realcut = true);
	///
	void copySelection();
	///
	void pasteSelection(size_t sel_index = 0);

	/** the DTP switches for paragraphs. LyX will store the top settings
	 always in the first physical paragraph, the bottom settings in the
	 last. When a paragraph is broken, the top settings rest, the bottom
	 settings are given to the new one. So I can make shure, they do not
	 duplicate themself (and you cannnot make dirty things with them! )
	 */
	void setParagraph(bool line_top, bool line_bottom,
			  bool pagebreak_top, bool pagebreak_bottom,
			  VSpace const & space_top,
			  VSpace const & space_bottom,
			  Spacing const & spacing,
			  LyXAlignment align,
			  std::string const & labelwidthstring,
			  bool noindent);

	/* these things are for search and replace */

	/**
	 * Sets the selection from the current cursor position to length
	 * characters to the right. No safety checks.
	 */
	void setSelectionRange(lyx::pos_type length);

	/** simple replacing. The font of the first selected character
	  is used
	  */
	void replaceSelectionWithString(std::string const & str);

	/// needed to insert the selection
	void insertStringAsLines(std::string const & str);
	/// needed to insert the selection
	void insertStringAsParagraphs(std::string const & str);

	/// Find next inset of some specified type.
	bool gotoNextInset(std::vector<InsetOld::Code> const & codes,
			   std::string const & contents = std::string());
	///
	void gotoInset(std::vector<InsetOld::Code> const & codes,
		       bool same_content);
	///
	void gotoInset(InsetOld::Code code, bool same_content);

	///
	int workWidth() const;

	///
	void computeBidiTables(ParagraphList::iterator pit,
		Buffer const &, RowList::iterator row) const;
	/// Maps positions in the visual string to positions in logical string.
	lyx::pos_type log2vis(lyx::pos_type pos) const;
	/// Maps positions in the logical string to positions in visual string.
	lyx::pos_type vis2log(lyx::pos_type pos) const;
	///
	lyx::pos_type bidi_level(lyx::pos_type pos) const;
	///
	bool bidi_InRange(lyx::pos_type pos) const;
private:
	///
	float getCursorX(ParagraphList::iterator pit,
	     RowList::iterator rit, lyx::pos_type pos,
			 lyx::pos_type last, bool boundary) const;
	/// used in setlayout
	void makeFontEntriesLayoutSpecific(BufferParams const &, Paragraph & par);

	/// Calculate and set the height of the row
	void setHeightOfRow(ParagraphList::iterator, RowList::iterator rit);

	// fix the cursor `cur' after a characters has been deleted at `where'
	// position. Called by deleteEmptyParagraphMechanism
	void fixCursorAfterDelete(LyXCursor & cur, LyXCursor const & where);

	/// delete double space (false) or empty paragraphs (true) around old_cursor
	bool deleteEmptyParagraphMechanism(LyXCursor const & old_cursor);

public:
	/** Updates all counters starting BEHIND the row. Changed paragraphs
	 * with a dynamic left margin will be rebroken. */
	void updateCounters();
	/**
	 * Returns an inset if inset was hit, or 0 if not.
	 * If hit, the coordinates are changed relative to the inset.
	 */
	InsetOld * checkInsetHit(int & x, int & y);

	///
	int singleWidth(ParagraphList::iterator pit, lyx::pos_type pos) const;
	///
	int singleWidth(ParagraphList::iterator pit,
		lyx::pos_type pos, char c, LyXFont const & Font) const;

	/// return the color of the canvas
	LColor_color backgroundColor() const;

	///
	mutable bool bidi_same_direction;

	unsigned char transformChar(unsigned char c, Paragraph const & par,
				    lyx::pos_type pos) const;

	/**
	 * Returns the left beginning of the text.
	 * This information cannot be taken from the layout object, because
	 * in LaTeX the beginning of the text fits in some cases
	 * (for example sections) exactly the label-width.
	 */
	int leftMargin(ParagraphList::iterator pit, Row const & row) const;
	///
	int rightMargin(ParagraphList::iterator pit, Buffer const &, Row const & row) const;

	/** this calculates the specified parameters. needed when setting
	 * the cursor and when creating a visible row */
	void prepareToPrint(ParagraphList::iterator pit,
	        RowList::iterator row) const;

private:
	///
	void setCounter(Buffer const &, ParagraphList::iterator pit);
	///
	void deleteWordForward();
	///
	void deleteWordBackward();
	///
	void deleteLineForward();

	/*
	 * some low level functions
	 */


	/// sets row.end to the pos value *after* which a row should break.
	/// for example, the pos after which isNewLine(pos) == true
	lyx::pos_type rowBreakPoint(ParagraphList::iterator pit,
		Row const & row) const;

	/// returns the minimum space a row needs on the screen in pixel
	int fill(ParagraphList::iterator pit,
		RowList::iterator row, int workwidth) const;

	/**
	 * returns the minimum space a manual label needs on the
	 * screen in pixels
	 */
	int labelFill(ParagraphList::iterator pit, Row const & row) const;

	/// FIXME
	int labelEnd(ParagraphList::iterator pit, Row const & row) const;

	///
	mutable std::vector<lyx::pos_type> log2vis_list;
	///
	mutable std::vector<lyx::pos_type> vis2log_list;
	///
	mutable std::vector<lyx::pos_type> bidi_levels;
	///
	mutable lyx::pos_type bidi_start;
	///
	mutable lyx::pos_type bidi_end;

	///
	void charInserted();
public:
	///
	bool in_inset_;
	///
	ParagraphList * paragraphs_;
	//
	// special owner functions
	///
	ParagraphList & ownerParagraphs() const;

	/// return true if this is owned by an inset.
	bool isInInset() const;

	/// return first row of text
	RowList::iterator firstRow() const;
	/// return last row of text
	RowList::iterator lastRow() const;
	/// return row "behind" last row of text
	RowList::iterator endRow() const;
	/// return next row crossing paragraph boundaries
	void nextRow(ParagraphList::iterator & pit,
		RowList::iterator & rit) const;
	/// return previous row crossing paragraph boundaries
	void previousRow(ParagraphList::iterator & pit,
		RowList::iterator & rit) const;

	///
	std::string selectionAsString(Buffer const & buffer, bool label) const;
private:
	/** Cursor related data.
	  Later this variable has to be removed. There should be now internal
	  cursor in a text */
	///
	///TextCursor cursor_;
	/// prohibit this as long as there are back pointers...
	LyXText(LyXText const &);
};

/// return the default height of a row in pixels, considering font zoom
extern int defaultRowHeight();

#endif // LYXTEXT_H
