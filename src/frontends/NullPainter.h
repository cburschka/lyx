// -*- C++ -*-
/**
 * \file NullPainter.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author John Levon
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef NULLPAINTER_H
#define NULLPAINTER_H

#include "Painter.h"

namespace lyx {

namespace frontend {

/**
 * NullPainter - A painter instance that does nothing
 */
class NullPainter : public Painter {
public:
	NullPainter() : Painter(1, false) {}

	~NullPainter() {}

	/// draw a line from point to point
	void line(int, int, int, int, Color,
		line_style = line_solid, int = thin_line) override {}

	///
	void lines(int const *, int const *, int, Color,
		fill_style = fill_none, line_style = line_solid,
		int = thin_line) override {}

	///
	void path(int const *, int const *, int const *, int const *,
		int const *, int const *, int, Color,
		fill_style = fill_none, line_style = line_solid, int = thin_line) override {}

	/// draw a rectangle
	void rectangle(int, int, int, int, Color,
		line_style = line_solid, int = thin_line) override {}

	/// draw a filled rectangle
	void fillRectangle(int, int, int, int, Color) override {}

	/// draw an arc
	void arc(int, int, unsigned int, unsigned int, int, int, Color) override {}

	/// draw an ellipse
	void ellipse(double, double, double, double, Color,
		fill_style = fill_none, line_style = line_solid,
		int = thin_line) override {}

	/// draw a pixel
	void point(int, int, Color) override {}

	/// draw an image from the image cache
	void image(int, int, int, int, graphics::Image const &, bool) override {}

	/// draw a string
	void text(int, int, docstring const &, FontInfo const &) override {}

	/// draw a char
	void text(int, int, char_type, FontInfo const &) override {}

	/// draw a string
	void text(int, int, docstring const &, Font const &, double, double) override {}

	///
	void text(int, int, docstring const &, Font const &,
	          Color, size_type, size_type, double, double) override {}

	/// This painter does not paint
	bool isNull() const override { return true; }

 	/// draw the underbar, strikeout, xout, uuline and uwave font attributes
	void textDecoration(FontInfo const &, int, int, int) override {}

	/**
	 * Draw a string and enclose it inside a rectangle. If
	 * back color is specified, the background is cleared with
	 * the given color. If frame is specified, a thin frame is drawn
	 * around the text with the given color.
	 */
	void rectText(int, int, docstring const &,
	              FontInfo const &, Color, Color) override {}

	/// draw a string and enclose it inside a button frame
	void buttonText(int, int, docstring const &,
	                FontInfo const &, Color, Color, int) override {}

	/// draw a character of a preedit string for cjk support.
	int preeditText(int, int, char_type, FontInfo const &,
	                preedit_style) override { return 0; }

	/// start monochrome painting mode, i.e. map every color a shade of \c blend.
	void enterMonochromeMode(Color const &) override {}
	/// leave monochrome painting mode
	void leaveMonochromeMode() override {}
	/// draws a wavy line that can be used for underlining.
	void wavyHorizontalLine(FontInfo const &, int, int, int, ColorCode) override {}
};

} // namespace frontend
} // namespace lyx

#endif // NULLPAINTER_H
