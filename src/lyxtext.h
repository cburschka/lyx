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

#include "lyxfont.h"
#include "lyxcursor.h"
#include "layout.h"
#include "LColor.h"
#include "insets/inset.h"

class Buffer;
class BufferParams;
class BufferView;
class InsetText;
class Paragraph;
class Row;
class Spacing;
class UpdatableInset;
class VSpace;
class WordLangTuple;
class ParagraphList;


/**
  This class holds the mapping between buffer paragraphs and screen rows.
  */
class LyXText {
public:
	/// what repainting is needed
	enum refresh_status {
		/// no repaint is needed
		REFRESH_NONE = 0,
		/// the refresh_row needs repainting
		REFRESH_ROW = 1,
		/// everything from refresh_y downwards needs repainting
		REFRESH_AREA = 2
	};

	///
	enum word_location {
		// the word around the cursor, only if the cursor is
		//not at a boundary
		WHOLE_WORD_STRICT,
		// the word around the cursor
		WHOLE_WORD,
		/// the word begining from the cursor position
		PARTIAL_WORD,
		/// the word around the cursor or before the cursor
		PREVIOUS_WORD,
		/// the next word (not yet used)
		NEXT_WORD
	};

	/// Constructor
	LyXText(BufferView *);
	/// sets inset as owner
	LyXText(BufferView *, InsetText *);

	/// Destructor
	~LyXText();

	void init(BufferView *, bool reinit = false);
	///
	int height;
	///
	unsigned int width;
	/// the current font settings
	LyXFont current_font;
	/// the current font
	LyXFont real_current_font;
private:
	/** the 'anchor' row: the position of this row remains constant
	 *  with respect to the top of the screen
	 */
	Row * anchor_row_;
	/** the pixel offset with respect to this row of top_y
	 */
	int anchor_row_offset_;
public:
	/// get the y coord. of the top of the screen (relative to doc start)
	int top_y() const;
	/// set the y coord. of the top of the screen (relative to doc start)
	void top_y(int newy);
	/// set the anchoring row. top_y will be computed relative to this
	void anchor_row(Row * row);
	///
	InsetText * inset_owner;
	///
	UpdatableInset * the_locking_inset;

	///
	int getRealCursorX() const;
	///
	LyXFont const getFont(Buffer const *, Paragraph * par,
		lyx::pos_type pos) const;
	///
	LyXFont const getLayoutFont(Buffer const *, Paragraph * par) const;
	///
	LyXFont const getLabelFont(Buffer const *, Paragraph * par) const;
	///
	void setCharFont(Buffer const *, Paragraph * par,
			 lyx::pos_type pos, LyXFont const & font);
	void setCharFont(Paragraph * par,
			 lyx::pos_type pos, LyXFont const & font, bool toggleall);

	///
	void breakAgainOneRow(Row * row);
	/// what you expect when pressing <enter> at cursor position
	void breakParagraph(ParagraphList & paragraphs, char keep_layout = 0);

	/** set layout over selection and make a total rebreak of
	  those paragraphs
	  */
	Paragraph * setLayout(LyXCursor & actual_cursor,
			      LyXCursor & selection_start,
			      LyXCursor & selection_end,
			      string const & layout);
	///
	void setLayout(string const & layout);

	/** increment depth over selection and make a total rebreak of those
	  paragraphs
	  */
	void incDepth();

	/** decrement depth over selection and make a total rebreak of those
	  paragraphs */
	void decDepth();

	/// get the depth at current cursor position
	int getDepth() const;

	/** set font over selection and make a total rebreak of those
	  paragraphs.
	  toggleall defaults to false.
	  */
	void setFont(LyXFont const &, bool toggleall = false);

	/** deletes and inserts again all paragaphs between the cursor
	  and the specified par. The Cursor is needed to set the refreshing
	  parameters.
	  This function is needed after SetLayout and SetFont etc.
	  */
	void redoParagraphs(LyXCursor const & cursor,
			    Paragraph const * end_par);
	///
	void redoParagraph();

	///
	void toggleFree(LyXFont const &, bool toggleall = false);

	///
	string getStringToIndex();

	/** recalculates the heights of all previous rows of the
	    specified paragraph.  needed, if the last characters font
	    has changed.
	    */
	void redoHeightOfParagraph();

	/** insert a character, moves all the following breaks in the
	  same Paragraph one to the right and make a little rebreak
	  */
	void insertChar(char c);
	///
	void insertInset(Inset * inset);

	/// Completes the insertion with a full rebreak
	void fullRebreak();

	///
	Row * need_break_row;

	/// clear any pending paints
	void clearPaint();

	/**
	 * Mark position y as the starting point for a repaint
	 */
	void postPaint(int start_y);

	/**
	 * Mark the given row at position y as needing a repaint.
	 */
	void postRowPaint(Row * row, int start_y);

