// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-1999 The LyX Team.
 *
 * ======================================================*/

#ifndef LYXTEXT_H
#define LYXTEXT_H

#ifdef __GNUG__
#pragma interface
#endif

#include "definitions.h"
#include "lyxfont.h"
#include "lyxrow.h"
#include "undo.h"
#include "lyxcursor.h"
#include "lyxparagraph.h"

class Buffer;
class BufferParams;
class LyXScreen;

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
		NEED_LITTLE_REFRESH,
		///
		NEED_MORE_REFRESH,
		///
		NEED_VERY_LITTLE_REFRESH
	};

	/// points to Buffer.params
	BufferParams * parameters;
	/// points to Buffer
	Buffer * params;
	///
	int number_of_rows;
	///
	long height;
	/// the current font settings
	LyXFont current_font;
	/// the current font
	LyXFont real_current_font;

	/// Constructor
	LyXText(int paperwidth, Buffer *);
   
	/// Destructor
	~LyXText();
   
#ifdef NEW_TEXT
	///
	LyXFont GetFont(LyXParagraph * par,
			LyXParagraph::size_type pos);
	///
	void SetCharFont(LyXParagraph * par,
			 LyXParagraph::size_type pos,
			 LyXFont font);
#else
	///
	LyXFont GetFont(LyXParagraph * par, int pos);
	///
	void SetCharFont(LyXParagraph * par, int pos, LyXFont font);
#endif
	/// returns a pointer to the very first LyXParagraph
	LyXParagraph * FirstParagraph();
  
	/// what you expect when pressing <enter> at cursor position
	void BreakParagraph(char keep_layout = 0);

	/** set layout over selection and make a total rebreak of
	  those paragraphs
	  */
	void SetLayout(char layout);
	
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
	int GetDepth() { return cursor.par->GetDepth(); }
	
	/** set font over selection and make a total rebreak of those
	  paragraphs.
	  toggleall defaults to false.
	  */
	void SetFont(LyXFont font, bool toggleall = false);
	
	/** deletes and inserts again all paragaphs between the cursor
	  and the specified par .The Cursor is needed to set the refreshing
	  parameters. 
	  This function is needed after SetLayout and SetFont etc.
	  */
	void RedoParagraphs(LyXCursor cursor, LyXParagraph * end_par);
	///
	void RedoParagraph();
	
	///
	void ToggleFree(LyXFont font, bool toggleall = false);
	
	/** recalculates the heights of all previous rows of the
	    specified paragraph.  needed, if the last characters font
	    has changed.  
	    */
	void RedoHeightOfParagraph(LyXCursor cursor);
	
	/** forces the redrawing of a paragraph. Needed when manipulating a 
	    right address box
	    */ 
	void RedoDrawingOfParagraph(LyXCursor cursor);
	
	/** insert a character, moves all the following breaks in the 
	  same Paragraph one to the right and make a little rebreak
	  */
	void InsertChar(char c);
	///
	void InsertInset(Inset * inset);
   
	/// completes the insertion with a full rebreak
	int FullRebreak();
   
	/// may be important for the menu
	char * GetLayout(int row);
	///
	LyXParagraph::footnote_flag GetFootnoteFlag(int row);
	///
	Row * need_break_row;
	///
	long refresh_y;
	///
	int refresh_height;
	///
	int refresh_width;
	///
	int refresh_x;
	///
	Row * refresh_row;
	///
	int refresh_pos;
	
	/** wether the screen needs a refresh,
	   starting with refresh_y
	   */
	text_status status;
	
	/** returns a pointer to the row near the specified y-coordinate
	  (relative to the whole text). y is set to the real beginning
	  of this row
	  */ 
	Row * GetRowNearY(long & y);
	
	/** returns the column near the specified x-coordinate of the row 
	 x is set to the real beginning of this column
	 */ 
	int GetColumnNearX(Row * row, int & x);
	
	/** returns a pointer to a specified row. y is set to the beginning
	 of the row
	 */
