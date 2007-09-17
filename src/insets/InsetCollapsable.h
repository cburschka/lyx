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
#include "TextClass.h"

#include "Box.h"
#include "Font.h"

#include <string>

namespace lyx {

class Text;
class Paragraph;
class CursorSlice;
class InsetLayout;

namespace frontend { class Painter; }

/** A collapsable text inset

*/
class InsetCollapsable : public InsetText {
public:
	///
	static int const TEXT_TO_TOP_OFFSET = 2;
	///
	static int const TEXT_TO_BOTTOM_OFFSET = 2;
	///
	InsetCollapsable(BufferParams const &, CollapseStatus status = Inset::Open);
	///
	InsetCollapsable(InsetCollapsable const & rhs);
	///
	docstring name() const { return from_ascii("Collapsable"); }
	///
	void setLayout(BufferParams const &);
	///
	void read(Buffer const &, Lexer &);
	///
	void write(Buffer const &, std::ostream &) const;
	///
	bool metrics(MetricsInfo &, Dimension &) const;
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
	void setLabel(docstring const & l);
	///
	virtual void setButtonLabel() {}
	///
	void setLabelFont(Font const & f);
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
	 *   Minimalistic | ButtonOnly          NoButton
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
	bool getStatus(Cursor &, FuncRequest const &, FuncStatus &) const;
	///
	void setStatus(Cursor & cur, CollapseStatus st);
	///
	bool setMouseHover(bool mouse_hover);
	///
	virtual Color_color backgroundColor() const {return layout_.bgcolor; }

	int latex(Buffer const &, odocstream &,
		  OutputParams const &) const;
	///
	void validate(LaTeXFeatures &) const;

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
	docstring floatName(std::string const & type, BufferParams const &) const;

protected:
	///
	mutable Box button_dim;
	///
	mutable int topx;
	///
	mutable int topbaseline;
	///
	mutable InsetLayout layout_;
	///
	CollapseStatus internalStatus() const { return status_; }
private:
	///
	mutable CollapseStatus status_;
	/// a substatus of the Open status, determined automatically in metrics
	mutable bool openinlined_;
	/// the inset will automatically open when the cursor is inside
	mutable bool autoOpen_;
	/// changes color when mouse enters/leaves this inset
	bool mouse_hover_;
};

// A helper function that pushes the cursor out of the inset.
void leaveInset(Cursor & cur, Inset const & in);

} // namespace lyx

#endif
