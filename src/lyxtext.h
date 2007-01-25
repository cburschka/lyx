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

#include "Bidi.h"
#include "dispatchresult.h"
#include "lyxfont.h"
#include "layout.h"
#include "lyxlayout_ptr_fwd.h"
#include "ParagraphList.h"

#include <iosfwd>


namespace lyx {

class Buffer;
class BufferParams;
class BufferView;
class CursorSlice;
class DocIterator;
class ErrorList;
class FuncRequest;
class FuncStatus;
class InsetBase;
class LColor_color;
class LCursor;
class LyXTextClass;
class PainterInfo;
class Row;
class RowMetrics;
class Spacing;


/// This class encapsulates the main text data and operations in LyX
class LyXText {
public:
	/// constructor
	explicit LyXText();

	///
	LyXFont getFont(Buffer const & buffer, Paragraph const & par,
		pos_type pos) const;
	///
	void applyOuterFont(Buffer const & buffer, LyXFont &) const;
	///
	LyXFont getLayoutFont(Buffer const & buffer, pit_type pit) const;
	///
	LyXFont getLabelFont(Buffer const & buffer,
		Paragraph const & par) const;
	///
	void setCharFont(Buffer const & buffer, pit_type pit, pos_type pos,
		LyXFont const & font);
	///
	void setCharFont(Buffer const & buffer, pit_type pit, pos_type pos,
		LyXFont const & font, bool toggleall);

	/// what you expect when pressing <enter> at cursor position
	void breakParagraph(LCursor & cur, bool keep_layout = false);

	/// set layout over selection
	void setLayout(Buffer const & buffer, pit_type start, pit_type end,
		std::string const & layout);
	/// Set given layout to current cursor position.
	/// FIXME: replace LCursor with DocIterator.
	void setLayout(LCursor & cur, std::string const & layout);

	/// what type of depth change to make
	enum DEPTH_CHANGE {
		INC_DEPTH,
		DEC_DEPTH
	};
	/// Increase or decrease the nesting depth of the selected paragraph(s)
	/// FIXME: replace LCursor with DocIterator.
	void changeDepth(LCursor & cur, DEPTH_CHANGE type);

	/// Returns whether something would be changed by changeDepth
	/// FIXME: replace LCursor with DocIterator.
	bool changeDepthAllowed(LCursor & cur, DEPTH_CHANGE type) const;

	/// Set font over selection paragraphs and rebreak.
	/// FIXME: replace LCursor with DocIterator.
	void setFont(LCursor & cur, LyXFont const &, bool toggleall = false);

	///
	void toggleFree(LCursor & cur, LyXFont const &, bool toggleall = false);

	/// ???
	/// FIXME: replace LCursor with DocIterator.
	docstring getStringToIndex(LCursor const & cur);

	/// insert a character at cursor position
	/// FIXME: replace LCursor with DocIterator.
	void insertChar(LCursor & cur, char_type c);
	/// insert an inset at cursor position
	/// FIXME: replace LCursor with DocIterator.
	void insertInset(LCursor & cur, InsetBase * inset);

	/// draw text (only used for insets)
	void draw(PainterInfo & pi, int x, int y) const;
	/// draw textselection
	void drawSelection(PainterInfo & pi, int x, int y) const;

	/// try to handle that request
	/// FIXME: replace LCursor with DocIterator.
	void dispatch(LCursor & cur, FuncRequest & cmd);
	/// do we want to handle this event?
	bool getStatus(LCursor & cur, FuncRequest const & cmd,
		FuncStatus & status) const;

	/// read-only access to individual paragraph
	Paragraph const & getPar(pit_type pit) const { return pars_[pit]; }
	/// read-write access to individual paragraph
	Paragraph & getPar(pit_type pit) { return pars_[pit]; }
	// Returns the current font and depth as a message.
	/// FIXME: replace LCursor with DocIterator.
	docstring currentState(LCursor & cur);

	/** returns row near the specified
	  * y-coordinate in given paragraph (relative to the screen).
	  */
	/// FIXME: move to TextMetrics.
	Row const & getRowNearY(BufferView const & bv, int y,
		pit_type pit) const;

	/// returns the paragraph number closest to screen y-coordinate.
	/// This method uses the BufferView CoordCache to locate the
	/// paragraph. The y-coodinate is allowed to be off-screen and
	/// the CoordCache will be automatically updated if needed. This is
	/// the reason why we need a non const BufferView and why this
	/// method is non-const.
	/// FIXME: move to TextMetrics.
	pit_type getPitNearY(BufferView & bv, int y);

