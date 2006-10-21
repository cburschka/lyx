// -*- C++ -*-
/**
 * \file nullpainter.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alfredo Braunstein
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef NULLPAINTER_H
#define NULLPAINTER_H

#include "LColor.h"
#include "Painter.h"


namespace lyx {
namespace frontend {

class NullPainter : public Painter {
public:
	///
	NullPainter() {}

	virtual ~NullPainter() {}

	/// begin painting
	void start() {}
	/// end painting
	void end() {}

	///
	int paperWidth() const { return 0; }
	///
	int paperHeight() const;

	///
	void line(int, int, int, int, LColor_color,
		       line_style = line_solid, line_width = line_thin) {}
	///
	void lines(int const *, int const *, int, LColor_color,
			line_style = line_solid, line_width = line_thin) {}
	///
	void rectangle(int, int, int, int,	LColor_color,
		line_style = line_solid, line_width = line_thin) {}
	///
	void fillRectangle(int, int, int, int, LColor_color) {}
	///
	void arc(int, int,	unsigned int, unsigned int,
		int, int, LColor_color) {}
	///
	void point(int, int, LColor_color) {}
	///
	void button(int, int, int, int) {}
	///
	void image(int, int, int, int, graphics::Image const &) {}
	///
	int text(int, int, docstring const &, LyXFont const &) { return 0; }
// 	///
// 	int text(int, int, char const *, size_t, LyXFont const &) { return 0; }
	///
	int text(int, int, char_type const *, size_t, LyXFont const &) { return 0; }
	///
	int text(int, int, char_type, LyXFont const &) { return 0; }
	///
	void rectText(int, int, docstring const &,
		LyXFont const &, LColor_color, LColor_color) {}
	///
	void buttonText(int, int, docstring const &, LyXFont const &) {}
	///
	void underline(LyXFont const &, int, int, int) {}
	///
	void buttonFrame(int, int, int, int) {}
};

} // namespace frontend
} // namespace lyx

#endif // NULLPAINTER_H
