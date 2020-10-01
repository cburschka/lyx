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

class CompletionList;
class CursorSlice;
class Dimension;
class ParagraphList;
class InsetCaption;
class InsetTabular;
class TocBuilder;

/**
 A text inset is like a TeX box to write full text
 (including styles and other insets) in a given space.
 */
class InsetText : public Inset {
public:
	enum UsePlain {
		DefaultLayout,
		PlainLayout
	};
	/// \param buffer
	/// \param useplain whether to use the plain layout
	/// This is needed because we cannot call the virtual function
	/// usePlainLayout() from within the constructor.
	explicit InsetText(Buffer * buffer, UsePlain type = DefaultLayout);
	///
	InsetText(InsetText const &);
	///
	void setBuffer(Buffer &) override;

	///
	Dimension const dimensionHelper(BufferView const &) const;

	/// empty inset to empty par
	void clear();
	///
	void read(Lexer & lex) override;
	///
	void write(std::ostream & os) const override;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const override;
	///
	void draw(PainterInfo & pi, int x, int y) const override;
	/// Drawing background is handled in draw
	void drawBackground(PainterInfo &, int, int) const override {}
	///
	bool editable() const override { return true; }
	///
	bool canTrackChanges() const override { return true; }
	/// Rely on RowPainter to draw the cue of inline insets.
	bool canPaintChange(BufferView const &) const override { return allowMultiPar(); }
	///
	InsetText * asInsetText() override { return this; }
	///
	InsetText const * asInsetText() const override { return this; }
	///
	Text & text() { return text_; }
	Text const & text() const { return text_; }
	///
	void latex(otexstream &, OutputParams const &) const override;
	///
	int plaintext(odocstringstream & ods, OutputParams const & op,
	              size_t max_length = INT_MAX) const override;
	///
	docstring xhtml(XMLStream &, OutputParams const &) const override;
	///
	enum XHTMLOptions {
		JustText = 0,
		WriteOuterTag = 1,
		WriteLabel = 2,
		WriteInnerTag = 4,
		WriteEverything = 7
	};
	///
	docstring insetAsXHTML(XMLStream &, OutputParams const &,
	                       XHTMLOptions) const;
	///
	void docbook(XMLStream &, OutputParams const &, XHTMLOptions opts) const;
	///
	void docbook(XMLStream &, OutputParams const &) const override;
	///
	void validate(LaTeXFeatures & features) const override;

	/// return the argument(s) only
	void getArgs(otexstream & os, OutputParams const &, bool const post = false) const;

	/// return x,y of given position relative to the inset's baseline
	void cursorPos(BufferView const & bv, CursorSlice const & sl,
		bool boundary, int & x, int & y) const override;
	///
	InsetCode lyxCode() const override { return TEXT_CODE; }
	///
	void setText(docstring const &, Font const &, bool trackChanges);
	///
	void setDrawFrame(bool);
	///
	ColorCode frameColor() const;
	///
	void setFrameColor(ColorCode);
	///
	Text * getText(int i) const override {
		return (i == 0) ? const_cast<Text*>(&text_) : 0;
	}
	///
	bool getStatus(Cursor & cur, FuncRequest const & cmd, FuncStatus &) const override;

	///
	void fixParagraphsFont();

	/// does the inset contain changes ?
	bool isChanged() const override { return is_changed_; }
	/// this is const because value is mutable
	void isChanged(bool ic) const { is_changed_ = ic; }
	/// set the change for the entire inset
	void setChange(Change const & change) override;
	/// accept the changes within the inset
	void acceptChanges() override;
	/// reject the changes within the inset
	void rejectChanges() override;

	/// append text onto the existing text
	void appendParagraphs(ParagraphList &);

	///
	void addPreview(DocIterator const &, graphics::PreviewLoader &) const override;

	///
	void edit(Cursor & cur, bool front, EntryDirection entry_from) override;
	///
	Inset * editXY(Cursor & cur, int x, int y) override;

