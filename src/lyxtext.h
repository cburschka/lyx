// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#ifndef LYXTEXT_H
#define LYXTEXT_H

#ifdef __GNUG__
#pragma interface
#endif

#include "lyxfont.h"
#include "lyxcursor.h"
#include "paragraph.h"
#include "layout.h"
#include "lyxrow.h"
#include "vspace.h"

class Buffer;
class BufferParams;
class BufferView;
class InsetText;


/**
  This class holds the mapping between buffer paragraphs and screen rows.
  */
class LyXText {
public:
	///
	enum text_status {
		///
		UNCHANGED = 0,
		///
		CHANGED_IN_DRAW = 1,
		///
		NEED_VERY_LITTLE_REFRESH = 2,
		///
		NEED_MORE_REFRESH = 3
	};
	///
	enum word_location {
		/// the word around the cursor
		WHOLE_WORD,
		/// the word begining from the cursor position
		PARTIAL_WORD,
		/// the next word (not yet used)
		NEXT_WORD
	};

	/// Constructor
	LyXText(BufferView *);
	///
	LyXText(InsetText *);
   
	/// Destructor
	~LyXText();

	void init(BufferView *);
	///
	mutable int number_of_rows;
	///
	mutable int height;
	///
	mutable unsigned int width;
	/// the current font settings
	mutable LyXFont current_font;
	/// the current font
	mutable LyXFont real_current_font;
	/// first visible pixel-row is set from LyXScreen!!!
	// unsigned is wrong here for text-insets!
	int first;
	///
	BufferView * bv_owner;
	///
	InsetText * inset_owner;
	///
	UpdatableInset * the_locking_inset;

	///
	int getRealCursorX(BufferView *) const;
	///
	LyXFont const getFont(Buffer const *, Paragraph * par,
			Paragraph::size_type pos) const;
	///
	void setCharFont(Buffer const *, Paragraph * par,
	                 Paragraph::size_type pos, LyXFont const & font);
	void setCharFont(BufferView *, Paragraph * par,
	                 Paragraph::size_type pos,
	                 LyXFont const & font, bool toggleall);
	/// returns a pointer to the very first Paragraph
	Paragraph * firstParagraph() const;
  
	/// what you expect when pressing <enter> at cursor position
	void breakParagraph(BufferView *, char keep_layout = 0);

	/** set layout over selection and make a total rebreak of
	  those paragraphs
	  */
	Paragraph * setLayout(BufferView *, LyXCursor & actual_cursor,
				 LyXCursor & selection_start,
				 LyXCursor & selection_end,
				 LyXTextClass::size_type layout);
	void setLayout(BufferView *, LyXTextClass::size_type layout);
	
	/// used in setlayout
	void makeFontEntriesLayoutSpecific(Buffer const *, Paragraph * par);
	
	/** increment depth over selection and make a total rebreak of those 
	  paragraphs
	  */
	void incDepth(BufferView *);
	
	/** decrement depth over selection and make a total rebreak of those  
	  paragraphs */
	void decDepth(BufferView *);

	/** Get the depth at current cursor position
	 */
	int getDepth() const { return cursor.par()->getDepth(); }
	
	/** set font over selection and make a total rebreak of those
	  paragraphs.
	  toggleall defaults to false.
	  */
	void setFont(BufferView *, LyXFont const &, bool toggleall = false);
	
	/** deletes and inserts again all paragaphs between the cursor
	  and the specified par. The Cursor is needed to set the refreshing
	  parameters. 
	  This function is needed after SetLayout and SetFont etc.
	  */
	void redoParagraphs(BufferView *, LyXCursor const & cursor,
			    Paragraph const * end_par) const;
	///
	void redoParagraph(BufferView *) const;
	
	///
	void toggleFree(BufferView *, LyXFont const &, bool toggleall = false);
	
	/** recalculates the heights of all previous rows of the
	    specified paragraph.  needed, if the last characters font
	    has changed.  
	    */
	void redoHeightOfParagraph(BufferView *, LyXCursor const & cursor);
	
	/** forces the redrawing of a paragraph. Needed when manipulating a 
	    right address box
	    */ 
	void redoDrawingOfParagraph(BufferView *, LyXCursor const & cursor);
	
	/** insert a character, moves all the following breaks in the 
	  same Paragraph one to the right and make a little rebreak
	  */
	void insertChar(BufferView *, char c);
	///
	void insertInset(BufferView *, Inset * inset);
   
	/** Completes the insertion with a full rebreak.
	    Returns true if something was broken. */
        bool fullRebreak(BufferView *);

	///
	Row * need_break_row;
	///
	mutable int refresh_y;
	///
	int refresh_height;
	///
	int refresh_width;
	///
	int refresh_x;
	///
	mutable Row * refresh_row;
	///
	int refresh_pos;

