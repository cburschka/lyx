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
#include "Bidi.h"
#include "dispatchresult.h"
#include "lyxfont.h"
#include "layout.h"
#include "lyxlayout_ptr_fwd.h"
#include "ParagraphList_fwd.h"
#include "RowList_fwd.h"

#include <iosfwd>

class Buffer;
class BufferParams;
class BufferView;
class CursorSlice;
class Dimension;
class InsetBase;
class InsetOld_code;
class FuncRequest;
class LColor_color;
class LCursor;
class LyXTextClass;
class MetricsInfo;
class PainterInfo;
class Paragraph;
class Row;
class Spacing;
class UpdatableInset;
class VSpace;


/// This class encapsulates the main text data and operations in LyX
class LyXText {
public:
	/// constructor
	LyXText(BufferView *, bool ininset);
	///
	void init(BufferView *);

	/// update y coordinate cache of all paragraphs
	void updateParPositions();
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
	setLayout(ParagraphList::iterator start,
		  ParagraphList::iterator end,
		  std::string const & layout);
	///
	void setLayout(std::string const & layout);

	/// Increase or decrease the nesting depth of the selected paragraph(s)
	void changeDepth(bv_funcs::DEPTH_CHANGE type);

	/// Returns whether something would be changed by changeDepth
	bool changeDepthAllowed(bv_funcs::DEPTH_CHANGE type);

	/// get the depth at current cursor position
	int getDepth() const;

	/** set font over selection and make a total rebreak of those
	  paragraphs.
	  toggleall defaults to false.
	  */
	void setFont(LyXFont const &, bool toggleall = false);

	/// rebreaks all paragaphs between the given pars.
	void redoParagraphs(ParagraphList::iterator begin,
			    ParagraphList::iterator end);
	/// rebreaks the given par
	void redoParagraph(ParagraphList::iterator pit);
	/// rebreaks the cursor par
	void redoParagraph();

	///
	void toggleFree(LyXFont const &, bool toggleall = false);

	///
	std::string getStringToIndex();

	/// insert a character at cursor position
	void insertChar(char c);
	/// insert an inset at cursor position
	void insertInset(InsetBase * inset);

	/// a full rebreak of the whole text
	void fullRebreak();
	/// compute text metrics
	void metrics(MetricsInfo & mi, Dimension & dim);
	/// draw text (only used for insets)
	void draw(PainterInfo & pi, int x, int y) const;

	/// try to handle that request
	DispatchResult dispatch(LCursor & cur, FuncRequest const & cmd);
	/// access to out BufferView. This should go...
	BufferView * bv();
	/// access to out BufferView. This should go...
	BufferView * bv() const;

	/// returns an iterator pointing to a cursor paragraph
	ParagraphList::iterator getPar(CursorSlice const & cursor) const;
	///
	ParagraphList::iterator getPar(lyx::paroffset_type par) const;
	///
	int parOffset(ParagraphList::iterator pit) const;
	/// convenience
	ParagraphList::iterator cursorPar() const;
	///
	RowList::iterator cursorRow() const;

	/** returns an iterator pointing to the row near the specified
	  * y-coordinate (relative to the whole text). y is set to the
	  * real beginning of this row
	  */
	RowList::iterator getRowNearY(int y,
		ParagraphList::iterator & pit) const;

	/** returns the column near the specified x-coordinate of the row
	 x is set to the real beginning of this column
	 */
	lyx::pos_type getColumnNearX(ParagraphList::iterator pit,
		Row const & row, int & x, bool & boundary) const;

	/** Find the word under \c from in the relative location
	 *  defined by \c word_location.
	 *  @param from return here the start of the word
	 *  @param to return here the end of the word
	 */
	void getWord(CursorSlice & from, CursorSlice & to, lyx::word_location const);
	/// just selects the word the cursor is in
	void selectWord(lyx::word_location loc);
	/// returns the inset at cursor (if it exists), 0 otherwise
	InsetBase * getInset() const;

	/// accept selected change
	void acceptChange();

	/// reject selected change
	void rejectChange();

	///
	void setCursor(ParagraphList::iterator pit, lyx::pos_type pos);
	/// returns true if par was empty and was removed
	bool setCursor(lyx::paroffset_type par, lyx::pos_type pos,
		       bool setfont = true, bool boundary = false);
	///
	void setCursor(CursorSlice &, lyx::paroffset_type par,
		       lyx::pos_type pos, bool boundary = false);
	///
	void setCursorIntern(lyx::paroffset_type par, lyx::pos_type pos,
			     bool setfont = true, bool boundary = false);
	///
	void setCurrentFont();

	///
	void recUndo(lyx::paroffset_type first, lyx::paroffset_type last) const;
	///
	void recUndo(lyx::paroffset_type first) const;
	///
	void setCursorFromCoordinates(int x, int y);
	///
	void setCursorFromCoordinates(CursorSlice &, int x, int y);
	///
	void edit(LCursor & cur, int x, int y);
	///
	void cursorUp(bool selecting = false);
	///
	void cursorDown(bool selecting = false);
	///
	bool cursorLeft(bool internal = true);
	///
	bool cursorRight(bool internal = true);
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

	/// returns success
	bool toggleInset();
	///
	void cutSelection(bool doclear = true, bool realcut = true);
	///
	void copySelection();
	///
	void pasteSelection(size_t sel_index = 0);