	///
	Inset::RESULT dispatch(FuncRequest const & cmd);

	BufferView * bv();

	BufferView * bv() const;

	friend class LyXScreen;

	/**
	 * Return the status. This represents what repaints are
	 * pending after some operation (e.g. inserting a char).
	 */
	refresh_status refreshStatus() const;

private:
	/**
	 * The pixel y position from which to repaint the screen.
	 * The position is absolute along the height of outermost
	 * lyxtext (I think). REFRESH_AREA and REFRESH_ROW
	 * repaints both use this as a starting point (if it's within
	 * the viewable portion of the lyxtext).
	 */
	int refresh_y;
	/**
	 * The row from which to repaint the screen, used by screen.c.
	 * This must be set if the pending update is REFRESH_ROW.
	 * It doesn't make any difference for REFRESH_AREA.
	 */
	Row * refresh_row;

	refresh_status refresh_status_;

	/// only the top-level LyXText has this non-zero
	BufferView * bv_owner;

public:
	/** returns a pointer to the row near the specified y-coordinate
	  (relative to the whole text). y is set to the real beginning
	  of this row
	  */
	Row * getRowNearY(int & y) const;

	/** returns the column near the specified x-coordinate of the row
	 x is set to the real beginning of this column
	 */
	lyx::pos_type getColumnNearX(Row * row,
					    int & x, bool & boundary) const;

	/** returns a pointer to a specified row. y is set to the beginning
	 of the row
	 */
	Row * getRow(Paragraph * par, lyx::pos_type pos, int & y) const;
	/** returns the firstrow, this could be done with the above too but
	    IMO it's stupid to have to allocate a dummy y all the time I need
	    the first row
	*/
	Row * firstRow() const { return firstrow; }

	/** The cursor.
	  Later this variable has to be removed. There should be now internal
	  cursor in a text (and thus not in a buffer). By keeping this it is
	  (I think) impossible to have several views with the same buffer, but
	  the cursor placed at different places.
	  [later]
	  Since the LyXText now has been moved from Buffer to BufferView
	  it should not be absolutely needed to move the cursor...
	  */
	LyXCursor cursor; // actual cursor position