#ifdef NEW_TEXT
	Row * GetRow(LyXParagraph * par,
		     LyXParagraph::size_type pos, long & y);
#else
	Row * GetRow(LyXParagraph * par, int pos, long & y);
#endif
	/** returns the height of a default row, needed  for scrollbar
	 */
	int DefaultHeight();
   
	/** The cursor.
	  Later this variable has to be removed. There should be now internal
	  cursor in a text (and thus not in a buffer). By keeping this it is
	  (I think) impossible to have several views with the same buffer, but
	  the cursor placed at different places.
	  */
	LyXCursor cursor;
   
	/* the selection cursor */
	/// 
	bool selection;
	///
	bool mark_set;

	///
	LyXCursor sel_cursor;
	///
	LyXCursor sel_start_cursor;
	///
	LyXCursor sel_end_cursor;
	/// needed for the toggling
	LyXCursor last_sel_cursor;
	///
	LyXCursor toggle_cursor;
	///
	LyXCursor toggle_end_cursor;
   
	/// need the selection cursor:
	void SetSelection();
	///
	void ClearSelection();

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
#ifdef NEW_TEXT
	///
	void SetCursor(LyXParagraph * par,
		       LyXParagraph::size_type pos);
	///
	void SetCursorIntern(LyXParagraph * par,
			     LyXParagraph::size_type pos);
#else
	///
	void SetCursor(LyXParagraph * par, int pos);
	///
	void SetCursorIntern(LyXParagraph * par, int pos);
#endif
	///
	void SetCursorFromCoordinates(int x, long y);
	///
	void CursorUp();
	///
	void CursorDown();
	///
	void CursorLeft();
	///
	void CursorRight();
	///
	void CursorLeftOneWord();
	///
	void CursorRightOneWord();
	///
	void CursorUpParagraph();
	///
	void CursorDownParagraph();
	///
	void CursorHome();
	///
	void CursorEnd();
	///
	void CursorTab();
	///
	void CursorTop();
	///
	void CursorBottom();
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
	int SelectWordWhenUnderCursor();
	/// Change the case of the word at cursor position
	/** Change the case of the word at cursor position.
	    action is 0 for lower case, 1 for capitalization and 2 for
	    uppercase. 
	 */
	enum TextCase {
		text_lowercase = 0,
		text_capitalization = 1,
		text_uppercase = 2
	};
	void ChangeWordCase(TextCase action);

	/** returns a printed row in a pixmap. The y value is needed to
	  decide, wether it is selected text or not. This is a strange
	  solution but faster.
	 */ 
	void GetVisibleRow(LyXScreen & scr, int offset, 
			   Row * row_ptr, long y);
					   
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
			  VSpace space_top, VSpace space_bottom,
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
#ifdef NEW_TEXT
	bool IsStringInText(LyXParagraph * par,
			    LyXParagraph::size_type pos,
			    char const * str);
#else
	bool IsStringInText(LyXParagraph * par, int pos, char const * str);
#endif
	/** sets the selection over the number of characters of string,
	  no check!!
	  */
	void SetSelectionOverString(char const * str);

	/** simple replacing. The font of the first selected character
	  is used
	  */
	void ReplaceSelectionWithString(char const * string);

	/** if the string can be found: return true and set the cursor to
	  the new position */
	bool SearchForward(char const * string);
	bool SearchBackward(char const * string);

#ifdef NEW_TEXT
	/// needed to insert the selection
	void InsertStringA(LyXParagraph::TextContainer const & text);
	/// needed to insert the selection
	void InsertStringB(LyXParagraph::TextContainer const & text);
