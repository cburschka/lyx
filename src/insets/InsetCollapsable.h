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

#include "InsetText.h"

#include "Box.h"

#include <map>

namespace lyx {

class CursorSlice;
class InsetLayout;

namespace frontend { class Painter; }

/** A collapsable text inset

*/
class InsetCollapsable : public InsetText {
public:
	///
	InsetCollapsable(Buffer *, InsetText::UsePlain = InsetText::PlainLayout);
	///
	InsetCollapsable(InsetCollapsable const & rhs);
	///
	virtual ~InsetCollapsable();
	///
	InsetCollapsable * asInsetCollapsable() { return this; }
	///
	InsetCollapsable const * asInsetCollapsable() const { return this; }
	///
	docstring toolTip(BufferView const & bv, int x, int y) const;
	///
	docstring layoutName() const { return from_ascii("Collapsable"); }
	///
	void read(Lexer &);
	///
	void write(std::ostream &) const;
	///
	void metrics(MetricsInfo &, Dimension &) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;

	/// return x,y of given position relative to the inset's baseline
	void cursorPos(BufferView const & bv, CursorSlice const & sl,
		bool boundary, int & x, int & y) const;
	/// Returns true if (mouse) action is over the inset's button.
	/// Always returns false when the inset does not have a
	/// button.
	bool hitButton(FuncRequest const &) const;
	///
	docstring const getNewLabel(docstring const & l) const;
	///
	bool editable() const;
	///
	bool hasSettings() const { return true; }
	///
	bool clickable(BufferView const &, int x, int y) const;
	/// can we go further down on mouse click?
	bool descendable(BufferView const & bv) const;
	///
	void setLabel(docstring const & l);
	///
	virtual void setButtonLabel() {}
	///
	virtual docstring const buttonLabel(BufferView const &) const;
	///
	bool isOpen(BufferView const & bv) const
		{ return geometry(bv) != ButtonOnly; }
	///
	enum CollapseStatus {
		Collapsed,
		Open
	};
	///
	virtual void setStatus(Cursor & cur, CollapseStatus st);
	///
	CollapseStatus status(BufferView const & bv) const;
	/** Of the old CollapseStatus we only keep the values
	 *  Open and Collapsed.
	 * We define a list of possible inset decoration
	 * styles, and a list of possible (concrete, visual)
	 * inset geometries. Relationships between them
	 * (geometries in body of table):
	 *
	 *               \       CollapseStatus:
	 *   Decoration:  \ Open                Collapsed
	 *   -------------+-------------------------------
	 *   Classic      | *) TopButton, <--x) ButtonOnly
	 *                | LeftButton
	 *   Minimalistic | NoButton            ButtonOnly
	 *   Conglomerate | SubLabel            Corners
	 *   ---------------------------------------------
	 *   *) toggled by openinlined_
	 *   x) toggled by auto_open_
	 */

	/// Default looks
	virtual InsetLayout::InsetDecoration decoration() const;
	/// Inset font
	virtual FontInfo getFont() const { return getLayout().font(); }
	/// Label font
	virtual FontInfo getLabelfont() const { return getLayout().labelfont(); }
	///
	enum Geometry {
		TopButton,
		ButtonOnly,
		NoButton,
		LeftButton,
		SubLabel,
		Corners
	};
	/// Returns the geometry based on CollapseStatus
	/// (status_), auto_open_[BufferView] and openinlined_,
	/// and of course decoration().
	Geometry geometry(BufferView const & bv) const;
	/// Returns the geometry disregarding auto_open_
	Geometry geometry() const;
	///
	bool getStatus(Cursor &, FuncRequest const &, FuncStatus &) const;
	///
	bool setMouseHover(BufferView const * bv, bool mouse_hover) const;
	///
	ColorCode backgroundColor(PainterInfo const &) const
		{ return getLayout().bgcolor(); }
	///
	ColorCode labelColor() const { return getLayout().labelfont().color(); }
	///
	InsetCode lyxCode() const { return COLLAPSABLE_CODE; }

	///
	virtual bool usePlainLayout() const { return true; }
	///
	std::string contextMenu(BufferView const & bv, int x, int y) const;
	///
	std::string contextMenuName() const;
protected:
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	void edit(Cursor & cur, bool front,
		EntryDirection entry_from = ENTRY_DIRECTION_IGNORE);
	///
	Inset * editXY(Cursor & cur, int x, int y);
	///
	mutable CollapseStatus status_;
private:
	///
	Dimension dimensionCollapsed(BufferView const & bv) const;
	///
	docstring labelstring_;
	///
	mutable Box button_dim;
	/// a substatus of the Open status, determined automatically in metrics
	mutable bool openinlined_;
	/// the inset will automatically open when the cursor is inside. This is
	/// dependent on the bufferview, compare with MathMacro::editing_.
	mutable std::map<BufferView const *, bool> auto_open_;
	/// changes color when mouse enters/leaves this inset
	mutable std::map<BufferView const *, bool> mouse_hover_;
};

} // namespace lyx

#endif
