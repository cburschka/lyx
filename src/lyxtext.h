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
#include "insets/insettext.h"

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
	LyXText(BufferView *);
	LyXText(InsetText *);
   
	/// Destructor
	~LyXText();

	void init(BufferView *);
	///
	mutable int number_of_rows;
	///
	mutable long height;
	mutable long width;
	/// the current font settings
	mutable LyXFont current_font;
	/// the current font
	mutable LyXFont real_current_font;

	///
	//	void owner(BufferView *);
	
	///
	LyXFont GetFont(Buffer const *, LyXParagraph * par,
			LyXParagraph::size_type pos) const;
	///
	void SetCharFont(Buffer const *, LyXParagraph * par,
			 LyXParagraph::size_type pos,
			 LyXFont const & font);
	/// returns a pointer to the very first LyXParagraph
	LyXParagraph * FirstParagraph() const;
  
	/// what you expect when pressing <enter> at cursor position
	void BreakParagraph(BufferView *, char keep_layout = 0);

	/** set layout over selection and make a total rebreak of
	  those paragraphs
	  */
	LyXParagraph * SetLayout(BufferView *, LyXCursor & actual_cursor,
				 LyXCursor & selection_start,
				 LyXCursor & selection_end,
				 LyXTextClass::size_type layout);
	void SetLayout(BufferView *, LyXTextClass::size_type layout);
	
	/// used in setlayout
	void MakeFontEntriesLayoutSpecific(Buffer const *, LyXParagraph * par);
	
	/** increment depth over selection and make a total rebreak of those 
	  paragraphs
	  */
	void IncDepth(BufferView *);
	
	/** decrement depth over selection and make a total rebreak of those  
	  paragraphs */
	void DecDepth(BufferView *);

	/** Get the depth at current cursor position
	 */
	int GetDepth() const { return cursor.par()->GetDepth(); }
	
	/** set font over selection and make a total rebreak of those
	  paragraphs.
	  toggleall defaults to false.
	  */
	void SetFont(BufferView *, LyXFont const &, bool toggleall = false);
	
	/** deletes and inserts again all paragaphs between the cursor
	  and the specified par. The Cursor is needed to set the refreshing
	  parameters. 
	  This function is needed after SetLayout and SetFont etc.
	  */
	void RedoParagraphs(BufferView *, LyXCursor const & cursor,
			    LyXParagraph const * end_par) const;
	///
	void RedoParagraph(BufferView *) const;
	
	///
	void ToggleFree(BufferView *, LyXFont const &, bool toggleall = false);
	
	/** recalculates the heights of all previous rows of the
	    specified paragraph.  needed, if the last characters font
	    has changed.  
	    */
	void RedoHeightOfParagraph(BufferView *, LyXCursor const & cursor);
	
	/** forces the redrawing of a paragraph. Needed when manipulating a 
	    right address box
	    */ 
	void RedoDrawingOfParagraph(BufferView *, LyXCursor const & cursor);
	
	/** insert a character, moves all the following breaks in the 
	  same Paragraph one to the right and make a little rebreak
	  */
	void InsertChar(BufferView *, char c);
	///
	void InsertInset(BufferView *, Inset * inset);
   
	/** Completes the insertion with a full rebreak.
	    Returns true if something was broken. */
        bool FullRebreak(BufferView *);

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
	int GetColumnNearX(BufferView *, Row * row, int & x, bool & boundary) const;
	
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
	void SetSelection(BufferView *);
	///
	void ClearSelection() const;
	///
	string selectionAsString(Buffer const *) const;
	
	/// just selects the word the cursor is in
	void SelectWord(BufferView *);

	/** 'selects" the next word, where the cursor is not in 
	 and returns this word as string. THe cursor will be moved 
	 to the beginning of this word. 
	 With SelectSelectedWord can this be highlighted really
	 */ 
	char * SelectNextWord(BufferView *, float & value);
	///
	void SelectSelectedWord(BufferView *);
	///
	void SetCursor(BufferView *, LyXParagraph * par,
		       LyXParagraph::size_type pos,
		       bool setfont = true,
		       bool boundary = false) const;

	void SetCursor(BufferView *, LyXCursor &, LyXParagraph * par,
		       LyXParagraph::size_type pos,
		       bool boundary = false) const;
	///
	void SetCursorIntern(BufferView *, LyXParagraph * par,
			     LyXParagraph::size_type pos,
			     bool setfont = true,
			     bool boundary = false) const;
	///
	void SetCurrentFont(BufferView *) const;

	///
	bool IsBoundary(Buffer const *, LyXParagraph * par,
			LyXParagraph::size_type pos) const;
	///
	bool IsBoundary(Buffer const *, LyXParagraph * par,
			 LyXParagraph::size_type pos,
			 LyXFont const & font) const;

	///
	void SetCursorFromCoordinates(BufferView *, int x, long y) const;
	void SetCursorFromCoordinates(BufferView *, LyXCursor &, int x, long y) const;
	///
	void CursorUp(BufferView *) const;
	///
	void CursorDown(BufferView *) const;
	///
	void CursorLeft(BufferView *, bool internal = true) const;
	///
	void CursorRight(BufferView *, bool internal = true) const;
	///
	void CursorLeftIntern(BufferView *, bool internal = true) const;
	///
	void CursorRightIntern(BufferView *, bool internal = true) const;
	///
	void CursorLeftOneWord(BufferView *) const;
	///
	void CursorRightOneWord(BufferView *) const;
	///
	void CursorUpParagraph(BufferView *) const;
	///
	void CursorDownParagraph(BufferView *) const;
	///
	void CursorHome(BufferView *) const;
	///
	void CursorEnd(BufferView *) const;
	///
	void CursorTab(BufferView *) const;
	///
	void CursorTop(BufferView *) const;
	///
	void CursorBottom(BufferView *) const;
	///
	void Delete(BufferView *);
	///
	void Backspace(BufferView *);
	///
	void DeleteWordForward(BufferView *);
	///
	void DeleteWordBackward(BufferView *);
	///
	void DeleteLineForward(BufferView *);
	///
	bool SelectWordWhenUnderCursor(BufferView *);

	enum TextCase {
		text_lowercase = 0,
		text_capitalization = 1,
		text_uppercase = 2
	};
	/// Change the case of the word at cursor position.
	void ChangeWordCase(BufferView *, TextCase action);

	/** returns a printed row in a pixmap. The y value is needed to
	  decide, wether it is selected text or not. This is a strange
	  solution but faster.
	 */
	void GetVisibleRow(BufferView *, int y_offset, int x_offset,
			   Row * row_ptr, long y);

	/* footnotes: */
	///
	void ToggleFootnote(BufferView *);
	///
	void OpenStuff(BufferView *);
	///
	void OpenFootnotes();
	///
	void OpenFootnote(BufferView *);
	///
	void CloseFootnotes();
	///
	void CloseFootnote(BufferView *);

	/** turn the selection into a new environment. If there is no
	  selection, create an empty environment
	 */ 
	void InsertFootnoteEnvironment(BufferView *,
				       LyXParagraph::footnote_kind kind);
	///
	void MeltFootnoteEnvironment(BufferView *);
	///
	void CutSelection(BufferView *, bool = true);
	///
	void CopySelection(BufferView *);
	///
	void PasteSelection(BufferView *);
	///
	void copyEnvironmentType();
	///
	void pasteEnvironmentType(BufferView *);
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
	void SetParagraph(BufferView *,
			  bool line_top, bool line_bottom,
			  bool pagebreak_top, bool pagebreak_bottom,
			  VSpace const & space_top,
			  VSpace const & space_bottom,
			  LyXAlignment align, 
			  string labelwidthstring,
			  bool noindent);
	void SetParagraphExtraOpt(BufferView *, int type,
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
	void SetSelectionOverString(BufferView *, char const * str);

	/** simple replacing. The font of the first selected character
	  is used
	  */
	void ReplaceSelectionWithString(BufferView *, char const * str);

	/** if the string can be found: return true and set the cursor to
	  the new position */
	bool SearchForward(BufferView *, char const * str) const;
	bool SearchBackward(BufferView *, char const * str) const;

	/// needed to insert the selection
	void InsertStringA(BufferView *, string const & str);
	/// needed to insert the selection
	void InsertStringB(BufferView *, string const & str);

	/// usefull when texing from within LyX
	bool GotoNextError(BufferView *) const;

	/// just another feature :)
	bool GotoNextNote(BufferView *) const;

	/* for the greater insets */
  
	/// returns 0 if inset wasn't found
	int UpdateInset(BufferView *, Inset * inset);
	///
	void CheckParagraph(BufferView *, LyXParagraph * par,
			    LyXParagraph::size_type pos);
	///
	int NumberOfCell(LyXParagraph * par,
			 LyXParagraph::size_type pos) const;
	/* table stuff -- begin*/

	/** all table features of the text-class are put together in
	  this function. Possible values of feature are defined in table.h
	  */
	void TableFeatures(BufferView *, int feature, string const & val) const;
        ///
	void TableFeatures(BufferView *, int feature) const;

	/** pos points at the beginning of the next cell (if there is one)
	 */
	int WidthOfCell(BufferView *, LyXParagraph * par,
			LyXParagraph::size_type & pos) const;
	///
	void CheckParagraphInTable(BufferView *, LyXParagraph * par,
				   LyXParagraph::size_type pos);
	///
	void InsertCharInTable(BufferView *, char c);
	///
	void BackspaceInTable(BufferView *);
	///
	bool HitInTable(BufferView *, Row * row, int x) const;
	///
	bool MouseHitInTable(BufferView *, int x, long y) const;
	/* table stuff -- end*/
	///
	LyXParagraph * GetParFromID(int id);

	// undo functions
	/// returns false if no undo possible
	bool TextUndo(BufferView *);
	/// returns false if no redo possible
	bool TextRedo(BufferView *);
	/// used by TextUndo/TextRedo
	bool TextHandleUndo(BufferView *, Undo * undo);
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
	void SetUndo(Buffer *, Undo::undo_kind kind,
		     LyXParagraph const * before,
		     LyXParagraph const * end) const;
	///
	void SetRedo(Buffer *, Undo::undo_kind kind,
		     LyXParagraph const * before,
		     LyXParagraph const * end);
	///
	Undo * CreateUndo(Buffer *, Undo::undo_kind kind,
			  LyXParagraph const * before,
			  LyXParagraph const * end) const;
	/// for external use in lyx_cb.C
	void SetCursorParUndo(Buffer *);
	///
	void RemoveTableRow(LyXCursor & cursor) const;
	///
	bool IsEmptyTableCell() const;
	///
	void toggleAppendix(BufferView *);
	///
	int workWidth(BufferView *) const;
	///
	//	Buffer * buffer() const;
	///
	void ComputeBidiTables(Buffer const *, Row * row) const;

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

	inline
	bool bidi_InRange(LyXParagraph::size_type pos) const {
		return bidi_start == -1 ||
			(bidi_start <= pos && pos <= bidi_end);
	}
private:
	///
	void init();
	///
	BufferView * bv_owner;
	///
	InsetText * inset_owner;
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
	void InsertParagraph(BufferView *, LyXParagraph * par, Row * row) const;

	/** appends  the implizit specified paragraph behind the specified row,
	 * start at the implizit given position */
	void AppendParagraph(BufferView *, Row * row) const;
   
	///
	void BreakAgain(BufferView *, Row * row) const;
	///
	void BreakAgainOneRow(BufferView *, Row * row);
	/// Calculate and set the height of the row
	void SetHeightOfRow(BufferView *, Row * row_ptr) const;

	/** this calculates the specified parameters. needed when setting
	 * the cursor and when creating a visible row */ 
	void PrepareToPrint(BufferView *, Row * row, float & x,
			    float & fill_separator, 
			    float & fill_hfill,
			    float & fill_label_hfill,
			    bool bidi = true) const;

	///
	void DeleteEmptyParagraphMechanism(BufferView *,
					   LyXCursor const & old_cursor) const;

	/** Updates all counters starting BEHIND the row. Changed paragraphs
	 * with a dynamic left margin will be rebroken. */ 
	void UpdateCounters(BufferView *, Row * row) const;
	///
	void SetCounter(Buffer const *, LyXParagraph * par) const;
   
	/*
	 * some low level functions
	 */
	
	///
	int SingleWidth(BufferView *, LyXParagraph * par,
			LyXParagraph::size_type pos) const;
	///
	int SingleWidth(BufferView *, LyXParagraph * par,
			LyXParagraph::size_type pos, char c) const;
	///
	void draw(BufferView *, Row const * row,
		  LyXParagraph::size_type & pos,
		  int offset, float & x);

	/// get the next breakpoint in a given paragraph
	LyXParagraph::size_type NextBreakPoint(BufferView *, Row const * row,
					       int width) const;
	/// returns the minimum space a row needs on the screen in pixel
	int Fill(BufferView *, Row * row, int workwidth) const;
	
	/** returns the minimum space a manual label needs on the
	  screen in pixel */ 
	int LabelFill(BufferView *, Row const * row) const;

	///
	LyXParagraph::size_type
	BeginningOfMainBody(Buffer const *, LyXParagraph const * par) const;
	
	/** Returns the left beginning of the text.
	  This information cannot be taken from the layouts-objekt, because
	  in LaTeX the beginning of the text fits in some cases
	  (for example sections) exactly the label-width.
	  */
	int LeftMargin(BufferView *, Row const * row) const;
	///
	int RightMargin(Buffer const *, Row const * row) const;
	///
	int LabelEnd (BufferView *, Row const * row) const;

	/** returns the number of separators in the specified row.
	  The separator on the very last column doesnt count
	  */ 
	int NumberOfSeparators(Buffer const *, Row const * row) const;

	/** returns the number of hfills in the specified row. The
	  LyX-Hfill is a LaTeX \hfill so that the hfills at the
	  beginning and at the end were ignored. This is {\em MUCH}
	  more usefull than not to ignore!
	  */
	int NumberOfHfills(Buffer const *, Row const * row) const;
   
	/// like NumberOfHfills, but only those in the manual label!
	int NumberOfLabelHfills(Buffer const *, Row const * row) const;
	/** returns true, if a expansion is needed. Rules are given by 
	  LaTeX
	  */
	bool HfillExpansion(Buffer const *, Row const * row_ptr,
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
	mutable LyXParagraph::size_type bidi_end;

	///
	mutable bool bidi_same_direction;

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
	///
	/// special owner functions
	///
	LyXParagraph * OwnerParagraph() const;
	///
	LyXParagraph * OwnerParagraph(LyXParagraph *) const;
	///
};

#endif
