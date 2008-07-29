// -*- C++ -*-
/**
 * \file InsetText.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSETTEXT_H
#define INSETTEXT_H

#include "Inset.h"

#include "ColorCode.h"
#include "Text.h"

namespace lyx {

class Buffer;
class BufferParams;
class BufferView;
class CompletionList;
class CursorSlice;
class Dimension;
class ParagraphList;
class InsetTabular;

/**
 A text inset is like a TeX box to write full text
 (including styles and other insets) in a given space.
 */
class InsetText : public Inset {
public:
	///
	explicit InsetText(Buffer const & buffer);
	///
	InsetText(InsetText const &);
	///
	void setBuffer(Buffer &);

	///
	Dimension const dimension(BufferView const &) const;

	/// empty inset to empty par
	void clear();
	///
	void read(Lexer & lex);
	///
	void write(std::ostream & os) const;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	docstring editMessage() const;
	///
	EDITABLE editable() const { return HIGHLY_EDITABLE; }
	///
	bool canTrackChanges() const { return true; }
	///
	InsetText * asInsetText() { return this; }
	///
	InsetText const * asInsetText() const { return this; }
	///
	Text & text() { return text_; }
	Text const & text() const { return text_; }
	///
	int latex(odocstream &, OutputParams const &) const;
	///
	int plaintext(odocstream &, OutputParams const &) const;
	///
	int docbook(odocstream &, OutputParams const &) const;
	///
	void validate(LaTeXFeatures & features) const;

	/// return x,y of given position relative to the inset's baseline
	void cursorPos(BufferView const & bv, CursorSlice const & sl,
		bool boundary, int & x, int & y) const;
	///
	InsetCode lyxCode() const { return TEXT_CODE; }
	///
	void setText(docstring const &, Font const &, bool trackChanges);
	///
	void setAutoBreakRows(bool);
	///
	bool getAutoBreakRows() const { return text_.autoBreakRows_; }
	///
	void setDrawFrame(bool);
	///
	ColorCode frameColor() const;
	///
	void setFrameColor(ColorCode);
	///
	bool showInsetDialog(BufferView *) const;
	///
	Text * getText(int i) const {
		return (i == 0) ? const_cast<Text*>(&text_) : 0;
	}
	///
	virtual bool getStatus(Cursor & cur, FuncRequest const & cmd, FuncStatus &) const;

	/// set the change for the entire inset
	void setChange(Change const & change);
	/// accept the changes within the inset
	void acceptChanges(BufferParams const & bparams);
	/// reject the changes within the inset
	void rejectChanges(BufferParams const & bparams);

	/// append text onto the existing text
	void appendParagraphs(ParagraphList &);

	///
	void addPreview(graphics::PreviewLoader &) const;

	///
	void edit(Cursor & cur, bool front, EntryDirection entry_from);
	///
	Inset * editXY(Cursor & cur, int x, int y);

	/// number of cells in this inset
	size_t nargs() const { return 1; }
	///
	ParagraphList & paragraphs();
	///
	ParagraphList const & paragraphs() const;
	///
	bool insetAllowed(InsetCode) const { return true; }
	///
	bool allowSpellCheck() const { return true; }
	///
	virtual bool isMacroScope() const { return false; }
	///
	virtual bool allowMultiPar() const { return true; }

	// Update the counters of this inset and of its contents
	void updateLabels(ParIterator const &);
	///
	void addToToc(DocIterator const &);
	///
	Inset * clone() const { return new InsetText(*this); }
	///
	bool notifyCursorLeaves(Cursor const & old, Cursor & cur);

	///
	bool completionSupported(Cursor const &) const;
	///
	bool inlineCompletionSupported(Cursor const & cur) const;
	///
	bool automaticInlineCompletion() const;
	///
	bool automaticPopupCompletion() const;
	///
	bool showCompletionCursor() const;
	///
	CompletionList const * createCompletionList(Cursor const & cur) const;
	///
	docstring completionPrefix(Cursor const & cur) const;
	///
	bool insertCompletion(Cursor & cur, docstring const & s, bool finished);
	///
	void completionPosAndDim(Cursor const &, int & x, int & y, Dimension & dim) const;

	///
	virtual docstring contextMenu(BufferView const & bv, int x, int y) const;
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
private:
	///
	void initParagraphs(BufferParams const &);
	///
	void setParagraphOwner();
	///
	bool drawFrame_;
	///
	ColorCode frame_color_;
	///
	mutable pit_type old_pit;
	///
	mutable Text text_;
};

} // namespace lyx

#endif
