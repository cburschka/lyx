// -*- C++ -*-
/**
 * \file Text.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author Lars Gullik Bjønnes
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef TEXT_H
#define TEXT_H

#include "Bidi.h"
#include "DispatchResult.h"
#include "Font.h"
#include "Layout.h"
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
class Inset;
class Color_color;
class Cursor;
class PainterInfo;
class Row;
class RowMetrics;
class Spacing;


/// This class encapsulates the main text data and operations in LyX
class Text {
public:
	/// constructor
	explicit Text();

	/// \return true if there's no content at all.
	/// \warning a non standard layout on an empty paragraph doesn't
	// count as empty.
	bool empty() const;

	///
	Font getFont(Buffer const & buffer, Paragraph const & par,
		pos_type pos) const;
	///
	void applyOuterFont(Buffer const & buffer, Font &) const;
	///
	Font getLayoutFont(Buffer const & buffer, pit_type pit) const;
	///
	Font getLabelFont(Buffer const & buffer,
		Paragraph const & par) const;
	/** Set font of character at position \p pos in paragraph \p pit.
	 *  Must not be called if \p pos denotes an inset with text contents,
	 *  and the inset is not allowed inside a font change (see below).
	 */
	void setCharFont(Buffer const & buffer, pit_type pit, pos_type pos,
		Font const & font);

	/** Needed to propagate font changes to all text cells of insets
	 *  that are not allowed inside a font change (bug 1973).
	 *  Must not be called if \p pos denotes an ordinary character or an
	 *  inset that is alowed inside a font change.
	 *  FIXME: This should be removed, see documentation of noFontChange
	 *  in insetbase.h
	 */
	void setInsetFont(Buffer const & buffer, pit_type pit, pos_type pos,
		Font const & font, bool toggleall = false);

	/// what you expect when pressing \<enter\> at cursor position
	void breakParagraph(Cursor & cur, bool keep_layout = false);

	/// set layout over selection
	void setLayout(Buffer const & buffer, pit_type start, pit_type end,
		docstring const & layout);
	/// Set given layout to current cursor position.
	/// FIXME: replace Cursor with DocIterator.
	void setLayout(Cursor & cur, docstring const & layout);

	/// what type of depth change to make
	enum DEPTH_CHANGE {
		INC_DEPTH,
		DEC_DEPTH
	};
	/// Increase or decrease the nesting depth of the selected paragraph(s)
	/// FIXME: replace Cursor with DocIterator.
	void changeDepth(Cursor & cur, DEPTH_CHANGE type);

	/// Returns whether something would be changed by changeDepth
	/// FIXME: replace Cursor with DocIterator.
	bool changeDepthAllowed(Cursor & cur, DEPTH_CHANGE type) const;

	/// Set font over selection paragraphs and rebreak.
	/// FIXME: replace Cursor with DocIterator.
	void setFont(Cursor & cur, Font const &, bool toggleall = false);
	/// Set font from \p begin to \p end and rebreak.
	void setFont(Buffer const & buffer, CursorSlice const & begin,
		CursorSlice const & end, Font const &,
		bool toggleall = false);

	///
	void toggleFree(Cursor & cur, Font const &, bool toggleall = false);

	/// ???
	/// FIXME: replace Cursor with DocIterator.
	docstring getStringToIndex(Cursor const & cur);

	/// insert a character at cursor position
	/// FIXME: replace Cursor with DocIterator.
	void insertChar(Cursor & cur, char_type c);
	/// insert an inset at cursor position
	/// FIXME: replace Cursor with DocIterator.
	void insertInset(Cursor & cur, Inset * inset);

	/// draw text (only used for insets)
	void draw(PainterInfo & pi, int x, int y) const;
	/// draw textselection
	void drawSelection(PainterInfo & pi, int x, int y) const;

	/// try to handle that request
	/// FIXME: replace Cursor with DocIterator.
	void dispatch(Cursor & cur, FuncRequest & cmd);
	/// do we want to handle this event?
	bool getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & status) const;

	/// read-only access to individual paragraph
	Paragraph const & getPar(pit_type pit) const { return pars_[pit]; }
	/// read-write access to individual paragraph
	Paragraph & getPar(pit_type pit) { return pars_[pit]; }
	// Returns the current font and depth as a message.
	/// FIXME: replace Cursor with DocIterator.
	docstring currentState(Cursor & cur);

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
	/// the reason why we need a non const BufferView.
	/// FIXME: move to TextMetrics.
	pit_type getPitNearY(BufferView & bv, int y) const;

	/** Find the word under \c from in the relative location
	 *  defined by \c word_location.
	 *  @param from return here the start of the word
	 *  @param to return here the end of the word
	 */
	void getWord(CursorSlice & from, CursorSlice & to, word_location const);
	/// just selects the word the cursor is in
	void selectWord(Cursor & cur, word_location loc);

	/// what type of change operation to make
	enum ChangeOp {
		ACCEPT,
		REJECT
	};
	/// accept or reject the selected change
	void acceptOrRejectChanges(Cursor & cur, ChangeOp op);
	/// accept the changes within the complete Text
	void acceptChanges(BufferParams const & bparams);
	/// reject the changes within the complete Text
	void rejectChanges(BufferParams const & bparams);

	/// returns true if par was empty and was removed
	bool setCursor(Cursor & cur, pit_type par, pos_type pos,
		       bool setfont = true, bool boundary = false);
	///
	void setCursor(CursorSlice &, pit_type par, pos_type pos);
	///
	void setCursorIntern(Cursor & cur, pit_type par,
		 pos_type pos, bool setfont = true, bool boundary = false);
	///
	void setCurrentFont(Cursor & cur);

	///
	void recUndo(Cursor & cur, pit_type first, pit_type last) const;
	///
	void recUndo(Cursor & cur, pit_type first) const;

	/// sets cursor only within this Text.
	/// x,y are screen coordinates
	void setCursorFromCoordinates(Cursor & cur, int x, int y);

	/// sets cursor recursively descending into nested editable insets
	/**
	\return the inset pointer if x,y is covering that inset
	\param x,y are absolute screen coordinates.
	\retval inset is non-null if the cursor is positionned inside
	*/
	/// FIXME: move to TextMetrics.
	/// FIXME: cleanup to use BufferView::getCoveringInset() and
	/// setCursorFromCoordinates() instead of checkInsetHit().
	Inset * editXY(Cursor & cur, int x, int y);

	/// Move cursor one position left
	/**
	 * Returns true if an update is needed after the move.
	 */
	bool cursorLeft(Cursor & cur);
	/// Move cursor one position right
	/**
	 * Returns true if an update is needed after the move.
	 */
	bool cursorRight(Cursor & cur);
	///
	bool cursorLeftOneWord(Cursor & cur);
	///
	bool cursorRightOneWord(Cursor & cur);
	///
	bool cursorUpParagraph(Cursor & cur);
	///
	bool cursorDownParagraph(Cursor & cur);
	///
	/// FIXME: move to TextMetrics.
	bool cursorHome(Cursor & cur);
	///
	/// FIXME: move to TextMetrics.
	bool cursorEnd(Cursor & cur);
	///
	void cursorPrevious(Cursor & cur);
	///
	void cursorNext(Cursor & cur);
	///
	bool cursorTop(Cursor & cur);
	///
	bool cursorBottom(Cursor & cur);
	/// Erase character at cursor. Honour change tracking
	/// FIXME: replace Cursor with DocIterator.
	bool erase(Cursor & cur);
	/// Delete character before cursor. Honour CT
	/// FIXME: replace Cursor with DocIterator.
	bool backspace(Cursor & cur);
	// Dissolve the inset under cursor
	/// FIXME: replace Cursor with DocIterator.
	bool dissolveInset(Cursor & cur);
	///
	bool selectWordWhenUnderCursor(Cursor & cur, word_location);
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
	void changeCase(Cursor & cur, TextCase action);
	/// Transposes the character at the cursor with the one before it
	void charsTranspose(Cursor & cur);

	/** the DTP switches for paragraphs. LyX will store the top settings
	 always in the first physical paragraph, the bottom settings in the
	 last. When a paragraph is broken, the top settings rest, the bottom
	 settings are given to the new one.
	 */
	void setParagraph(Cursor & cur,
			  Spacing const & spacing,
			  LyXAlignment align,
			  docstring const & labelwidthstring,
			  bool noindent);

	/* these things are for search and replace */

	/// needed to insert the selection
	/// FIXME: replace Cursor with DocIterator.
	void insertStringAsLines(Cursor & cur, docstring const & str);
	/// needed to insert the selection
	/// FIXME: replace Cursor with DocIterator.
	void insertStringAsParagraphs(Cursor & cur, docstring const & str);

	/// Returns an inset if inset was hit, or 0 if not.
	/// \warning This method is not recursive! It will return the
	/// outermost inset within this Text.
	/// \sa BufferView::getCoveringInset() to get the innermost inset.
	Inset * checkInsetHit(BufferView &, int x, int y);

	///
	/// FIXME: move to TextMetrics.
	int singleWidth(Buffer const &, Paragraph const & par,
		pos_type pos) const;
	///
	/// FIXME: move to TextMetrics.
	int singleWidth(Paragraph const & par, pos_type pos, char_type c,
		Font const & Font) const;

	/// return the color of the canvas
	Color_color backgroundColor() const;

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
	/// FIXME: replace Cursor with DocIterator.
	docstring getPossibleLabel(Cursor & cur) const;
	/// is this paragraph right-to-left?
	bool isRTL(Buffer const &, Paragraph const & par) const;
	/// is this position in the paragraph right-to-left?
	bool isRTL(Buffer const & buffer, CursorSlice const & sl, bool boundary) const;
	/// is between pos-1 and pos an RTL<->LTR boundary?
	bool isRTLBoundary(Buffer const & buffer, Paragraph const & par,
	  pos_type pos) const;
	/// would be a RTL<->LTR boundary between pos and the given font?
	bool isRTLBoundary(Buffer const & buffer, Paragraph const & par,
	  pos_type pos, Font const & font) const;

	///
	bool checkAndActivateInset(Cursor & cur, bool front);

	///
	void write(Buffer const & buf, std::ostream & os) const;
	/// returns whether we've seen our usual 'end' marker
	bool read(Buffer const & buf, Lexer & lex, ErrorList & errorList);

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
	/// FIXME: replace Cursor with DocIterator. This is not possible right
	/// now because recordUndo() is called which needs a Cursor.
	static bool deleteEmptyParagraphMechanism(Cursor & cur,
		Cursor & old, bool & need_anchor_change);

	/// delete double spaces, leading spaces, and empty paragraphs
	/// from \first to \last paragraph
	void deleteEmptyParagraphMechanism(pit_type first, pit_type last, bool trackChanges);

