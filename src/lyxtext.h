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

#include <iosfwd>

class Buffer;
class BufferParams;
class BufferView;
class CursorSlice;
class Dimension;
class InsetBase;
class InsetBase_code;
class FuncRequest;
class FuncStatus;
class LColor_color;
class LCursor;
class LyXTextClass;
class MetricsInfo;
class PainterInfo;
class Row;
class RowMetrics;
class Spacing;


/// This class encapsulates the main text data and operations in LyX
class LyXText {
public:
	///
	typedef lyx::pos_type pos_type;
	///
	typedef lyx::pit_type pit_type;

	/// constructor
	explicit LyXText(BufferView *);
	///
	void init(BufferView *);

	/// update y coordinate cache of all paragraphs
	void updateParPositions();
	///
	LyXFont getFont(Paragraph const & par, pos_type pos) const;
	///
	LyXFont getLayoutFont(pit_type pit) const;
	///
	LyXFont getLabelFont(Paragraph const & par) const;
	///
	void setCharFont(pit_type pit, pos_type pos, LyXFont const & font);
	///
	void setCharFont(pit_type pit, pos_type pos, LyXFont const & font,
		bool toggleall);

	/// what you expect when pressing <enter> at cursor position
	void breakParagraph(LCursor & cur, char keep_layout = 0);

	/// set layout over selection
	pit_type setLayout(pit_type start, pit_type end,
		std::string const & layout);
	///
	void setLayout(LCursor & cur, std::string const & layout);

	/// what type of depth change to make
	enum DEPTH_CHANGE {
		INC_DEPTH,
		DEC_DEPTH
	};
	/// Increase or decrease the nesting depth of the selected paragraph(s)
	void changeDepth(LCursor & cur, DEPTH_CHANGE type);

	/// Returns whether something would be changed by changeDepth
	bool changeDepthAllowed(LCursor & cur, DEPTH_CHANGE type) const;

	/// Set font over selection paragraphs and rebreak.
	void setFont(LCursor & cur, LyXFont const &, bool toggleall = false);

	/// rebreaks all paragaphs between the given pars.
	void redoParagraphs(pit_type begin, pit_type end);
	/// rebreaks the given par
	void redoParagraph(pit_type pit);
	/// rebreaks the cursor par
	void redoParagraph(LCursor & cur);

	/// returns pos in given par at given x coord
	pos_type x2pos(pit_type pit, int row, int x) const;
	int pos2x(pit_type pit, pos_type pos) const;

	///
	void toggleFree(LCursor & cur, LyXFont const &, bool toggleall = false);

	///
	std::string getStringToIndex(LCursor & cur);

	/// insert a character at cursor position
	void insertChar(LCursor & cur, char c);
	/// insert an inset at cursor position
	void insertInset(LCursor & cur, InsetBase * inset);

	/// a full rebreak of the whole text
	void fullRebreak();
	/// compute text metrics
	void metrics(MetricsInfo & mi, Dimension & dim);
	/// draw text (only used for insets)
	void draw(PainterInfo & pi, int x, int y) const;
	/// draw textselection
	void drawSelection(PainterInfo & pi, int x, int y) const;
	/// returns distance of this cell to the point given by x and y
	// assumes valid position and size cache
	int dist(int x, int y) const;

	/// try to handle that request
	void dispatch(LCursor & cur, FuncRequest & cmd);
	/// do we want to handle this event?
	bool getStatus(LCursor & cur, FuncRequest const & cmd,
		FuncStatus & status) const;

	/// access to out BufferView. This should go...
	BufferView * bv();
	/// access to out BufferView. This should go...
	BufferView * bv() const;

	/// access to individual paragraphs
	Paragraph & getPar(pit_type par) const;
	// Returns the current font and depth as a message.
	std::string LyXText::currentState(LCursor & cur);

	/** returns an iterator pointing to the row near the specified
	  * y-coordinate (relative to the whole text). y is set to the
	  * real beginning of this row
	  */
	Row const & getRowNearY(int y, pit_type & pit) const;

	/** returns the column near the specified x-coordinate of the row
	 x is set to the real beginning of this column
	 */
	pos_type getColumnNearX(pit_type pit,
		Row const & row, int & x, bool & boundary) const;

	/** Find the word under \c from in the relative location
	 *  defined by \c word_location.
	 *  @param from return here the start of the word
	 *  @param to return here the end of the word
	 */
	void getWord(CursorSlice & from, CursorSlice & to, lyx::word_location const);
	/// just selects the word the cursor is in
	void selectWord(LCursor & cur, lyx::word_location loc);

	/// accept selected change
	void acceptChange(LCursor & cur);
	/// reject selected change
	void rejectChange(LCursor & cur);

	/// returns true if par was empty and was removed
	bool setCursor(LCursor & cur, pit_type par, pos_type pos,
		       bool setfont = true, bool boundary = false);
	///
	void setCursor(CursorSlice &, pit_type par,
		       pos_type pos, bool boundary = false);
	///
	void setCursorIntern(LCursor & cur, pit_type par,
	         pos_type pos, bool setfont = true, bool boundary = false);
	///
	void setCurrentFont(LCursor & cur);