	/// number of cells in this inset
	size_t nargs() const override { return 1; }
	///
	ParagraphList & paragraphs();
	///
	ParagraphList const & paragraphs() const;
	///
	bool insetAllowed(InsetCode) const override;
	///
	bool allowSpellCheck() const override { return getLayout().spellcheck() && !getLayout().isPassThru(); }
	///
	virtual bool isMacroScope() const { return false; }
	///
	bool allowMultiPar() const override { return getLayout().isMultiPar(); }
	///
	bool isInTitle() const override { return intitle_context_; }
	///
	/// should paragraphs be forced to use the empty layout?
	bool forcePlainLayout(idx_type = 0) const override
		{ return getLayout().forcePlainLayout(); }
	/// should the user be allowed to customize alignment, etc.?
	bool allowParagraphCustomization(idx_type = 0) const override
		{ return getLayout().allowParagraphCustomization(); }
	/// should paragraphs be forced to use a local font language switch?
	bool forceLocalFontSwitch() const override
		{ return getLayout().forceLocalFontSwitch(); }

	/// Update the counters of this inset and of its contents
	void updateBuffer(ParIterator const &, UpdateType, bool const deleted = false) override;
	///
	void setMacrocontextPositionRecursive(DocIterator const & pos);
	///
	void toString(odocstream &) const override;
	///
	void forOutliner(docstring &, size_t const, bool const) const override;
	///
	void addToToc(DocIterator const & di, bool output_active,
				  UpdateType utype, TocBackend & backend) const override;
	///
	Inset * clone() const override { return new InsetText(*this); }
	///
	bool notifyCursorLeaves(Cursor const & old, Cursor & cur) override;

	///
	bool completionSupported(Cursor const &) const override;
	///
	bool inlineCompletionSupported(Cursor const & cur) const override;
	///
	bool automaticInlineCompletion() const override;
	///
	bool automaticPopupCompletion() const override;
	///
	bool showCompletionCursor() const override;
	///
	CompletionList const * createCompletionList(Cursor const & cur) const override;
	///
	docstring completionPrefix(Cursor const & cur) const override;
	///
	bool insertCompletion(Cursor & cur, docstring const & s, bool finished) override;
	///
	void completionPosAndDim(Cursor const &, int & x, int & y, Dimension & dim) const override;
	/// returns the text to be used as tooltip
	/// \param prefix: a string that will precede the tooltip,
	/// e.g., "Index: ".
	/// \param len: length of the resulting string
	/// NOTE This routine is kind of slow. It's fine to use it within the
	/// GUI, but definitely do not try to use it in updateBuffer or anything
	/// of that sort. (Note: unnecessary internal copies have been removed
	/// since the previous note. The efficiency would have to be assessed
	/// again by profiling.)
	docstring toolTipText(docstring prefix = empty_docstring(),
	                      size_t len = 400) const;

	///
	std::string contextMenu(BufferView const &, int, int) const override;
	///
	std::string contextMenuName() const override;
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd) override;

	///
	bool confirmDeletion() const override { return !text().empty(); }

	///
	bool needsCProtection(bool const maintext = false,
			      bool const fragile = false) const override;
	///
	bool hasCProtectContent(bool fragile = false) const;

protected:
	///
	void iterateForToc(DocIterator const & cdit, bool output_active,
					   UpdateType utype, TocBackend & backend) const;
private:
	/// Open the toc item for paragraph pit. Returns the paragraph index where
	/// it should end.
	pit_type openAddToTocForParagraph(pit_type pit,
	                                  DocIterator const & dit,
	                                  bool output_active,
	                                  TocBackend & backend) const;
	/// Close a toc item opened in start and closed in end
	void closeAddToTocForParagraph(pit_type start, pit_type end,
	                               TocBackend & backend) const;
	///
	bool drawFrame_;
	/// true if the inset contains change
	mutable bool is_changed_;
	///
	bool intitle_context_;
	///
	ColorCode frame_color_;
	///
	Text text_;
};


InsetText::XHTMLOptions operator|(InsetText::XHTMLOptions a1, InsetText::XHTMLOptions a2);

} // namespace lyx

#endif
