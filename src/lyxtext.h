// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#ifndef LYXTEXT_H
#define LYXTEXT_H

#ifdef __GNUG__
#pragma interface
#endif

#include "lyxfont.h"
#include "undo.h"
#include "lyxcursor.h"
#include "lyxparagraph.h"
#include "layout.h"

class Buffer;
class BufferParams;
class Row;
class BufferView;


/**
  This class holds the mapping between buffer paragraphs and screen rows.
  */
class LyXText {
public:
	///
	enum text_status {
		///
		UNCHANGED,
		///
		NEED_MORE_REFRESH,
		///
		NEED_VERY_LITTLE_REFRESH
	};

	enum Letter_Form {
		FORM_ISOLATED,
		FORM_INITIAL,
		FORM_MEDIAL,
		FORM_FINAL
	};

	/// Constructor
	LyXText(BufferView *, int paperwidth, Buffer *);
   
	/// Destructor
	~LyXText();

	/// points to Buffer.params
	//BufferParams * bparams;
	/// points to Buffer
	Buffer * buffer;
	///
	mutable int number_of_rows;
	///
	mutable long height;
	/// the current font settings
	mutable LyXFont current_font;
	/// the current font
	mutable LyXFont real_current_font;

	///
	void owner(BufferView *);
	
	///
	LyXFont GetFont(LyXParagraph * par,
			LyXParagraph::size_type pos) const;
	///
	void SetCharFont(LyXParagraph * par,
			 LyXParagraph::size_type pos,
			 LyXFont const & font);
	/// returns a pointer to the very first LyXParagraph
	LyXParagraph * FirstParagraph() const;
  
	/// what you expect when pressing <enter> at cursor position
	void BreakParagraph(char keep_layout = 0);

	/** set layout over selection and make a total rebreak of
	  those paragraphs
	  */
	LyXParagraph * SetLayout(LyXCursor & actual_cursor,
				 LyXCursor & selection_start,
				 LyXCursor & selection_end,
				 LyXTextClass::size_type layout);
	void SetLayout(LyXTextClass::size_type layout);
	
	/// used in setlayout
	void MakeFontEntriesLayoutSpecific(LyXParagraph * par);
	
	/** increment depth over selection and make a total rebreak of those 
	  paragraphs
	  */
	void IncDepth();
	
	/** decrement depth over selection and make a total rebreak of those  
	  paragraphs */
	void DecDepth();

	/** Get the depth at current cursor position
	 */
	int GetDepth() const { return cursor.par->GetDepth(); }
	
	/** set font over selection and make a total rebreak of those
	  paragraphs.
	  toggleall defaults to false.
	  */
	void SetFont(LyXFont const & font, bool toggleall = false);
	
	/** deletes and inserts again all paragaphs between the cursor
	  and the specified par. The Cursor is needed to set the refreshing
	  parameters. 
	  This function is needed after SetLayout and SetFont etc.
	  */
	void RedoParagraphs(LyXCursor const & cursor,
			    LyXParagraph const * end_par) const;
	///
	void RedoParagraph() const;
	
	///
	void ToggleFree(LyXFont const & font, bool toggleall = false);
	
	/** recalculates the heights of all previous rows of the
	    specified paragraph.  needed, if the last characters font
	    has changed.  
	    */
	void RedoHeightOfParagraph(LyXCursor const & cursor);
	
	/** forces the redrawing of a paragraph. Needed when manipulating a 
	    right address box
	    */ 
	void RedoDrawingOfParagraph(LyXCursor const & cursor);
	
	/** insert a character, moves all the following breaks in the 
	  same Paragraph one to the right and make a little rebreak
	  */
	void InsertChar(char c);
	///
	void InsertInset(Inset * inset);
   
	/** Completes the insertion with a full rebreak.
	    Returns true if something was broken. */
        bool FullRebreak();

	///
	LyXParagraph::footnote_flag GetFootnoteFlag(int row);
	///
	Row * need_break_row;
	///
	mutable long refresh_y;
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
	
	/** wether the screen needs a refresh,
	   starting with refresh_y
	   */
	mutable text_status status;
	
	/** returns a pointer to the row near the specified y-coordinate
	  (relative to the whole text). y is set to the real beginning
	  of this row
	  */ 
	Row * GetRowNearY(long & y) const;
	
	/** returns the column near the specified x-coordinate of the row 
	 x is set to the real beginning of this column
	 */ 
	int GetColumnNearX(Row * row, int & x) const;
	