	/** The structure that keeps track of the selections set. */
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
		LyXCursor cursor; // temporary cursor to hold a cursor position
				  // until setSelection is called!
		LyXCursor start;  // start of a REAL selection
		LyXCursor end;    // end of a REAL selection
	private:
		bool set_; // former selection
		bool mark_; // former mark_set

	};
	Selection selection;
	// this is used to handle XSelection events in the right manner
	Selection xsel_cache;

	/// needed for the toggling (cursor position on last selection made)
	LyXCursor last_sel_cursor;
	/// needed for toggling the selection in screen.C
	LyXCursor toggle_cursor;
	/// needed for toggling the selection in screen.C
	LyXCursor toggle_end_cursor;

	/// need the selection cursor:
	void setSelection();
	///
	void clearSelection();
	///
	string const selectionAsString(Buffer const *, bool label) const;

	/// select the word we need depending on word_location
	void getWord(LyXCursor & from, LyXCursor & to,
		     word_location const);
	/// just selects the word the cursor is in
	void selectWord(word_location const);
	/// returns the inset at cursor (if it exists), 0 otherwise
	Inset * getInset() const;

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
	/// returns true if par was empty and was removed
	bool setCursor(Paragraph * par,
		       lyx::pos_type pos,
		       bool setfont = true,
		       bool boundary = false);
	///
	void setCursor(LyXCursor &, Paragraph * par,
		       lyx::pos_type pos,
		       bool boundary = false);
	///
	void setCursorIntern(Paragraph * par,
			     lyx::pos_type pos,
			     bool setfont = true,
			     bool boundary = false);
	///
	void setCurrentFont();

	///
	bool isBoundary(Buffer const *, Paragraph * par,
			lyx::pos_type pos) const;
	///
	bool isBoundary(Buffer const *, Paragraph * par,
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
	void cursorTab();
	///
	void cursorTop();
	///
	void cursorBottom();
	///
	void Delete();
	///
	void backspace();
	///
	bool selectWordWhenUnderCursor(word_location);
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
	void transposeChars();

	///
	void toggleInset();
	///
	void cutSelection(bool doclear = true, bool realcut = true);
	///
	void copySelection();
	///
	void pasteSelection();
	///
	void copyEnvironmentType();
	///
	void pasteEnvironmentType();

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
			  string const & labelwidthstring,
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
	void replaceSelectionWithString(string const & str);

	/// needed to insert the selection
	void insertStringAsLines(string const & str);
	/// needed to insert the selection
	void insertStringAsParagraphs(string const & str);

	/// Find next inset of some specified type.
	bool gotoNextInset(std::vector<Inset::Code> const & codes,
			   string const & contents = string());
	///
	void gotoInset(std::vector<Inset::Code> const & codes,
		       bool same_content);
	///
	void gotoInset(Inset::Code code, bool same_content);
	///

	/* for the greater insets */

	/// returns false if inset wasn't found
	bool updateInset(Inset *);
	///
	void checkParagraph(Paragraph * par, lyx::pos_type pos);
	///
	int workWidth() const;
	///
	int workWidth(Inset * inset) const;

	///
	void computeBidiTables(Buffer const *, Row * row) const;
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
	Row * firstrow;
	///
	Row * lastrow;

	///
	void cursorLeftOneWord(LyXCursor &);

	///
	float getCursorX(Row *, lyx::pos_type pos,
			 lyx::pos_type last, bool boundary) const;
	/// used in setlayout
	void makeFontEntriesLayoutSpecific(Buffer const &, Paragraph & par);

	/** forces the redrawing of a paragraph. Needed when manipulating a
	    right address box
	    */
	void redoDrawingOfParagraph(LyXCursor const & cursor);

	/** Copybuffer for copy environment type.
	  Asger has learned that this should be a buffer-property instead
	  Lgb has learned that 'char' is a lousy type for non-characters
	  */
	string copylayouttype;

	/** inserts a new row behind the specified row, increments
	    the touched counters */
	void insertRow(Row * row, Paragraph * par, lyx::pos_type pos);
	/// removes the row and reset the touched counters
	void removeRow(Row * row);

	/// remove all following rows of the paragraph of the specified row.
	void removeParagraph(Row * row);

	/// insert the specified paragraph behind the specified row
	void insertParagraph(Paragraph * par, Row * row);

	/** appends  the implizit specified paragraph behind the specified row,
	 * start at the implizit given position */
	void appendParagraph(Row * row);

	///
	void breakAgain(Row * row);
	/// Calculate and set the height of the row
	void setHeightOfRow(Row * row_ptr);

	// fix the cursor `cur' after a characters has been deleted at `where'
	// position. Called by deleteEmptyParagraphMechanism
	void fixCursorAfterDelete(LyXCursor & cur,
				  LyXCursor const & where);

	/// delete double space (false) or empty paragraphs (true) around old_cursor
	bool deleteEmptyParagraphMechanism(LyXCursor const & old_cursor);

public:
	/** Updates all counters starting BEHIND the row. Changed paragraphs
	 * with a dynamic left margin will be rebroken. */
	void updateCounters();
	///
	void update();
	/**
	 * Returns an inset if inset was hit, or 0 if not.
	 * If hit, the coordinates are changed relative to the inset.
	 */
	Inset * checkInsetHit(int & x, int & y);

	///
	int singleWidth(Paragraph * par,
		lyx::pos_type pos) const;
	///
	int singleWidth(Paragraph * par,
		lyx::pos_type pos, char c) const;

	/// return the color of the canvas
	LColor::color backgroundColor() const;

	///
	mutable bool bidi_same_direction;

	unsigned char transformChar(unsigned char c, Paragraph * par,
				    lyx::pos_type pos) const;

	/**
	 * Returns the left beginning of the text.
	 * This information cannot be taken from the layout object, because
	 * in LaTeX the beginning of the text fits in some cases
	 * (for example sections) exactly the label-width.
	 */
	int leftMargin(Row const * row) const;
	///
	int rightMargin(Buffer const &, Row const & row) const;

	/** this calculates the specified parameters. needed when setting
	 * the cursor and when creating a visible row */
	void prepareToPrint(Row * row, float & x,
			    float & fill_separator,
			    float & fill_hfill,
			    float & fill_label_hfill,
			    bool bidi = true) const;

private:
	///
	void setCounter(Buffer const *, Paragraph * par);
	///
	void deleteWordForward();
	///
	void deleteWordBackward();
	///
	void deleteLineForward();

	/*
	 * some low level functions
	 */


	/// return the pos value *before* which a row should break.
	/// for example, the pos at which IsNewLine(pos) == true
	lyx::pos_type rowBreakPoint(Row const & row) const;

	/// returns the minimum space a row needs on the screen in pixel
	int fill(Row & row, int workwidth) const;

	/**
	 * returns the minimum space a manual label needs on the
	 * screen in pixels
	 */
	int labelFill(Row const & row) const;

	/// FIXME
	int labelEnd(Row const & row) const;

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
	//
	// special owner functions
	///
	Paragraph * ownerParagraph() const;
	//
	void ownerParagraph(Paragraph *) const;
	// set it searching first for the right owner using the paragraph id
	void ownerParagraph(int id, Paragraph *) const;

	/// return true if this is owned by an inset.
	bool isInInset() const;
};

/// return the default height of a row in pixels, considering font zoom
extern int defaultRowHeight();

#endif // LYXTEXT_H
