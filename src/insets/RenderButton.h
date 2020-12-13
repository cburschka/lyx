// -*- C++ -*-
/**
 * \file RenderButton.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef RENDERBUTTON_H
#define RENDERBUTTON_H

#include "RenderBase.h"
#include "Box.h"
#include "support/docstring.h"


namespace lyx {


class RenderButton : public RenderBase
{
public:
	RenderButton();

	RenderBase * clone(Inset const *) const override;

	/// compute the size of the object returned in dim
	void metrics(MetricsInfo & mi, Dimension & dim) const override;
	/// draw inset and update (xo, yo)-cache
	void draw(PainterInfo & pi, int x, int y, bool const darkmode = false) const override;

	/// Provide the text for the button
	void update(docstring const &, bool editable,
		    bool inherit, bool broken = false);

	/// The "sensitive area" box, i.e., the button area
	Box box() const { return button_box_; }
	///
	void setBox(Box b) { button_box_ = b; }

	/// equivalent to dynamic_cast
	RenderButton * asButton() override { return this; }

private:
	/// The stored data.
	docstring text_;
	bool editable_;
	bool broken_;
	bool inherit_font_;
	Box button_box_;
};


} // namespace lyx

#endif // NOT RENDERBUTTON_H