	/// give and set the LyXText status
	text_status status() const;
	void status(BufferView *, text_status) const;

private:	
	/** wether the screen needs a refresh,
	   starting with refresh_y
	   */
	mutable text_status status_;
	
public:
	/** returns a pointer to the row near the specified y-coordinate
	  (relative to the whole text). y is set to the real beginning
	  of this row
	  */ 
	Row * getRowNearY(int & y) const;
	
	/** returns the column near the specified x-coordinate of the row 
	 x is set to the real beginning of this column
	 */ 
	int getColumnNearX(BufferView *, Row * row,
			   int & x, bool & boundary) const;
	
	/** returns a pointer to a specified row. y is set to the beginning
	 of the row
	 */
	Row * getRow(Paragraph * par,
		     Paragraph::size_type pos, int & y) const;

	/** returns the height of a default row, needed  for scrollbar
	 */
	int defaultHeight() const;
   
	/** The cursor.
	  Later this variable has to be removed. There should be now internal
	  cursor in a text (and thus not in a buffer). By keeping this it is
	  (I think) impossible to have several views with the same buffer, but
	  the cursor placed at different places.
	  [later]
	  Since the LyXText now has been moved from Buffer to BufferView
	  it should not be absolutely needed to move the cursor...
	  */
	mutable LyXCursor cursor;

	/** The structrue that keeps track of the selections set. */
	struct Selection {
		Selection() 
			: set_(false), mark_(false)
			{}
		bool set() const {
			return set_;
		}
		void set(bool s) {
			set_ = s;
		}
		bool mark() const {
			return mark_;
		}
		void mark(bool m) {
			mark_ = m;
		}
		LyXCursor cursor;
		LyXCursor start;
		LyXCursor end;
	private:
		bool set_; // former selection
		bool mark_; // former mark_set
		
	};
	mutable Selection selection;

	/// needed for the toggling
	LyXCursor last_sel_cursor;
	///
	LyXCursor toggle_cursor;
	///
	LyXCursor toggle_end_cursor;
   
	/// need the selection cursor:
	void setSelection(BufferView *);
	///
	void clearSelection(BufferView *) const;
	///
	string const selectionAsString(Buffer const *) const;
	
	/// select the word we need depending on word_location
	void getWord(LyXCursor & from, LyXCursor & to, word_location) const;
	/// just selects the word the cursor is in
	void selectWord(BufferView *);

	/** 'selects" the next word, where the cursor is not in 
	 and returns this word as string. THe cursor will be moved 
	 to the beginning of this word. 
	 With SelectSelectedWord can this be highlighted really
	 */ 
	string const selectNextWord(BufferView *, float & value) const;
	///
	void selectSelectedWord(BufferView *);
	///
	void setCursor(BufferView *, Paragraph * par,
		       Paragraph::size_type pos,
		       bool setfont = true,
		       bool boundary = false) const;
	///
	void setCursor(BufferView *, LyXCursor &, Paragraph * par,
		       Paragraph::size_type pos,
		       bool boundary = false) const;
	///
	void setCursorIntern(BufferView *, Paragraph * par,
			     Paragraph::size_type pos,
			     bool setfont = true,
			     bool boundary = false) const;
	///
	void setCurrentFont(BufferView *) const;

	///
	bool isBoundary(Buffer const *, Paragraph * par,
			Paragraph::size_type pos) const;
	///
	bool isBoundary(Buffer const *, Paragraph * par,
			 Paragraph::size_type pos,
			 LyXFont const & font) const;

	///
	void setCursorFromCoordinates(BufferView *, int x, int y) const;
	///
	void setCursorFromCoordinates(BufferView *, LyXCursor &,
				      int x, int y) const;
	///
	void cursorUp(BufferView *) const;
	///
	void cursorDown(BufferView *) const;
	///
	void cursorLeft(BufferView *, bool internal = true) const;
	///
	void cursorRight(BufferView *, bool internal = true) const;
	///
	void cursorLeftOneWord(BufferView *) const;
	///
	void cursorLeftOneWord(LyXCursor &) const;
	///
	void cursorRightOneWord(BufferView *) const;
	///
	void cursorUpParagraph(BufferView *) const;
	///
	void cursorDownParagraph(BufferView *) const;
	///
	void cursorHome(BufferView *) const;
	///
	void cursorEnd(BufferView *) const;
	///
	void cursorTab(BufferView *) const;
	///
	void cursorTop(BufferView *) const;
	///
	void cursorBottom(BufferView *) const;
	///
	void Delete(BufferView *);
	///
	void backspace(BufferView *);
	///
	void deleteWordForward(BufferView *);
	///
	void deleteWordBackward(BufferView *);
	///
	void deleteLineForward(BufferView *);
	///
	bool selectWordWhenUnderCursor(BufferView *);
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
	void changeCase(BufferView *, TextCase action);
	///
	void changeRegionCase(BufferView * bview,
				       LyXCursor const & from,
				       LyXCursor const & to,
				       LyXText::TextCase action);
	///
	void transposeChars(BufferView &);
	