#endif
	/// needed to insert the selection
	void InsertStringA(char const * string);
	/// needed to insert the selection
	void InsertStringB(char const * string);

	/// usefull when texing from within LyX
	bool GotoNextError();

	/// just another feature :)
	bool GotoNextNote();

	/** needed to switch between different classes this works
	  for a list of paragraphs beginning with the specified par 
	  return value is the number of wrong conversions
	  */ 
	int SwitchLayoutsBetweenClasses(char class1, char class2,
					LyXParagraph * par);

	/* for the greater insets */
  
	/// returns 0 if inset wasn't found
	int UpdateInset(Inset * inset);
#ifdef NEW_TEXT
	///
	void CheckParagraph(LyXParagraph * par,
			    LyXParagraph::size_type pos);
	///
	int NumberOfCell(LyXParagraph * par,
			 LyXParagraph::size_type pos);
#else
	///
	void CheckParagraph(LyXParagraph * par, int pos);
	///
	int NumberOfCell(LyXParagraph * par, int pos);
#endif
	/* table stuff -- begin*/

	/** all table features of the text-class are put together in
	  this function. Possible values of feature are defined in table.h
	  */
	void TableFeatures(int feature, string val);
        ///
	void TableFeatures(int feature);

	/** pos points at the beginning of the next cell (if there is one)
	 */
#ifdef NEW_TEXT
	int WidthOfCell(LyXParagraph * par, LyXParagraph::size_type & pos);
	///
	void CheckParagraphInTable(LyXParagraph * par,
				   LyXParagraph::size_type pos);
#else
	int WidthOfCell(LyXParagraph * par, int & pos);
	///
	void CheckParagraphInTable(LyXParagraph * par, int pos);
#endif
	///
	void InsertCharInTable(char c);
	///
	void BackspaceInTable();
	///
	char HitInTable(Row * row, int x);
	///
	bool MouseHitInTable(int x, long y);
	/* table stuff -- end*/
	///
	LyXParagraph * GetParFromID(int id);

	// undo functions
	/// returns false if no undo possible
	bool  TextUndo();
	/// returns false if no redo possible
	bool  TextRedo();
	/// used by TextUndo/TextRedo
	bool TextHandleUndo(Undo * undo);
	/// makes sure the next operation will be stored
	void FinishUndo();
	/// this is dangerous and for internal use only
	void FreezeUndo();
	/// this is dangerous and for internal use only
	void UnFreezeUndo();
	/// the flag used by FinishUndo();
	bool undo_finished;
	/// a flag
	bool undo_frozen;
	///
	void SetUndo(Undo::undo_kind kind, LyXParagraph * before,
		     LyXParagraph * end);
	///
	void SetRedo(Undo::undo_kind kind, LyXParagraph * before,
		     LyXParagraph * end);
	///
	Undo * CreateUndo(Undo::undo_kind kind, LyXParagraph * before,
			  LyXParagraph * end);
	/// for external use in lyx_cb.C
	void SetCursorParUndo();
	///
	void CursorLeftIntern();
	///
	void CursorRightIntern();
        ///
        void RemoveTableRow(LyXCursor * cursor);
        ///
        bool IsEmptyTableRow(LyXCursor * cursor);
        ///
        bool IsEmptyTableCell();
        ///
        void toggleAppendix();

private:
	///
	Row * firstrow;
	///
	Row * lastrow;

	/** Copybuffer for copy environment type
	  Asger has learned that this should be a buffer-property instead
	  Lgb has learned that 'char' is a lousy type for non-characters
	  */
	char copylayouttype;

	/// the currentrow is needed to access rows faster*/ 
	Row * currentrow;		/* pointer to the current row  */
	/// position in the text 
	long  currentrow_y;
	/// width of the paper
	unsigned short  paperwidth;
   
	/** inserts a new row behind the specified row, increments
	 * the touched counters */
#ifdef NEW_TEXT
	void InsertRow(Row * row, LyXParagraph * par,
		       LyXParagraph::size_type pos );
#else
	void InsertRow(Row * row, LyXParagraph * par, int pos );
