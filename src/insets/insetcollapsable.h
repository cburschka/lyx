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
class InsetCollapsable : public InsetText {
public:
	///
	static int const TEXT_TO_TOP_OFFSET = 2;
	///
	static int const TEXT_TO_BOTTOM_OFFSET = 2;
	///
	enum CollapseStatus {
		Collapsed,
		Inlined,
		Open
	};
	///
	InsetCollapsable(BufferParams const &, CollapseStatus status = Open);
	///
	void read(Buffer const &, LyXLex &);
	///
	void write(Buffer const &, std::ostream &) const;
	///
	void metrics(MetricsInfo &, Dimension &) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void drawSelection(PainterInfo & pi, int x, int y) const;
	/// return x,y of given position relative to the inset's baseline
	void getCursorPos(CursorSlice const & sl, int & x, int & y) const;
	///
	bool hitButton(FuncRequest const &) const;
	///
	std::string const getNewLabel(std::string const & l) const;
	///
	EDITABLE editable() const;
	/// can we go further down on mouse click?
	bool descendable() const;
	///
	bool isTextInset() const { return true; }
	///
	void setLabel(std::string const & l);
	///
	virtual void setButtonLabel() {}
	///
	void setLabelFont(LyXFont & f);
	///
	int scroll(bool recursive = true) const;
	///
	void scroll(BufferView & bv, double sx) const;
	///
	void scroll(BufferView & bv, int offset) const;
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
	bool allowSpellCheck() const { return true; }
	///
	bool getStatus(LCursor &, FuncRequest const &, FuncStatus &) const;

protected:
	///
	void setStatus(CollapseStatus st);
	///
	virtual void doDispatch(LCursor & cur, FuncRequest & cmd);
	///
	Dimension dimensionCollapsed() const;
	///
	int getMaxTextWidth(Painter & pain, UpdatableInset const *) const;
	///
	Box const & buttonDim() const;
	///
	void edit(LCursor & cur, bool left);
	///
	InsetBase * editXY(LCursor & cur, int x, int y) const;

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
private:
	///
	mutable CollapseStatus status_;
	/// a substatus of the Open status, determined automatically in metrics
	mutable bool openinlined_;
	///
	mutable Dimension textdim_;
};

// A helper function that pushes the cursor out of the inset.
void leaveInset(LCursor & cur, InsetBase const & in);

#endif