	/** returns a pointer to a specified row. y is set to the beginning
	 of the row
	 */
	Row * GetRow(LyXParagraph * par,
		     LyXParagraph::size_type pos, long & y) const;
	/** returns the height of a default row, needed  for scrollbar
	 */
	int DefaultHeight() const;
   
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
   
	/* the selection cursor */
	/// 
	mutable bool selection;
	///
	mutable bool mark_set;

	///
	mutable LyXCursor sel_cursor;
	///
	LyXCursor sel_start_cursor;
	///
	mutable LyXCursor sel_end_cursor;
	/// needed for the toggling
	LyXCursor last_sel_cursor;
	///
	LyXCursor toggle_cursor;
	///
	LyXCursor toggle_end_cursor;
   
	/// need the selection cursor:
	void SetSelection();
	///
	void ClearSelection() const;
	///
	string selectionAsString() const;
	
	/// just selects the word the cursor is in
	void SelectWord();

	/** 'selects" the next word, where the cursor is not in 
	 and returns this word as string. THe cursor will be moved 
	 to the beginning of this word. 
	 With SelectSelectedWord can this be highlighted really
	 */ 
	char * SelectNextWord(float & value);
	///
	void SelectSelectedWord();
	///
	void SetCursor(LyXParagraph * par,
		       LyXParagraph::size_type pos,
		       bool setfont = true) const;
	void SetCursor(LyXCursor &, LyXParagraph * par,
		       LyXParagraph::size_type pos) const;
	///
	void SetCursorIntern(LyXParagraph * par,
			     LyXParagraph::size_type pos,
			     bool setfont = true) const;
	///
	void SetCursorFromCoordinates(int x, long y) const;
	void SetCursorFromCoordinates(LyXCursor &, int x, long y) const;
	///
	void CursorUp() const;
	///
	void CursorDown() const;
	///
	void CursorLeft() const;
	///
	void CursorRight() const;
	///
	void CursorLeftOneWord() const;
	///
	void CursorRightOneWord() const;
	///
	void CursorUpParagraph() const;
	///
	void CursorDownParagraph() const;
	///
	void CursorHome() const;
	///
	void CursorEnd() const;
	///
	void CursorTab() const;
	///
	void CursorTop() const;
	///
	void CursorBottom() const;
	///
	void Delete();
	///
	void Backspace();
	///
	void DeleteWordForward();
	///
	void DeleteWordBackward();
	///
	void DeleteLineForward();
	///
	bool SelectWordWhenUnderCursor();

	enum TextCase {
		text_lowercase = 0,
		text_capitalization = 1,
		text_uppercase = 2
	};
	/// Change the case of the word at cursor position.
	void ChangeWordCase(TextCase action);

	/** returns a printed row in a pixmap. The y value is needed to
	  decide, wether it is selected text or not. This is a strange
	  solution but faster.
	 */
	void GetVisibleRow(int offset, Row * row_ptr, long y);

	/* footnotes: */
	///
	void ToggleFootnote();
	///
	void OpenStuff();
	///
	void OpenFootnotes();
	///
	void OpenFootnote();
	///
	void CloseFootnotes();
	///
	void CloseFootnote();

	/** turn the selection into a new environment. If there is no
	  selection, create an empty environment
	 */ 
	void InsertFootnoteEnvironment(LyXParagraph::footnote_kind kind);
	///
	void MeltFootnoteEnvironment();
	///
	void CutSelection(bool = true);
	///
	void CopySelection();
	///
	void PasteSelection();
	///
	void copyEnvironmentType();
	///
	void pasteEnvironmentType();
	///
	void InsertFootnote();
	///
	void InsertMarginpar();
	///
	void InsertFigure();
	///
	void InsertTabular();

	/** the DTP switches for paragraphs. LyX will store the top settings
	 always in the first physical paragraph, the bottom settings in the
	 last. When a paragraph is broken, the top settings rest, the bottom 
	 settings are given to the new one. So I can make shure, they do not
	 duplicate themself (and you cannnot make dirty things with them! )
	 */ 
	void SetParagraph(bool line_top, bool line_bottom,
			  bool pagebreak_top, bool pagebreak_bottom,
			  VSpace const & space_top,
			  VSpace const & space_bottom,
			  LyXAlignment align, 
			  string labelwidthstring,
			  bool noindent);
	void SetParagraphExtraOpt(int type,
				  char const * width,
				  char const * widthp,
				  int alignment, bool hfill,
				  bool start_minipage);

