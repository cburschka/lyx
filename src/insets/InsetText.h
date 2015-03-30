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
	bool editable() const { return true; }
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
	void latex(otexstream &, OutputParams const &) const;
	///
	int plaintext(odocstringstream & ods, OutputParams const & op,
	              size_t max_length = INT_MAX) const;
	///
	int docbook(odocstream &, OutputParams const &) const;
	///
	docstring xhtml(XHTMLStream &, OutputParams const &) const;
	///
	enum XHTMLOptions {
		JustText = 0,
		WriteOuterTag = 1,
		WriteLabel = 2,
		WriteInnerTag = 4,
		WriteEverything = 7
	};
	///
	docstring insetAsXHTML(XHTMLStream &, OutputParams const &, 
	                       XHTMLOptions) const;
	///
	void validate(LaTeXFeatures & features) const;
	
	/// return the argument(s) only
	void getArgs(otexstream & os, OutputParams const &, bool const post = false) const;

	/// return x,y of given position relative to the inset's baseline
	void cursorPos(BufferView const & bv, CursorSlice const & sl,
		bool boundary, int & x, int & y) const;
	///
	InsetCode lyxCode() const { return TEXT_CODE; }
	///
	void setText(docstring const &, Font const &, bool trackChanges);
	///
	void setDrawFrame(bool);
	///
	ColorCode frameColor() const;
	///
	void setFrameColor(ColorCode);
	///
	Text * getText(int i) const {
		return (i == 0) ? const_cast<Text*>(&text_) : 0;
	}
	///
	virtual bool getStatus(Cursor & cur, FuncRequest const & cmd, FuncStatus &) const;

	///
	void fixParagraphsFont();

	/// set the change for the entire inset
	void setChange(Change const & change);
	/// accept the changes within the inset
	void acceptChanges();
	/// reject the changes within the inset
	void rejectChanges();

	/// append text onto the existing text
	void appendParagraphs(ParagraphList &);

	///
	void addPreview(DocIterator const &, graphics::PreviewLoader &) const;

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
	bool insetAllowed(InsetCode) const;
	///
	bool allowSpellCheck() const { return getLayout().spellcheck() && !getLayout().isPassThru(); }
	///
	virtual bool isMacroScope() const { return false; }
	///
	virtual bool allowMultiPar() const { return getLayout().isMultiPar(); }
	///
	/// should paragraphs be forced to use the empty layout?
	virtual bool forcePlainLayout(idx_type = 0) const 
		{ return getLayout().forcePlainLayout(); }
	/// should the user be allowed to customize alignment, etc.?
	virtual bool allowParagraphCustomization(idx_type = 0) const 
		{ return getLayout().allowParagraphCustomization(); }

	/// Update the counters of this inset and of its contents
	virtual void updateBuffer(ParIterator const &, UpdateType);
	/// 
	void setMacrocontextPositionRecursive(DocIterator const & pos);
	///
	void toString(odocstream &) const;
	///
	void forOutliner(docstring &, size_t) const;
	///
	void addToToc(DocIterator const & di, bool output_active) const;
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
	/// returns the text to be used as tooltip
	/// \param prefix: a string that will preced the tooltip,
	/// e.g., "Index: ".
	/// \param numlines: the number of lines in the tooltip
	/// \param len: length of those lines
	docstring toolTipText(docstring prefix = empty_docstring(),
			size_t numlines = 5, size_t len = 80) const;

	///
	std::string contextMenu(BufferView const &, int, int) const;
	///
	std::string contextMenuName() const;
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
protected:
	///
	InsetCaption const * getCaptionInset() const;
	///
	docstring getCaptionText(OutputParams const &) const;
	///
	docstring getCaptionHTML(OutputParams const &) const;
	///
	void iterateForToc(DocIterator const & cdit, bool output_active) const;
private:
	///
	bool drawFrame_;
	///
	ColorCode frame_color_;
	///
	mutable Text text_;
};


InsetText::XHTMLOptions operator|(InsetText::XHTMLOptions a1, InsetText::XHTMLOptions a2);

} // namespace lyx

#endif
