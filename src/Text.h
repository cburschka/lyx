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

#include "DocIterator.h"
#include "ParagraphList.h"

namespace lyx {

class Buffer;
class BufferParams;
class BufferView;
class CompletionList;
class CursorSlice;
class DocIterator;
class ErrorList;
class Font;
class FontInfo;
class FuncRequest;
class FuncStatus;
class Inset;
class Cursor;
class Lexer;
class PainterInfo;
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
	FontInfo layoutFont(Buffer const & buffer, pit_type pit) const;
	///
	FontInfo labelFont(Buffer const & buffer,
		Paragraph const & par) const;
	/** Set font of character at position \p pos in paragraph \p pit.
	 *  Must not be called if \p pos denotes an inset with text contents,
	 *  and the inset is not allowed inside a font change (see below).
	 */
	void setCharFont(Buffer const & buffer, pit_type pit, pos_type pos,
		Font const & font, Font const & display_font);

	/** Needed to propagate font changes to all text cells of insets
	 *  that are not allowed inside a font change (bug 1973).
	 *  Must not be called if \p pos denotes an ordinary character or an
	 *  inset that is alowed inside a font change.
	 *  FIXME: This should be removed, see documentation of noFontChange
	 *  in insetbase.h
	 */
	void setInsetFont(BufferView const & bv, pit_type pit, pos_type pos,
		Font const & font, bool toggleall = false);

	/// what you expect when pressing \<enter\> at cursor position
	void breakParagraph(Cursor & cur, bool inverse_logic = false);

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
	void setFont(BufferView const & bv, CursorSlice const & begin,
		CursorSlice const & end, Font const &,
		bool toggleall = false);

	///
	void toggleFree(Cursor & cur, Font const &, bool toggleall = false);

	/// ???
	/// FIXME: replace Cursor with DocIterator.
	docstring getStringToIndex(Cursor const & cur);

	/// Convert the paragraphs to a string.
	/// \param AsStringParameter options. This can contain any combination of
	/// asStringParameter values. Valid examples:
	///		asString(AS_STR_LABEL)
	///		asString(AS_STR_LABEL | AS_STR_INSETS)
	///		asString(AS_STR_INSETS)
	docstring asString(int options = AS_STR_NONE) const;
	///
	docstring asString(pit_type beg, pit_type end,
		int options = AS_STR_NONE) const;

	/// insert a character at cursor position
	/// FIXME: replace Cursor with DocIterator.
	void insertChar(Cursor & cur, char_type c);
	/// insert an inset at cursor position
	/// FIXME: replace Cursor with DocIterator.
	void insertInset(Cursor & cur, Inset * inset);

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
	docstring currentState(Cursor const & cur) const;

	/** Find the word under \c from in the relative location
	 *  defined by \c word_location.
	 *  @param from return here the start of the word
	 *  @param to return here the end of the word
	 */
	void getWord(CursorSlice & from, CursorSlice & to, word_location const) const;
	/// just selects the word the cursor is in
	void selectWord(Cursor & cur, word_location loc);
	/// select all text
	void selectAll(Cursor & cur);
	/// convenience function get the previous word or an empty string
	docstring previousWord(CursorSlice const & sl) const;
	
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
	void recUndo(Cursor & cur, pit_type first, pit_type last) const;
	///
	void recUndo(Cursor & cur, pit_type first) const;

	/// Move cursor one position backwards
	/**
	 * Returns true if an update is needed after the move.
	 */
	bool cursorBackward(Cursor & cur);
	/// Move cursor visually one position to the left
	/**
	 * \param skip_inset if true, don't enter insets
	 * Returns true if an update is needed after the move.
	 */
	bool cursorVisLeft(Cursor & cur, bool skip_inset = false);
	/// Move cursor one position forward
	/**
	 * Returns true if an update is needed after the move.
	 */
	bool cursorForward(Cursor & cur);
	/// Move cursor visually one position to the right
	/**
	 * \param skip_inset if true, don't enter insets
	 * Returns true if an update is needed after the move.
	 */
	bool cursorVisRight(Cursor & cur, bool skip_inset = false);
	///
	bool cursorBackwardOneWord(Cursor & cur);
	///
	bool cursorForwardOneWord(Cursor & cur);
	///
	bool cursorVisLeftOneWord(Cursor & cur);
	///
	bool cursorVisRightOneWord(Cursor & cur);
	/// Delete from cursor up to the end of the current or next word.
	void deleteWordForward(Cursor & cur);
	/// Delete from cursor to start of current or prior word.
	void deleteWordBackward(Cursor & cur);
	///
	bool cursorUpParagraph(Cursor & cur);
	///
	bool cursorDownParagraph(Cursor & cur);
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
	/// Change the case of the word at cursor position.
	void changeCase(Cursor & cur, TextCase action);
	/// Transposes the character at the cursor with the one before it
	void charsTranspose(Cursor & cur);

