// -*- C++ -*-
/**
 * \file InsetCollapsable.h
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

#include "Inset.h"
#include "InsetText.h"

#include "Box.h"
#include "Font.h"

#include <string>

namespace lyx {

class Text;
class Paragraph;
class CursorSlice;

namespace frontend { class Painter; }

/** A collapsable text inset

*/
class InsetCollapsable : public InsetText {
public:
	///
	InsetCollapsable(BufferParams const &, CollapseStatus status = Open);
	///
	InsetCollapsable(InsetCollapsable const & rhs);
	///
	docstring name() const { return from_ascii("Collapsable"); }
	///
	void read(Buffer const &, Lexer &);
	///
	void write(Buffer const &, std::ostream &) const;
	///
	bool metrics(MetricsInfo &, Dimension &) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void drawSelection(PainterInfo & pi, int x, int y) const;
	/// return x,y of given position relative to the inset's baseline
	void cursorPos(BufferView const & bv, CursorSlice const & sl,
		bool boundary, int & x, int & y) const;
	///
	bool hitButton(FuncRequest const &) const;
	///
	docstring const getNewLabel(docstring const & l) const;
	///
	EDITABLE editable() const;
	/// can we go further down on mouse click?
	bool descendable() const;
	///
	void setLabel(docstring const & l);
	///
	virtual void setButtonLabel() {}
	///
	void setLabelFont(Font & f);
	///
	bool isOpen() const { return status_ == Open || status_ == Inlined; }
	///
	bool inlined() const { return status_ == Inlined; }
	///
	CollapseStatus status() const;
	///
	bool allowSpellCheck() const { return true; }
	///
	bool getStatus(Cursor &, FuncRequest const &, FuncStatus &) const;
	///
	void setStatus(Cursor & cur, CollapseStatus st);
	///
	bool setMouseHover(bool mouse_hover);

protected:
	///
	virtual void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	Dimension dimensionCollapsed() const;
	///
	Box const & buttonDim() const;
	///
	void edit(Cursor & cur, bool left);
	///
	Inset * editXY(Cursor & cur, int x, int y);
	///
	void setInlined() { status_ = Inlined; }
	///
	docstring floatName(std::string const & type, BufferParams const &) const;

protected:
	///
	Font labelfont_;
	///
	mutable Box button_dim;
	///
	mutable int topx;
	///
	mutable int topbaseline;
	///
	mutable docstring label;
private:
	///
	mutable CollapseStatus status_;
	/// a substatus of the Open status, determined automatically in metrics
	mutable bool openinlined_;
	/// the inset will automatically open when the cursor is inside
	mutable bool autoOpen_;
	///
	mutable Dimension textdim_;
	/// changes color when mouse enters/leaves this inset
	bool mouse_hover_;
};

// A helper function that pushes the cursor out of the inset.
void leaveInset(Cursor & cur, Inset const & in);

} // namespace lyx

#endif
