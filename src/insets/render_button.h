// -*- C++ -*-
/**
 * \file render_button.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef RENDER_BUTTON_H
#define RENDER_BUTTON_H

#include "render_base.h"
#include "box.h"
#include <string>


class RenderButton : public RenderBase
{
public:
	RenderButton();

	RenderBase * clone() const;

	/// compute the size of the object returned in dim
	virtual void metrics(MetricsInfo & mi, Dimension & dim) const;
	/// draw inset and update (xo, yo)-cache
	virtual void draw(PainterInfo & pi, int x, int y) const;

	/// Provide the text for the button
	void update(std::string const &, bool editable);

	/// The "sensitive area" box, i.e., the button area
	Box box() const { return button_box_; }
	///
	void setBox(Box b) { button_box_ = b; }

	/// equivalent to dynamic_cast
	virtual RenderButton * asButton() { return this; }

private:
	/// The stored data.
	std::string text_;
	bool editable_;
	Box button_box_;
};

#endif // NOT RENDER_BUTTON_H
