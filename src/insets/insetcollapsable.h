// -*- C++ -*-
/**
 * \file insetcollapsable.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author Jürgen Vigna
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSETCOLLAPSABLE_H
#define INSETCOLLAPSABLE_H

#include "inset.h"
#include "insettext.h"

#include "box.h"
#include "lyxfont.h"

class Painter;
class LyXText;
class Paragraph;
class LyXCursor;

/** A collapsable text inset

*/
class InsetCollapsable : public UpdatableInset {
public:
	///
	static int const TEXT_TO_TOP_OFFSET = 2;
	///
	static int const TEXT_TO_BOTTOM_OFFSET = 2;
	/// inset is initially collapsed if bool = true
	InsetCollapsable(BufferParams const &, bool collapsed = false);
	///
	InsetCollapsable(InsetCollapsable const & in);
	///
	void read(Buffer const &, LyXLex &);
	///
	void write(Buffer const &, std::ostream &) const;
	///
	void metrics(MetricsInfo &, Dimension &) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	/// draw, either inlined (no button) or collapsed/open
	void draw(PainterInfo & pi, int x, int y, bool inlined) const;
	///
	bool hitButton(FuncRequest const &) const;
	///
	std::string const getNewLabel(std::string const & l) const;
	///
	EDITABLE editable() const;
	/// can we go further down on mouse click?
	bool descendable() const { return isOpen(); }
	///
	bool insertInset(BufferView *, InsetOld * inset);
	///
	bool insetAllowed(InsetOld::Code code) const;
	///
	bool isTextInset() const { return true; }
	///
	int latex(Buffer const &, std::ostream &,
		  OutputParams const &) const;
	///
	int plaintext(Buffer const &, std::ostream &,
		  OutputParams const &) const;
	///
	int linuxdoc(Buffer const &, std::ostream &,
		     OutputParams const &) const;
	///
	int docbook(Buffer const &, std::ostream &,
		    OutputParams const & runparams) const;
	///
	void validate(LaTeXFeatures & features) const;
	/// get the screen x,y of the cursor
	void getCursorPos(int & x, int & y) const;
	///
	void setLabel(std::string const & l) const;
	///
	void setLabelFont(LyXFont & f);
#if 0
	///
	void setAutoCollapse(bool f);
#endif
	/// Appends \c list with all labels found within this inset.
	void getLabelList(Buffer const &, std::vector<std::string> & list) const;
	///
	int scroll(bool recursive=true) const;
	///
	void scroll(BufferView *bv, float sx) const;
	///
	void scroll(BufferView *bv, int offset) const;
	///
	ParagraphList * getParagraphs(int) const;
	///
	int numParagraphs() const;
	///
	LyXText * getText(int) const;
	///
	bool display() const { return isOpen(); }
	///
	bool isOpen() const;
	///
	void open();
	///
	void close() const;
	///
	void markErased();
	///
	void addPreview(lyx::graphics::PreviewLoader &) const;
	///
	void setBackgroundColor(LColor_color);

protected:
	///
	virtual
	DispatchResult
	priv_dispatch(FuncRequest const &, idx_type &, pos_type &);
	///
	void dimension_collapsed(Dimension &) const;
	///
	int height_collapsed() const;
	///
	void draw_collapsed(PainterInfo & pi, int x, int y) const;
	///
	int getMaxTextWidth(Painter & pain, UpdatableInset const *) const;
	/// Should be non-const...
	void setCollapsed(bool) const;
	///
	Box const & buttonDim() const;
	///
	void edit(BufferView *, bool);
	///
	void edit(BufferView *, int, int);

private:
	///
	DispatchResult lfunMouseRelease(FuncRequest const &);
	///
	FuncRequest adjustCommand(FuncRequest const &);

public:
	///
	mutable InsetText inset;
private:
	///
	mutable bool collapsed_;
	///
	LyXFont labelfont_;
	///
	mutable Box button_dim;
	///
	mutable int topx;
	///
	mutable int topbaseline;
	///
	mutable std::string label;
#if 0
	///
	bool autocollapse;
#endif
};

#endif
