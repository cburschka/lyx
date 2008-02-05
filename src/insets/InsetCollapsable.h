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
#include "InsetLayout.h"
#include "InsetText.h"

#include "Box.h"
#include "TextClass.h"
#include "TextClassPtr.h"

#include <string>

namespace lyx {

class CursorSlice;
class FontInfo;
class InsetLayout;
class Paragraph;
class Text;

namespace frontend { class Painter; }

/** A collapsable text inset

*/
class InsetCollapsable : public InsetText {
public:
	///
	InsetCollapsable(
		BufferParams const &,
		CollapseStatus status = Inset::Open,
		TextClassPtr tc = TextClassPtr((TextClass *)0)
		);
	///
	InsetCollapsable(InsetCollapsable const & rhs);
	
	InsetCollapsable * asInsetCollapsable() { return this; }
	InsetCollapsable const * asInsetCollapsable() const { return this; }
	docstring toolTip(BufferView const & bv, int x, int y) const;
	docstring name() const { return from_ascii("Collapsable"); }
	InsetLayout const & getLayout(BufferParams const &) const
	{ return *layout_; } 
	InsetLayout const & getLayout() const
	{ return *layout_; } 
	///
	void setLayout(BufferParams const &);
	/// (Re-)set the character style parameters from \p tc according
	/// to name()
	void setLayout(TextClassPtr tc);
	///
	void read(Buffer const &, Lexer &);
	///
	void write(Buffer const &, std::ostream &) const;
	///
	void metrics(MetricsInfo &, Dimension &) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;

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
	bool isMacroScope(Buffer const & buf) const;
	///
	void setLabel(docstring const & l);
	///
	virtual void setButtonLabel() {}
	///
	bool isOpen() const { return geometry() != ButtonOnly; }
	///
	CollapseStatus status() const;
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
	 *   x) toggled by autoOpen_
	 */

	///
	enum Decoration {
		Classic,
		Minimalistic,
		Conglomerate
	};
	/// Default looks
	virtual Decoration decoration() const;
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
	/// (status_), autoOpen_ and openinlined_, and of
	/// course decoration().
	Geometry geometry() const;
	///
	bool allowSpellCheck() const { return true; }
	///
	bool allowMultiPar() const;
	///
	bool getStatus(Cursor &, FuncRequest const &, FuncStatus &) const;
	///
	void setStatus(Cursor & cur, CollapseStatus st);
	///
	bool setMouseHover(bool mouse_hover);
	///
	virtual ColorCode backgroundColor() const {return layout_->bgcolor; }

	int latex(Buffer const &, odocstream &,
		  OutputParams const &) const;
	///
	void validate(LaTeXFeatures &) const;
	///
	virtual InsetCode lyxCode() const { return COLLAPSABLE_CODE; }

	/// Allow multiple blanks
	virtual bool isFreeSpacing() const { return layout_->freespacing; }
	/// Don't eliminate empty paragraphs
	virtual bool allowEmpty() const { return layout_->keepempty; }
	/// Force inset into LTR environment if surroundings are RTL?
	virtual bool forceLTR() const { return layout_->forceltr; }

protected:
	///
	virtual void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	void edit(Cursor & cur, bool left);
	///
	Inset * editXY(Cursor & cur, int x, int y);
	///
	docstring floatName(std::string const & type, BufferParams const &) const;
	///
	virtual void resetParagraphsFont();

private:
	/// text class to keep the InsetLayout above in memory
	TextClassPtr textClass_;
	/// cache for the layout_. Make sure it is in sync with the text class!
	InsetLayout const * layout_;
	///
	Dimension dimensionCollapsed() const;
	///
	docstring labelstring_;
	///
	mutable Box button_dim;
	///
	mutable CollapseStatus status_;
	/// a substatus of the Open status, determined automatically in metrics
	mutable bool openinlined_;
	/// the inset will automatically open when the cursor is inside
	mutable bool autoOpen_;
	/// changes color when mouse enters/leaves this inset
	bool mouse_hover_;
};

} // namespace lyx

#endif
