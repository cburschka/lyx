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
#include "TextClass.h"

namespace lyx {

class CursorSlice;
class InsetLayout;

namespace frontend { class Painter; }

/** A collapsable text inset

*/
class InsetCollapsable : public InsetText {
public:
	///	By default, InsetCollapsable uses the plain layout. If you 
	/// want to override this in a subclass, you'll need to call 
	/// Paragraph::setDefaultLayout() in its constructor. See
	/// InsetBranch for an example.
	InsetCollapsable(Buffer const &);
	///
	InsetCollapsable(InsetCollapsable const & rhs);
	///
	InsetCollapsable * asInsetCollapsable() { return this; }
	///
	InsetCollapsable const * asInsetCollapsable() const { return this; }
	///
	docstring toolTip(BufferView const & bv, int x, int y) const;
	///
	docstring name() const { return from_ascii("Collapsable"); }
	///
	InsetLayout const & getLayout(BufferParams const &) const { return *layout_; }
	///
	InsetLayout const & getLayout() const { return *layout_; } 
	///
	void setLayout(BufferParams const &);
	/// (Re-)set the character style parameters from \p tc according
	/// to name()
	void setLayout(DocumentClass const * const tc);
	///
	virtual bool usePlainLayout() { return true; }
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
	EDITABLE editable() const;
	/// can we go further down on mouse click?
	bool descendable() const;
	///
	void setLabel(docstring const & l);
	///
	virtual void setButtonLabel() {}
	///
	virtual docstring const buttonLabel(BufferView const &) const
		{ return labelstring_; }
	///
	bool isOpen(BufferView const & bv) const 
		{ return geometry(bv) != ButtonOnly; }
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
	/// Allow spellchecking, except for insets with latex_language
	bool allowSpellCheck() const { return !forceLTR(); }
	///
	bool allowMultiPar() const;
	///
	bool getStatus(Cursor &, FuncRequest const &, FuncStatus &) const;
	///
	void setStatus(Cursor & cur, CollapseStatus st);
	///
	bool setMouseHover(bool mouse_hover);
	///
	ColorCode backgroundColor() const { return layout_->bgcolor(); }
	///
	int latex(odocstream &, OutputParams const &) const;
	///
	void validate(LaTeXFeatures &) const;
	///
	InsetCode lyxCode() const { return COLLAPSABLE_CODE; }

	/// Allow multiple blanks
	virtual bool isFreeSpacing() const { return layout_->isFreeSpacing(); }
	/// Don't eliminate empty paragraphs
	virtual bool allowEmpty() const { return layout_->isKeepEmpty(); }
	/// Force inset into LTR environment if surroundings are RTL?
	virtual bool forceLTR() const { return layout_->isForceLtr(); }
	///
	virtual bool usePlainLayout() const { return true; }
	/// Is this inset's layout defined in the document's textclass?
	/// May be wrong after textclass change or paste from another document
	bool undefined() const;
	///
	virtual docstring contextMenu(BufferView const & bv, int x, int y) const;
protected:
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	void edit(Cursor & cur, bool front, 
		EntryDirection entry_from = ENTRY_DIRECTION_IGNORE);
	///
	Inset * editXY(Cursor & cur, int x, int y);
	///
	docstring floatName(std::string const & type, BufferParams const &) const;
	///
	virtual void resetParagraphsFont();
	///
	mutable CollapseStatus status_;
private:
	/// cache for the layout_. Make sure it is in sync with the document class!
	InsetLayout const * layout_;
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
	bool mouse_hover_;
};

} // namespace lyx

#endif
