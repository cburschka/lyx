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
class CursorSlice;

/** A collapsable text inset

*/
class InsetCollapsable : public UpdatableInset {
public:
	///
	static int const TEXT_TO_TOP_OFFSET = 2;
	///
	static int const TEXT_TO_BOTTOM_OFFSET = 2;
	///
	enum CollapseStatus {
		Open,
		Collapsed,
		Inlined
	};
	///
	InsetCollapsable(BufferParams const &, CollapseStatus status = Open);
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
	///
	bool hitButton(FuncRequest const &) const;
	///
	std::string const getNewLabel(std::string const & l) const;
	///
	EDITABLE editable() const;
	/// can we go further down on mouse click?
	bool descendable() const;
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
	void getCursorPos(int cell, int & x, int & y) const;
	///
	void setLabel(std::string const & l);
	///
	virtual void setButtonLabel() {};
 	///
	void setLabelFont(LyXFont & f);
	/// Appends \c list with all labels found within this inset.
	void getLabelList(Buffer const &, std::vector<std::string> & list) const;
	///
	int scroll(bool recursive=true) const;
	///
	void scroll(BufferView & bv, float sx) const;
	///
	void scroll(BufferView & bv, int offset) const;
	///
	int numParagraphs() const;
	///
	LyXText * getText(int) const;
	///
	bool isOpen() const { return status_ == Open || status_ == Inlined; }
	///
	bool inlined() const { return status_ == Inlined; }
	///
	CollapseStatus status() const { return status_; }
	///
	void open();
	///
	void close();
	///
	void markErased();
	///
	void addPreview(lyx::graphics::PreviewLoader &) const;
	///
	void setBackgroundColor(LColor_color);
	///
	void setStatus(CollapseStatus st);

protected:
	///
	virtual
	DispatchResult
	priv_dispatch(BufferView & bv, FuncRequest const & cmd);
	///
	void dimension_collapsed(Dimension &) const;
	///
	int height_collapsed() const;
	///
	void draw_collapsed(PainterInfo & pi, int x, int y) const;
	///
	int getMaxTextWidth(Painter & pain, UpdatableInset const *) const;
	///
	Box const & buttonDim() const;
	///
	void edit(BufferView *, bool);
	///
	void edit(BufferView *, int, int);

private:
	///
	DispatchResult lfunMouseRelease(BufferView & bv, FuncRequest const &);
	///
	FuncRequest adjustCommand(FuncRequest const &);

public:
	///
	mutable InsetText inset;
private:
	///
	mutable CollapseStatus status_;
protected:
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
};

#endif
