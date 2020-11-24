// -*- C++ -*-
/**
 * \file CaretGeometry.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CARETGEOMETRY_H
#define CARETGEOMETRY_H

#include "Dimension.h"

#include <list>
#include <vector>

namespace lyx {

namespace frontend {

/**
 * CaretGeometry - Data for painting the caret
 */
struct CaretGeometry {
	///
	int height() const { return bottom - top; }
	///
	int width() const { return right - left; }

	// A polygon
	typedef std::vector<Point> Shape;
	// several polygons
	typedef std::list<Shape> Shapes;
	// data for pointer
	Shapes shapes;
	// left-most abcissa
	int left = 0;
	// right-most abcissa
	int right = 0;
	// top value
	int top = 0;
	// bottom value
	int bottom = 0;
};

} // namespace frontend
} // namespace lyx

#endif // CARETGEOMETRY_H