	/** Find the word under \c from in the relative location
	 *  defined by \c word_location.
	 *  @param from return here the start of the word
	 *  @param to return here the end of the word
	 */
	void getWord(CursorSlice & from, CursorSlice & to, word_location const);
	/// just selects the word the cursor is in
	void selectWord(LCursor & cur, word_location loc);

	/// what type of change operation to make 
	enum ChangeOp {
		ACCEPT,
		REJECT
	};
	/// accept or reject the selected change
	void acceptOrRejectChanges(LCursor & cur, ChangeOp op);
	/// accept the changes within the complete LyXText
	void acceptChanges(BufferParams const & bparams);
	/// reject the changes within the complete LyXText
	void rejectChanges(BufferParams const & bparams);

	/// returns true if par was empty and was removed
	bool setCursor(LCursor & cur, pit_type par, pos_type pos,
		       bool setfont = true, bool boundary = false);
	///
	void setCursor(CursorSlice &, pit_type par, pos_type pos);
	///
	void setCursorIntern(LCursor & cur, pit_type par,
		 pos_type pos, bool setfont = true, bool boundary = false);
	///
	void setCurrentFont(LCursor & cur);

	///
	void recUndo(LCursor & cur, pit_type first, pit_type last) const;
	///
	void recUndo(LCursor & cur, pit_type first) const;

	/// sets cursor only within this LyXText.
	/// x,y are screen coordinates
	void setCursorFromCoordinates(LCursor & cur, int x, int y);

	/// sets cursor recursively descending into nested editable insets
	/**
	\return the inset pointer if x,y is covering that inset
	\param x,y are absolute screen coordinates.
	\retval inset is non-null if the cursor is positionned inside
	*/
	/// FIXME: move to TextMetrics.
	InsetBase * editXY(LCursor & cur, int x, int y);
	
	/// Move cursor one line up.
	/**
	 * Returns true if an update is needed after the move.
	 */
	/// FIXME: move to TextMetrics.
	bool cursorUp(LCursor & cur);
	/// Move cursor one line down.
	/**
	 * Returns true if an update is needed after the move.
	 */
	/// FIXME: move to TextMetrics.
	bool cursorDown(LCursor & cur);
	/// Move cursor one position left
	/**
	 * Returns true if an update is needed after the move.
	 */
	bool cursorLeft(LCursor & cur);
	/// Move cursor one position right
	/**
	 * Returns true if an update is needed after the move.
	 */
	bool cursorRight(LCursor & cur);
	///
	bool cursorLeftOneWord(LCursor & cur);
	///
	bool cursorRightOneWord(LCursor & cur);
	///
	bool cursorUpParagraph(LCursor & cur);
	///
	bool cursorDownParagraph(LCursor & cur);
	///
	/// FIXME: move to TextMetrics.
	bool cursorHome(LCursor & cur);
	///
	/// FIXME: move to TextMetrics.
	bool cursorEnd(LCursor & cur);
	///
	void cursorPrevious(LCursor & cur);
	///
	void cursorNext(LCursor & cur);
	///
	bool cursorTop(LCursor & cur);
	///
	bool cursorBottom(LCursor & cur);
	/// Erase character at cursor. Honour change tracking
	/// FIXME: replace LCursor with DocIterator.
	bool erase(LCursor & cur);
	/// Delete character before cursor. Honour CT
	/// FIXME: replace LCursor with DocIterator.
	bool backspace(LCursor & cur);
	// Dissolve the inset under cursor
	/// FIXME: replace LCursor with DocIterator.
	bool dissolveInset(LCursor & cur);
	///
	bool selectWordWhenUnderCursor(LCursor & cur, word_location);
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
	/// Transposes the character at the cursor with the one before it
	void charsTranspose(LCursor & cur);

	/** the DTP switches for paragraphs. LyX will store the top settings
	 always in the first physical paragraph, the bottom settings in the
	 last. When a paragraph is broken, the top settings rest, the bottom
	 settings are given to the new one.
	 */
	void setParagraph(LCursor & cur,
			  Spacing const & spacing,
			  LyXAlignment align,
			  docstring const & labelwidthstring,
			  bool noindent);

	/* these things are for search and replace */

	/// needed to insert the selection
	/// FIXME: replace LCursor with DocIterator.
	void insertStringAsLines(LCursor & cur, docstring const & str);
	/// needed to insert the selection
	/// FIXME: replace LCursor with DocIterator.
	void insertStringAsParagraphs(LCursor & cur, docstring const & str);