	/** the DTP switches for paragraphs. LyX will store the top settings
	 always in the first physical paragraph, the bottom settings in the
	 last. When a paragraph is broken, the top settings rest, the bottom
	 settings are given to the new one.
	 */
	void setParagraph(
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
	bool gotoNextInset(std::vector<InsetOld_code> const & codes,
			   std::string const & contents = std::string());
	///
	void gotoInset(std::vector<InsetOld_code> const & codes,
		       bool same_content);
	///
	void gotoInset(InsetOld_code code, bool same_content);

	/// current max text width
	int textWidth() const;

	/// updates all counters
	void updateCounters();
	/// Returns an inset if inset was hit, or 0 if not.
	InsetBase * checkInsetHit(int x, int y);

	///
	int singleWidth(ParagraphList::iterator pit, lyx::pos_type pos) const;
	///
	int singleWidth(ParagraphList::iterator pit,
		lyx::pos_type pos, char c, LyXFont const & Font) const;

	/// return the color of the canvas
	LColor_color backgroundColor() const;

	/**
	 * Returns the left beginning of the text.
	 * This information cannot be taken from the layout object, because
	 * in LaTeX the beginning of the text fits in some cases
	 * (for example sections) exactly the label-width.
	 */
	int leftMargin(ParagraphList::iterator pit, lyx::pos_type pos) const;
	int leftMargin(ParagraphList::iterator pit) const;
	///
	int rightMargin(Paragraph const & par) const;

	/** this calculates the specified parameters. needed when setting
	 * the cursor and when creating a visible row */
	void prepareToPrint(ParagraphList::iterator pit, Row & row) const;

	/// access to our paragraphs
	ParagraphList & paragraphs() const;
	/// return true if this is owned by an inset.
	bool isInInset() const;
	/// the first of our paragraphs
	ParagraphList::iterator firstPar() const;
	/// the last of our paragraphs
	ParagraphList::iterator lastPar() const;
	/// one past the last of our paragraphs
	ParagraphList::iterator endPar() const;
	
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

	/// is this row the last in the text?
	bool isLastRow(ParagraphList::iterator pit, Row const & row) const;
	/// is this row the first in the text?
	bool isFirstRow(ParagraphList::iterator pit, Row const & row) const;

	///
	std::string selectionAsString(Buffer const & buffer, bool label) const;
	///
	double spacing(Paragraph const &) const;
	///
	void cursorLeftOneWord(CursorSlice &);
	///
	void cursorRightOneWord(CursorSlice &);

	///
	DispatchResult moveRight();
	///
	DispatchResult moveLeft();
	///
	DispatchResult moveRightIntern(bool front,
		bool activate_inset, bool selecting);
	///
	DispatchResult moveLeftIntern(bool front,
		bool activate_inset, bool selecting);
	///
	DispatchResult moveUp();
	///
	DispatchResult moveDown();
	///
	bool checkAndActivateInset(bool front);

	///
	void write(Buffer const & buf, std::ostream & os) const;
	/// returns whether we've seen our usual 'end' marker
	bool read(Buffer const & buf, LyXLex & lex);

	///
	int ascent() const;
	///
	int descent() const;
	///
	int cursorX(CursorSlice const & cursor) const;
	///
	int cursorY(CursorSlice const & cursor) const;

	/// the topmost cursor slice
	CursorSlice & cursor();
	/// the topmost cursor slice
	CursorSlice const & cursor() const;

	friend class LyXScreen;

public:
	///
	int height;
	///
	unsigned int width;
	///
	int textwidth_;
	/// the current font settings
	LyXFont current_font;
	/// the current font
	LyXFont real_current_font;
	/// our buffer's default layout font
	LyXFont defaultfont_;
	///
	int background_color_;

	/// only the top-level LyXText has this non-zero
	BufferView * bv_owner;

	///
	mutable Bidi bidi;
	///
	bool in_inset_;
	///
	ParagraphList paragraphs_;

	/// absolute document pixel coordinates of this LyXText
	mutable int xo_;
	mutable int yo_;

	/// our 'outermost' Font
	LyXFont font_;


private:
	/// return past-the-last paragraph influenced by a layout
	/// change on pit
	ParagraphList::iterator undoSpan(ParagraphList::iterator pit);
	
	/// rebreaks the given par
	void redoParagraphInternal(ParagraphList::iterator pit);
	/// used in setlayout
	void makeFontEntriesLayoutSpecific(BufferParams const &, Paragraph & par);

	/// Calculate and set the height of the row
	void setHeightOfRow(ParagraphList::iterator, Row & row);

	// fix the cursor `cur' after a characters has been deleted at `where'
	// position. Called by deleteEmptyParagraphMechanism
	void fixCursorAfterDelete(CursorSlice & cur, CursorSlice const & where);

	/// delete double space (false) or empty paragraphs (true) around old_cursor
	bool deleteEmptyParagraphMechanism(CursorSlice const & old_cursor);

	///
	void setCounter(Buffer const &, ParagraphList::iterator pit);
	///
	void deleteWordForward();
	///
	void deleteWordBackward();
	///
	void deleteLineForward();

	/// sets row.end to the pos value *after* which a row should break.
	/// for example, the pos after which isNewLine(pos) == true
	void rowBreakPoint(ParagraphList::iterator pit, Row & row) const;

	/// sets row.witdh to the minimum space a row needs on the screen in pixel
	void fill(ParagraphList::iterator pit, Row & row, int workwidth) const;

	/**
	 * returns the minimum space a manual label needs on the
	 * screen in pixels
	 */
	int labelFill(ParagraphList::iterator pit, Row const & row) const;

	/// FIXME
	int labelEnd(ParagraphList::iterator pit) const;

	///
	void charInserted();
	/// set 'number' font property
	void number();
	/// is the cursor paragraph right-to-left?
	bool rtl() const;
};

/// return the default height of a row in pixels, considering font zoom
extern int defaultRowHeight();

///
std::string expandLabel(LyXTextClass const & textclass,
		LyXLayout_ptr const & layout, bool appendix);

#endif // LYXTEXT_H