#endif
	/** removes the row and reset the touched counters */
	void RemoveRow(Row * row);

	/** remove all following rows of the paragraph of the specified row. */
	void RemoveParagraph(Row * row);

	/** insert the specified paragraph behind the specified row */
	void InsertParagraph(LyXParagraph * par, Row * row);

	/** appends  the implizit specified paragraph behind the specified row,
	 * start at the implizit given position */
	void AppendParagraph(Row * row);
   
	///
	void BreakAgain(Row * row);
	///
	void BreakAgainOneRow(Row * row);
	///
	void SetHeightOfRow(Row * row_ptr); /* calculate and set the height 
					    * of the row */

	/** this calculates the specified parameters. needed when setting
	 * the cursor and when creating a visible row */ 
	void PrepareToPrint(Row * row, float & x, float & fill_separator, 
			    float & fill_hfill, float & fill_label_hfill);
	///
	void DeleteEmptyParagraphMechanism(LyXCursor old_cursor);

	/** Updates all counters starting BEHIND the row. Changed paragraphs
	 * with a dynamic left margin will be rebroken. */ 
	void UpdateCounters(Row * row);
	///
	void SetCounter(LyXParagraph * par);
   
	/*
	 * some low level functions
	 */
	
#ifdef NEW_TEXT
	///
	int SingleWidth(LyXParagraph * par,
			LyXParagraph::size_type pos);
	///
	int SingleWidth(LyXParagraph * par,
			LyXParagraph::size_type pos, char c);
	///
	void Draw(Row * row, LyXParagraph::size_type & pos,
		  LyXScreen & scr,
		  int offset, float & x);
	/// get the next breakpoint in a given paragraph
	LyXParagraph::size_type NextBreakPoint(Row * row,
					       int width);
#else
	///
	int SingleWidth(LyXParagraph * par, int pos);
	///
	int SingleWidth(LyXParagraph * par, int pos, char c);
	///
	void Draw(Row * row, int & pos, LyXScreen & scr,
		  int offset, float & x);
	/// get the next breakpoint in a given paragraph
	int NextBreakPoint(Row * row, int width);
#endif
	/// returns the minimum space a row needs on the screen in pixel
	int Fill(Row * row, int paperwidth);
	
	/** returns the minimum space a manual label needs on the
	  screen in pixel */ 
	int LabelFill(Row * row);

#ifdef NEW_TEXT
	///
	LyXParagraph::size_type BeginningOfMainBody(LyXParagraph * par);
#else
	///
	int BeginningOfMainBody(LyXParagraph * par);
#endif
	/** Returns the left beginning of the text.
	  This information cannot be taken from the layouts-objekt, because
	  in LaTeX the beginning of the text fits in some cases
	  (for example sections) exactly the label-width.
	  */
	int LeftMargin(Row * row);
	///
	int RightMargin(Row * row);
	///
	int LabelEnd (Row * row);

	/** returns the number of separators in the specified row.
	  The separator on the very last column doesnt count
	  */ 
	int NumberOfSeparators(Row * row);

	/** returns the number of hfills in the specified row. The
	  LyX-Hfill is a LaTeX \hfill so that the hfills at the
	  beginning and at the end were ignored. This is {\em MUCH}
	  more usefull than not to ignore!
	  */
	int NumberOfHfills(Row * row);
   
	/// like NumberOfHfills, but only those in the manual label!
	int NumberOfLabelHfills(Row * row);

	/** returns true, if a expansion is needed. Rules are given by 
	  LaTeX
	  */
#ifdef NEW_TEXT
	bool HfillExpansion(Row * row_ptr,
			    LyXParagraph::size_type pos);
	/** returns the paragraph position of the last character in the 
	  specified row
	  */
	LyXParagraph::size_type RowLast(Row * row);
#else
	bool HfillExpansion(Row * row_ptr, int pos);
	/** returns the paragraph position of the last character in the 
	  specified row
	  */
	int RowLast(Row * row);
#endif
};

#endif