public:
	/// the current font settings
	Font current_font;
	/// the current font
	Font real_current_font;
	///
	int background_color_;

	///
	ParagraphList pars_;

	/// our 'outermost' font. This is handed down from the surrounding
	// inset through the pi/mi parameter (pi.base.font)
	Font font_;

	///
	bool autoBreakRows_;
private:
	/// return past-the-last paragraph influenced by a layout
	/// change on pit
	pit_type undoSpan(pit_type pit);

	// fix the cursor `cur' after a characters has been deleted at `where'
	// position. Called by deleteEmptyParagraphMechanism
	static void fixCursorAfterDelete(CursorSlice & cur, CursorSlice const & where);

	// At cursor position 0, try to merge the paragraph with the one before it.
	// Ignore change tracking, i.e., physically remove the end-of-par character
	bool backspacePos0(Cursor & cur);
	/// handle the case where bibitems were deleted
	bool handleBibitems(Cursor & cur);

	///
	void deleteWordForward(Cursor & cur);
	///
	void deleteWordBackward(Cursor & cur);
	///
	void deleteLineForward(Cursor & cur);
	///
	void charInserted();
	/// set 'number' font property
	void number(Cursor & cur);
	/// draw selection for a single row
	void drawRowSelection(PainterInfo & pi, int x, Row const & row,
		DocIterator const & beg, DocIterator const & end, 
		bool drawOnBegMargin, bool drawOnEndMargin) const;

	/// paste plain text at current cursor.
	/// \param str string to paste
	/// \param asParagraphs whether to paste as paragraphs or as lines
	void pasteString(Cursor & cur, docstring const & str,
			bool asParagraphs);
};

} // namespace lyx

#endif // LYXTEXT_H
