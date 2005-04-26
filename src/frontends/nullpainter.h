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
	void fillPolygon(int const *, int const *, int, LColor_color) {}
	///
	void arc(int, int,	unsigned int, unsigned int,
		int, int, LColor_color) {}
	///
	void point(int, int, LColor_color) {}
	///
	void button(int, int, int, int) {}
	///
	void image(int, int, int, int, lyx::graphics::Image const &) {}
	///
	void text(int, int, std::string const &, LyXFont const &) {}
	///
	void text(int, int, char const *, size_t, LyXFont const &) {}
	///
	void text(int, int, char, LyXFont const &) {}
	///
	void rectText(int, int, std::string const &,
		LyXFont const &, LColor_color, LColor_color) {}
	///
	void buttonText(int, int, std::string const &, LyXFont const &) {}
	///
	void underline(LyXFont const &, int, int, int) {}
	///
	void buttonFrame(int, int, int, int) {}
};

#endif // NULLPAINTER_H