	/** returns a printed row in a pixmap. The y value is needed to
	  decide, wether it is selected text or not. This is a strange
	  solution but faster.
	 */
	void getVisibleRow(BufferView *, int y_offset, int x_offset,
			   Row * row_ptr, int y, bool cleared=false);

	/// 
	void openStuff(BufferView *);
	///
	void cutSelection(BufferView *, bool = true);
	///
	void copySelection(BufferView *);
	///
	void pasteSelection(BufferView *);
	///
	void copyEnvironmentType();
	///
	void pasteEnvironmentType(BufferView *);
	///
	void insertFootnote();
	///
	void insertMarginpar();
	///
	void insertFigure();
	///
	void insertTabular();

	/** the DTP switches for paragraphs. LyX will store the top settings
	 always in the first physical paragraph, the bottom settings in the
	 last. When a paragraph is broken, the top settings rest, the bottom 
	 settings are given to the new one. So I can make shure, they do not
	 duplicate themself (and you cannnot make dirty things with them! )
	 */ 
	void setParagraph(BufferView *,
			  bool line_top, bool line_bottom,
			  bool pagebreak_top, bool pagebreak_bottom,
			  VSpace const & space_top,
			  VSpace const & space_bottom,
			  LyXAlignment align, 
			  string labelwidthstring,
			  bool noindent);

	/* these things are for search and replace */

	/** sets the selection over the number of characters of string,
	  no check!!
	  */
	void setSelectionOverString(BufferView *, string const & str);

	/** simple replacing. The font of the first selected character
	  is used
	  */
	void replaceSelectionWithString(BufferView *, string const & str);

	/// needed to insert the selection
	void insertStringAsLines(BufferView *, string const & str);
	/// needed to insert the selection
	void insertStringAsParagraphs(BufferView *, string const & str);

	/// Find next inset of some specified type.
	bool gotoNextInset(BufferView *, std::vector<Inset::Code> const & codes,
			   string const & contents = string()) const;
	///

	/* for the greater insets */
  
	/// returns false if inset wasn't found
	bool updateInset(BufferView *, Inset *);
	///
	void checkParagraph(BufferView *, Paragraph * par,
			    Paragraph::size_type pos);
	///
	int numberOfCell(Paragraph * par,
			 Paragraph::size_type pos) const;
	///
	void removeTableRow(LyXCursor & cursor) const;
	///
	bool isEmptyTableCell() const;
	///
	void toggleAppendix(BufferView *);
	///
	int workWidth(BufferView *) const;
	///
	void computeBidiTables(Buffer const *, Row * row) const;

	/// Maps positions in the visual string to positions in logical string.
	inline
	Paragraph::size_type log2vis(Paragraph::size_type pos) const {
		if (bidi_start == -1)
			return pos;
		else
			return log2vis_list[pos-bidi_start];
	}

	/// Maps positions in the logical string to positions in visual string.
	inline
	Paragraph::size_type vis2log(Paragraph::size_type pos) const {
		if (bidi_start == -1)
			return pos;
		else
			return vis2log_list[pos-bidi_start];
	}
	///
	inline
	Paragraph::size_type bidi_level(Paragraph::size_type pos) const {
		if (bidi_start == -1)
			return 0;
		else
			return bidi_levels[pos-bidi_start];
	}	
	///
	inline
	bool bidi_InRange(Paragraph::size_type pos) const {
		return bidi_start == -1 ||
			(bidi_start <= pos && pos <= bidi_end);
	}
private:
	///
	mutable Row * firstrow;
	///
	mutable Row * lastrow;

	/** Copybuffer for copy environment type.
	  Asger has learned that this should be a buffer-property instead
	  Lgb has learned that 'char' is a lousy type for non-characters
	  */
	LyXTextClass::size_type copylayouttype;

	/** inserts a new row behind the specified row, increments
	    the touched counters */
	void insertRow(Row * row, Paragraph * par,
		       Paragraph::size_type pos) const;
	/** removes the row and reset the touched counters */
	void removeRow(Row * row) const;

	/** remove all following rows of the paragraph of the specified row. */
	void removeParagraph(Row * row) const;

	/** insert the specified paragraph behind the specified row */
	void insertParagraph(BufferView *,
			     Paragraph * par, Row * row) const;

