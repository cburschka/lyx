// -*- C++ -*-
/**
 * \file InsetCollapsible.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author Jürgen Vigna
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSETCOLLAPSIBLE_H
#define INSETCOLLAPSIBLE_H

#include "InsetText.h"

#include "Box.h"

#include <map>

namespace lyx {

class CursorSlice;
class InsetLayout;

namespace support { class TempFile; }

namespace frontend { class Painter; }

/** A collapsible text inset

*/
class InsetCollapsible : public InsetText {
public:
	///
	InsetCollapsible(Buffer *, InsetText::UsePlain = InsetText::PlainLayout);
	///
	InsetCollapsible(InsetCollapsible const & rhs);
	///
	InsetCollapsible & operator=(InsetCollapsible const &);
	///
	virtual ~InsetCollapsible();
	///
	InsetCollapsible * asInsetCollapsible() override { return this; }
	///
	InsetCollapsible const * asInsetCollapsible() const override { return this; }
	///
	docstring toolTip(BufferView const & bv, int x, int y) const override;
	///
	docstring layoutName() const override { return from_ascii("Collapsible"); }
	///
	void read(Lexer &) override;
	///
	void write(std::ostream &) const override;

	///
	int topOffset(BufferView const * bv) const override;
	///
	int bottomOffset(BufferView const * bv) const override;

	///
	void metrics(MetricsInfo &, Dimension &) const override;
	///
	void draw(PainterInfo & pi, int x, int y) const override;
	///
	void drawBackground(PainterInfo &, int, int) const override {}

	/// return x,y of given position relative to the inset's baseline
	void cursorPos(BufferView const & bv, CursorSlice const & sl,
		bool boundary, int & x, int & y) const override;
	///
	docstring const getNewLabel(docstring const & l) const;
	///
	bool editable() const override;
	///
	bool hasSettings() const override { return true; }
	/// Returns true if coordinates are over the inset's button.
	/// Always returns false when the inset does not have a
	/// button.
	bool clickable(BufferView const & bv, int x, int y) const override;
	/// can we go further down on mouse click?
	bool descendable(BufferView const & bv) const override;
	///
	void setLabel(docstring const & l);
	///
	docstring getLabel() const;
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
	///
	bool canPaintChange(BufferView const & bv) const override;
	///
	bool getStatus(Cursor &, FuncRequest const &, FuncStatus &) const override;
	///
	bool setMouseHover(BufferView const * bv, bool mouse_hover) const override;
	///
	ColorCode backgroundColor(PainterInfo const &) const override
		{ return getLayout().bgcolor(); }
	///
	ColorCode labelColor() const override { return getLayout().labelfont().color(); }
	///
	InsetCode lyxCode() const override { return COLLAPSIBLE_CODE; }

	///
	bool usePlainLayout() const override { return true; }
	///
	std::string contextMenu(BufferView const & bv, int x, int y) const override;
	///
	std::string contextMenuName() const override;
	///
	void addToToc(DocIterator const & dit, bool output_active,
	              UpdateType utype, TocBackend & backend) const override;

protected:
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd) override;
	///
	void edit(Cursor & cur, bool front,
		EntryDirection entry_from = ENTRY_DIRECTION_IGNORE) override;
	///
	Inset * editXY(Cursor & cur, int x, int y) override;
	///
	mutable CollapseStatus status_;
        ///
        unique_ptr<support::TempFile> tempfile_;
private:
	///
	Dimension dimensionCollapsed(BufferView const & bv) const;
	///
	docstring labelstring_;

	// These variables depend of the view in which the inset is displayed
	struct View
	{
		/// The dimension of the inset button
		Box button_dim_;
		/// a substatus of the Open status, determined automatically in metrics
		bool openinlined_;
		/// the inset will automatically open when the cursor is inside. This is
		/// dependent on the bufferview, compare with InsetMathMacro::editing_.
		bool auto_open_;
		/// changes color when mouse enters/leaves this inset
		bool mouse_hover_;
	};

	///
	mutable std::map<BufferView const *, View> view_;
};

} // namespace lyx

#endif