	/* these things are for search and replace */

	/** returns true if the specified string is at the specified
	  position
	  */
	bool IsStringInText(LyXParagraph * par,
			    LyXParagraph::size_type pos,
			    char const * str) const;
	/** sets the selection over the number of characters of string,
	  no check!!
	  */
	void SetSelectionOverString(char const * str);

	/** simple replacing. The font of the first selected character
	  is used
	  */
	void ReplaceSelectionWithString(char const * str);

	/** if the string can be found: return true and set the cursor to
	  the new position */
	bool SearchForward(char const * str) const;
	bool SearchBackward(char const * str) const;

	/// needed to insert the selection
	void InsertStringA(string const & str);
	/// needed to insert the selection
	void InsertStringB(string const & str);

	/// usefull when texing from within LyX
	bool GotoNextError() const;

	/// just another feature :)
	bool GotoNextNote() const;

	/* for the greater insets */
  
	/// returns 0 if inset wasn't found
	int UpdateInset(Inset * inset);
	///
	void CheckParagraph(LyXParagraph * par,
			    LyXParagraph::size_type pos);
	///
	int NumberOfCell(LyXParagraph * par,
			 LyXParagraph::size_type pos) const;
	/* table stuff -- begin*/

	/** all table features of the text-class are put together in
	  this function. Possible values of feature are defined in table.h
	  */
	void TableFeatures(int feature, string const & val) const;
        ///
	void TableFeatures(int feature) const;

	/** pos points at the beginning of the next cell (if there is one)
	 */
	int WidthOfCell(LyXParagraph * par,
			LyXParagraph::size_type & pos) const;
	///
	void CheckParagraphInTable(LyXParagraph * par,
				   LyXParagraph::size_type pos);
	///
	void InsertCharInTable(char c);
	///
	void BackspaceInTable();
	///
	bool HitInTable(Row * row, int x) const;
	///
	bool MouseHitInTable(int x, long y) const;
	/* table stuff -- end*/
	///
	LyXParagraph * GetParFromID(int id);

	// undo functions
	/// returns false if no undo possible
	bool TextUndo();
	/// returns false if no redo possible
	bool TextRedo();
	/// used by TextUndo/TextRedo
	bool TextHandleUndo(Undo * undo);
	/// makes sure the next operation will be stored
	void FinishUndo();
	/// this is dangerous and for internal use only
	void FreezeUndo();
	/// this is dangerous and for internal use only
	void UnFreezeUndo();
	/// the flag used by FinishUndo();
	mutable bool undo_finished;
	/// a flag
	bool undo_frozen;
	///
	void SetUndo(Undo::undo_kind kind,
		     LyXParagraph const * before,
		     LyXParagraph const * end) const;
	///
	void SetRedo(Undo::undo_kind kind,
		     LyXParagraph const * before,
		     LyXParagraph const * end);
	///
	Undo * CreateUndo(Undo::undo_kind kind,
			  LyXParagraph const * before,
			  LyXParagraph const * end) const;
	/// for external use in lyx_cb.C
	void SetCursorParUndo();
	///
	void CursorLeftIntern() const;
	///
	void CursorRightIntern() const;
	///
	void RemoveTableRow(LyXCursor * cursor) const;
	///
	bool IsEmptyTableCell() const;
	///
	void toggleAppendix();
	///
	unsigned short paperWidth() const { return paperwidth; }
private:
	///
	BufferView * owner_;
	
	/// width of the paper
	unsigned short  paperwidth;

	///
	mutable Row * firstrow;
	///
	mutable Row * lastrow;

	/** Copybuffer for copy environment type
	  Asger has learned that this should be a buffer-property instead
	  Lgb has learned that 'char' is a lousy type for non-characters
	  */
	LyXTextClass::size_type copylayouttype;

	/** inserts a new row behind the specified row, increments
	 * the touched counters */
	void InsertRow(Row * row, LyXParagraph * par,
		       LyXParagraph::size_type pos) const;
	/** removes the row and reset the touched counters */
	void RemoveRow(Row * row) const;

	/** remove all following rows of the paragraph of the specified row. */
	void RemoveParagraph(Row * row) const;

	/** insert the specified paragraph behind the specified row */
	void InsertParagraph(LyXParagraph * par, Row * row) const;

	/** appends  the implizit specified paragraph behind the specified row,
	 * start at the implizit given position */
	void AppendParagraph(Row * row) const;
   