	/// Returns an inset if inset was hit, or 0 if not.
	InsetBase * checkInsetHit(BufferView &, int x, int y);

	///
	/// FIXME: move to TextMetrics.
	int singleWidth(Buffer const &, Paragraph const & par,
		pos_type pos) const;
	///
	/// FIXME: move to TextMetrics.
	int singleWidth(Paragraph const & par, pos_type pos, char_type c,
		LyXFont const & Font) const;

	/// return the color of the canvas
	LColor_color backgroundColor() const;

	/**
	 * Returns the left beginning of the text.
	 * This information cannot be taken from the layout object, because
	 * in LaTeX the beginning of the text fits in some cases
	 * (for example sections) exactly the label-width.
	 */
	/// FIXME: move to TextMetrics.
	int leftMargin(Buffer const &, int max_width, pit_type pit, pos_type pos) const;
	int leftMargin(Buffer const &, int max_width, pit_type pit) const;

	/// access to our paragraphs
	ParagraphList const & paragraphs() const { return pars_; }
	ParagraphList & paragraphs() { return pars_; }
	/// return true if this is the main text
	bool isMainText(Buffer const &) const;

	/// is this row the last in the text?
	/// FIXME: move to TextMetrics.
	bool isLastRow(pit_type pit, Row const & row) const;
	/// is this row the first in the text?
	/// FIXME: move to TextMetrics.
	bool isFirstRow(pit_type pit, Row const & row) const;

	///
	double spacing(Buffer const & buffer, Paragraph const & par) const;
	/// make a suggestion for a label
	/// FIXME: replace LCursor with DocIterator.
	docstring getPossibleLabel(LCursor & cur) const;
	/// is this paragraph right-to-left?
	bool isRTL(Buffer const &, Paragraph const & par) const;
	///
	bool checkAndActivateInset(LCursor & cur, bool front);

	///
	void write(Buffer const & buf, std::ostream & os) const;
	/// returns whether we've seen our usual 'end' marker
	bool read(Buffer const & buf, LyXLex & lex, ErrorList & errorList);

	///
	/// FIXME: move to TextMetrics.
	int cursorX(BufferView const &, CursorSlice const & cursor,
		bool boundary) const;
	///
	/// FIXME: move to TextMetrics.
	int cursorY(BufferView const & bv, CursorSlice const & cursor,
		bool boundary) const;

	/// delete double spaces, leading spaces, and empty paragraphs around old cursor.
	/// \retval true if a change has happened and we need a redraw.
	/// FIXME: replace LCursor with DocIterator. This is not possible right
	/// now because recordUndo() is called which needs a LCursor.
	bool deleteEmptyParagraphMechanism(LCursor & cur,
		LCursor & old, bool & need_anchor_change);

	/// delete double spaces, leading spaces, and empty paragraphs
	/// from \first to \last paragraph
	void deleteEmptyParagraphMechanism(pit_type first, pit_type last, bool trackChanges);

public:
	/// the current font settings
	LyXFont current_font;
	/// the current font
	LyXFont real_current_font;
	///
	int background_color_;

	///
	mutable Bidi bidi;
	///
	ParagraphList pars_;

	/// our 'outermost' font. This is handed down from the surrounding
	// inset through the pi/mi parameter (pi.base.font)
	LyXFont font_;

	///
	bool autoBreakRows_;
private:
	/// return past-the-last paragraph influenced by a layout
	/// change on pit
	pit_type undoSpan(pit_type pit);

	// fix the cursor `cur' after a characters has been deleted at `where'
	// position. Called by deleteEmptyParagraphMechanism
	void fixCursorAfterDelete(CursorSlice & cur, CursorSlice const & where);

	// At cursor position 0, try to merge the paragraph with the one before it.
	// Ignore change tracking, i.e., physically remove the end-of-par character
	bool backspacePos0(LCursor & cur);

	///
	void deleteWordForward(LCursor & cur);
	///
	void deleteWordBackward(LCursor & cur);
	///
	void deleteLineForward(LCursor & cur);
	///
	void charInserted();
	/// set 'number' font property
	void number(LCursor & cur);

	/// paste plain text at current cursor.
	/// \param str string to paste
	/// \param asParagraphs whether to paste as paragraphs or as lines
	void pasteString(LCursor & cur, docstring const & str,
			bool asParagraphs);
};

} // namespace lyx

#endif // LYXTEXT_H