	/** appends  the implizit specified paragraph behind the specified row,
	 * start at the implizit given position */
	void appendParagraph(BufferView *, Row * row) const;
   
	///
	void breakAgain(BufferView *, Row * row) const;
	///
	void breakAgainOneRow(BufferView *, Row * row);
	/// Calculate and set the height of the row
	void setHeightOfRow(BufferView *, Row * row_ptr) const;

	/** this calculates the specified parameters. needed when setting
	 * the cursor and when creating a visible row */ 
	void prepareToPrint(BufferView *, Row * row, float & x,
			    float & fill_separator, 
			    float & fill_hfill,
			    float & fill_label_hfill,
			    bool bidi = true) const;

	///
	void deleteEmptyParagraphMechanism(BufferView *,
					   LyXCursor const & old_cursor) const;

public:
	/** Updates all counters starting BEHIND the row. Changed paragraphs
	 * with a dynamic left margin will be rebroken. */ 
	void updateCounters(BufferView *, Row * row) const;
private:
	///
	void setCounter(Buffer const *, Paragraph * par) const;
   
	/*
	 * some low level functions
	 */
	
	///
	int singleWidth(BufferView *, Paragraph * par,
			Paragraph::size_type pos) const;
	///
	int singleWidth(BufferView *, Paragraph * par,
			Paragraph::size_type pos, char c) const;
	///
	void draw(BufferView *, Row const * row,
		  Paragraph::size_type & pos,
		  int offset, float & x, bool cleared);

	/// get the next breakpoint in a given paragraph
	Paragraph::size_type nextBreakPoint(BufferView *, Row const * row,
					       int width) const;
	/// returns the minimum space a row needs on the screen in pixel
	int fill(BufferView *, Row * row, int workwidth) const;
	
	/** returns the minimum space a manual label needs on the
	  screen in pixel */ 
	int labelFill(BufferView *, Row const * row) const;

	///
	Paragraph::size_type
	beginningOfMainBody(Buffer const *, Paragraph const * par) const;
	
	/** Returns the left beginning of the text.
	  This information cannot be taken from the layouts-objekt, because
	  in LaTeX the beginning of the text fits in some cases
	  (for example sections) exactly the label-width.
	  */
	int leftMargin(BufferView *, Row const * row) const;
	///
	int rightMargin(Buffer const *, Row const * row) const;
	///
	int labelEnd (BufferView *, Row const * row) const;

	/** returns the number of separators in the specified row.
	  The separator on the very last column doesnt count
	  */ 
	int numberOfSeparators(Buffer const *, Row const * row) const;

	/** returns the number of hfills in the specified row. The
	  LyX-Hfill is a LaTeX \hfill so that the hfills at the
	  beginning and at the end were ignored. This is {\em MUCH}
	  more usefull than not to ignore!
	  */
	int numberOfHfills(Buffer const *, Row const * row) const;
   
	/// like NumberOfHfills, but only those in the manual label!
	int numberOfLabelHfills(Buffer const *, Row const * row) const;
	/** returns true, if a expansion is needed. Rules are given by 
	  LaTeX
	  */
	bool hfillExpansion(Buffer const *, Row const * row_ptr,
			    Paragraph::size_type pos) const;


	///
	mutable std::vector<Paragraph::size_type> log2vis_list;

	///
	mutable std::vector<Paragraph::size_type> vis2log_list;

	///
	mutable std::vector<Paragraph::size_type> bidi_levels;

	///
	mutable Paragraph::size_type bidi_start;

	///
	mutable Paragraph::size_type bidi_end;

	///
	mutable bool bidi_same_direction;

	///
	unsigned char transformChar(unsigned char c, Paragraph * par,
				    Paragraph::size_type pos) const;

	/** returns the paragraph position of the last character in the 
	  specified row
	  */
	Paragraph::size_type rowLast(Row const * row) const;
	///
	Paragraph::size_type rowLastPrintable(Row const * row) const;

	///
	void charInserted();
public:
	//
	// special owner functions
	///
	Paragraph * ownerParagraph() const;
	//
	Paragraph * ownerParagraph(Paragraph *) const;
	// set it searching first for the right owner using the paragraph id
	Paragraph * ownerParagraph(int id, Paragraph *) const;
};


/* returns a pointer to the row near the specified y-coordinate
 * (relative to the whole text). y is set to the real beginning
 * of this row */
inline
Row * LyXText::getRowNearY(int & y) const
{
	// If possible we should optimize this method. (Lgb)
	Row * tmprow = firstrow;
	int tmpy = 0;
	
	while (tmprow->next() && tmpy + tmprow->height() <= y) {
		tmpy += tmprow->height();
		tmprow = tmprow->next();
	}
	
	y = tmpy;   // return the real y
	return tmprow;
}
#endif