	///
	void BreakAgain(Row * row) const;
	///
	void BreakAgainOneRow(Row * row);
	/// Calculate and set the height of the row
	void SetHeightOfRow(Row * row_ptr) const;

	/** this calculates the specified parameters. needed when setting
	 * the cursor and when creating a visible row */ 
	void PrepareToPrint(Row * row, float & x,
			    float & fill_separator, 
			    float & fill_hfill,
			    float & fill_label_hfill,
			    bool bidi = true) const;
	///
	void DeleteEmptyParagraphMechanism(LyXCursor const & old_cursor) const;

	/** Updates all counters starting BEHIND the row. Changed paragraphs
	 * with a dynamic left margin will be rebroken. */ 
	void UpdateCounters(Row * row) const;
	///
	void SetCounter(LyXParagraph * par) const;
   
	/*
	 * some low level functions
	 */
	
	///
	int SingleWidth(LyXParagraph * par,
			LyXParagraph::size_type pos) const;
	///
	int SingleWidth(LyXParagraph * par,
			LyXParagraph::size_type pos, char c) const;
	///
	void draw(Row const * row,
		  LyXParagraph::size_type & pos,
		  int offset, float & x);

	/// get the next breakpoint in a given paragraph
	LyXParagraph::size_type NextBreakPoint(Row const * row,
					       int width) const;
	/// returns the minimum space a row needs on the screen in pixel
	int Fill(Row const * row, int paperwidth) const;
	
	/** returns the minimum space a manual label needs on the
	  screen in pixel */ 
	int LabelFill(Row const * row) const;

	///
	LyXParagraph::size_type BeginningOfMainBody(LyXParagraph * par) const;
	
	/** Returns the left beginning of the text.
	  This information cannot be taken from the layouts-objekt, because
	  in LaTeX the beginning of the text fits in some cases
	  (for example sections) exactly the label-width.
	  */
	int LeftMargin(Row const * row) const;
	///
	int RightMargin(Row const * row) const;
	///
	int LabelEnd (Row const * row) const;

	/** returns the number of separators in the specified row.
	  The separator on the very last column doesnt count
	  */ 
	int NumberOfSeparators(Row const * row) const;

	/** returns the number of hfills in the specified row. The
	  LyX-Hfill is a LaTeX \hfill so that the hfills at the
	  beginning and at the end were ignored. This is {\em MUCH}
	  more usefull than not to ignore!
	  */
	int NumberOfHfills(Row const * row) const;
   
	/// like NumberOfHfills, but only those in the manual label!
	int NumberOfLabelHfills(Row const * row) const;
	/** returns true, if a expansion is needed. Rules are given by 
	  LaTeX
	  */
	bool HfillExpansion(Row const * row_ptr,
			    LyXParagraph::size_type pos) const;


	///
	mutable std::vector<LyXParagraph::size_type> log2vis_list;

	///
	mutable std::vector<LyXParagraph::size_type> vis2log_list;

	///
	mutable std::vector<LyXParagraph::size_type> bidi_levels;

	///
	mutable LyXParagraph::size_type bidi_start;

	///
	mutable bool bidi_same_direction;

	///
	void ComputeBidiTables(Row *row) const;

	/// Maps positions in the visual string to positions in logical string.
	inline
	LyXParagraph::size_type log2vis(LyXParagraph::size_type pos) const {
		if (bidi_start == -1)
			return pos;
		else
			return log2vis_list[pos-bidi_start];
	}

	/// Maps positions in the logical string to positions in visual string.
	inline
	LyXParagraph::size_type vis2log(LyXParagraph::size_type pos) const {
		if (bidi_start == -1)
			return pos;
		else
			return vis2log_list[pos-bidi_start];
	}

	inline
	int bidi_level(LyXParagraph::size_type pos) const {
		if (bidi_start == -1)
			return 0;
		else
			return bidi_levels[pos-bidi_start];
	}	

	///
	unsigned char TransformChar(unsigned char c, Letter_Form form) const;

	///
	unsigned char TransformChar(unsigned char c, LyXParagraph * par,
				LyXParagraph::size_type pos) const;

	/** returns the paragraph position of the last character in the 
	  specified row
	  */
	LyXParagraph::size_type RowLast(Row const * row) const;
	///
	LyXParagraph::size_type RowLastPrintable(Row const * row) const;

	///
	void charInserted();
};

#endif