	///
	void recUndo(pit_type first, pit_type last) const;
	///
	void recUndo(pit_type first) const;
	///
	void setCursorFromCoordinates(LCursor & cur, int x, int y);
	///
	InsetBase * editXY(LCursor & cur, int x, int y) const;
	///
	void cursorUp(LCursor & cur);
	///
	void cursorDown(LCursor & cur);
	///
	void cursorLeft(LCursor & cur);
	///
	void cursorRight(LCursor & cur);
	///
	void cursorLeftOneWord(LCursor & cur);
	///
	void cursorRightOneWord(LCursor & cur);
	///
	void cursorUpParagraph(LCursor & cur);
	///
	void cursorDownParagraph(LCursor & cur);
	///
	void cursorHome(LCursor & cur);
	///
	void cursorEnd(LCursor & cur);
	///
	void cursorPrevious(LCursor & cur);
	///
	void cursorNext(LCursor & cur);
	///
	void cursorTop(LCursor & cur);
	///
	void cursorBottom(LCursor & cur);
	///
	void Delete(LCursor & cur);
	///
	void backspace(LCursor & cur);
	///
	bool selectWordWhenUnderCursor(LCursor & cur, lyx::word_location);
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
	void changeCase(LCursor & cur, TextCase action);

	/** the DTP switches for paragraphs. LyX will store the top settings
	 always in the first physical paragraph, the bottom settings in the
	 last. When a paragraph is broken, the top settings rest, the bottom
	 settings are given to the new one.
	 */
	void setParagraph(LCursor & cur,
			  Spacing const & spacing,
			  LyXAlignment align,
			  std::string const & labelwidthstring,
			  bool noindent);

	/* these things are for search and replace */

	/// needed to insert the selection
	void insertStringAsLines(LCursor & cur, std::string const & str);
	/// needed to insert the selection
	void insertStringAsParagraphs(LCursor & cur, std::string const & str);

	/// Find next inset of some specified type.
	bool gotoNextInset(LCursor & cur,
		std::vector<InsetBase_code> const & codes,
		std::string const & contents = std::string());
	///
	void gotoInset(LCursor & cur,
		std::vector<InsetBase_code> const & codes, bool same_content);
	///
	void gotoInset(LCursor & cur, InsetBase_code code, bool same_content);

	/// current text width
	int width() const;

	/// current text heigth
	int height() const;

	/// updates all counters
	void updateCounters();
	/// Returns an inset if inset was hit, or 0 if not.
	InsetBase * checkInsetHit(int x, int y) const;

	///
	int singleWidth(Paragraph const & par, pos_type pos) const;
	///
	int singleWidth(Paragraph const & par,
		pos_type pos, char c, LyXFont const & Font) const;

	/// return the color of the canvas
	LColor_color backgroundColor() const;

	/**
	 * Returns the left beginning of the text.
	 * This information cannot be taken from the layout object, because
	 * in LaTeX the beginning of the text fits in some cases
	 * (for example sections) exactly the label-width.
	 */
	int leftMargin(pit_type pit, pos_type pos) const;
	int leftMargin(pit_type pit) const;
	///
	int rightMargin(Paragraph const & par) const;

	/** this calculates the specified parameters. needed when setting
	 * the cursor and when creating a visible row */
	RowMetrics computeRowMetrics(pit_type pit, Row const & row) const;

	/// access to our paragraphs
	ParagraphList & paragraphs() const;
	/// return true if this is the main text
	bool isMainText() const;

	/// return first row of text
	Row const & firstRow() const;

	/// is this row the last in the text?
	bool isLastRow(pit_type pit, Row const & row) const;
	/// is this row the first in the text?
	bool isFirstRow(pit_type pit, Row const & row) const;

	///
	double spacing(Paragraph const & par) const;
	/// make a suggestion for a label
	std::string getPossibleLabel(LCursor & cur) const;
	/// is this paragraph right-to-left?
	bool isRTL(Paragraph const & par) const;
	///
	bool checkAndActivateInset(LCursor & cur, bool front);

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

	///
	friend class LyXScreen;

public:
	///
	unsigned int width_;
	///
	int maxwidth_;
	///
	int height_;
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
	ParagraphList pars_;

	/// absolute document pixel coordinates of this LyXText
	mutable int xo_;
	mutable int yo_;

	/// our 'outermost' Font
	LyXFont font_;

private:
	/// return past-the-last paragraph influenced by a layout
	/// change on pit
	pit_type undoSpan(pit_type pit);

	/// rebreaks the given par
	void redoParagraphInternal(pit_type pit);
	/// used in setlayout
	void makeFontEntriesLayoutSpecific(BufferParams const &, Paragraph & par);

	/// Calculate and set the height of the row
	void setHeightOfRow(pit_type, Row & row);

	// fix the cursor `cur' after a characters has been deleted at `where'
	// position. Called by deleteEmptyParagraphMechanism
	void fixCursorAfterDelete(CursorSlice & cur, CursorSlice const & where);

	/// delete double space or empty paragraphs around old cursor
	bool deleteEmptyParagraphMechanism(LCursor & cur, LCursor const & old);

	///
	void setCounter(Buffer const &, pit_type pit);
	///
	void deleteWordForward(LCursor & cur);
	///
	void deleteWordBackward(LCursor & cur);
	///
	void deleteLineForward(LCursor & cur);

	/// sets row.end to the pos value *after* which a row should break.
	/// for example, the pos after which isNewLine(pos) == true
	void rowBreakPoint(pit_type pit, Row & row) const;
	/// sets row.width to the minimum space a row needs on the screen in pixel
	void setRowWidth(pit_type pit, Row & row) const;
	/// the minimum space a manual label needs on the screen in pixels
	int labelFill(Paragraph const & par, Row const & row) const;
	/// FIXME
	int labelEnd(pit_type pit) const;

	///
	void charInserted();
	/// set 'number' font property
	void number(LCursor & cur);
};

/// return the default height of a row in pixels, considering font zoom
int defaultRowHeight();

///
std::string expandLabel(LyXTextClass const & textclass,
		LyXLayout_ptr const & layout, bool appendix);


#endif // LYXTEXT_H