	/** the DTP switches for paragraphs. LyX will store the top settings
	 always in the first physical paragraph, the bottom settings in the
	 last. When a paragraph is broken, the top settings rest, the bottom
	 settings are given to the new one.
	 This function will handle a multi-paragraph selection.
	 */
	void setParagraphs(Cursor & cur, docstring arg, bool modify = false);
	/// Sets parameters for current or selected paragraphs
	void setParagraphs(Cursor & cur, ParagraphParameters const & p);

	/* these things are for search and replace */

	/// needed to insert the selection
	/// FIXME: replace Cursor with DocIterator.
	void insertStringAsLines(Cursor & cur, docstring const & str);
	/// needed to insert the selection
	/// FIXME: replace Cursor with DocIterator.
	void insertStringAsParagraphs(Cursor & cur, docstring const & str);

	/// access to our paragraphs
	ParagraphList const & paragraphs() const { return pars_; }
	ParagraphList & paragraphs() { return pars_; }
	/// return true if this is the main text
	bool isMainText(Buffer const &) const;

	///
	double spacing(Buffer const & buffer, Paragraph const & par) const;
	/// make a suggestion for a label
	/// FIXME: replace Cursor with DocIterator.
	docstring getPossibleLabel(Cursor const & cur) const;
	/// is this paragraph right-to-left?
	bool isRTL(Buffer const &, Paragraph const & par) const;

	///
	bool checkAndActivateInset(Cursor & cur, bool front);
	///
	bool checkAndActivateInsetVisual(Cursor & cur, bool movingForward, bool movingLeft);

	///
	void write(Buffer const & buf, std::ostream & os) const;
	/// returns true if \end_document has not been read
	/// insetPtr is the containing Inset
	bool read(Buffer const & buf, Lexer & lex, ErrorList & errorList, 
	          InsetText * insetPtr);

	/// delete double spaces, leading spaces, and empty paragraphs around old cursor.
	/// \retval true if a change has happened and we need a redraw.
	/// FIXME: replace Cursor with DocIterator. This is not possible right
	/// now because recordUndo() is called which needs a Cursor.
	static bool deleteEmptyParagraphMechanism(Cursor & cur,
		Cursor & old, bool & need_anchor_change);

	/// delete double spaces, leading spaces, and empty paragraphs
	/// from \first to \last paragraph
	void deleteEmptyParagraphMechanism(pit_type first, pit_type last, bool trackChanges);

	/// To resolve macros properly the texts get their DocIterator.
	/// Every macro definition is stored with its DocIterator
	/// as well. Only those macros with a smaller iterator become 
	/// visible in a paragraph.
	DocIterator macrocontextPosition() const;
	///
	void setMacrocontextPosition(DocIterator const & pos);

	///
	bool completionSupported(Cursor const & cur) const;
	///
	CompletionList const * createCompletionList(Cursor const & cur) const;
	///
	bool insertCompletion(Cursor & cur, docstring const & s, bool /*finished*/);
	///
	docstring completionPrefix(Cursor const & cur) const;

public:
	///
	ParagraphList pars_;

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
	/// are we in a list item (description etc.)?
	bool inDescriptionItem(Cursor & cur) const;
	///
	void charInserted(Cursor & cur);
	/// set 'number' font property
	void number(Cursor & cur);

	/// paste plain text at current cursor.
	/// \param str string to paste
	/// \param asParagraphs whether to paste as paragraphs or as lines
	void pasteString(Cursor & cur, docstring const & str,
			bool asParagraphs);

	/// position of the text in the buffer.
	DocIterator macrocontext_position_;
};

} // namespace lyx

#endif // TEXT_H
