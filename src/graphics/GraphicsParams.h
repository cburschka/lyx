// -*- C++ -*-
/**
 * \file GraphicsParams.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 *
 * Used internally by the lyx::graphics::Image.
 */

#ifndef GRAPHICSPARAMS_H
#define GRAPHICSPARAMS_H

#include "support/FileName.h"

#include <string>
#include <iosfwd>


namespace lyx {
namespace graphics {

/** Parse a string of the form "200pt 500pt 300mm 5in" into a
 *  usable bounding box.
 */
class BoundingBox {
public:
	///
	BoundingBox();
	///
	BoundingBox(std::string const &);

	/// 0 0 0 0 is empty!
	bool empty() const;

	unsigned int xl;
	unsigned int yb;
	unsigned int xr;
	unsigned int yt;
};

bool operator==(BoundingBox const &, BoundingBox const &);
bool operator!=(BoundingBox const &, BoundingBox const &);

std::ostream & operator<<(std::ostream &, BoundingBox const &);


class Params
{
public:
	Params();

	bool display;
	unsigned int scale;
	double pixel_ratio;

	/// The image filename.
	support::FileName filename;

	/** Note that the BoundingBox is always relative to the BoundingBox
	 *  as stored in the EPS file.
	 *  Ie, bb.xl and bb.yb == 0 if that corner is not moved.
	 */
	BoundingBox bb;

	/** The size of the view inside lyx in pixels or the scaling of the
	 *  image.
	 */
	/// Rotation angle.
	double angle;
};

bool operator==(Params const &, Params const &);
bool operator!=(Params const &, Params const &);

} // namespace graphics
} // namespace lyx

#endif // GRAPHICSPARAMS_H
