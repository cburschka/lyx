// -*- C++ -*-
/**
 *  \file GraphicsParams.h
 *  Copyright 2002 the LyX Team
 *  Read the file COPYING
 *
 *  \author Angus Leeming <a.leeming@ic.ac.uk>
 *
 *  Used internally by the GraphicsCache.
 */

#ifndef GRAPHICSPARAMS_H
#define GRAPHICSPARAMS_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"


namespace grfx {

/** Parse a string of the form "200pt 500pt 300mm 5in" into a
 *  usable bounding box.
 */
struct BoundingBox {
	///
	BoundingBox();
	///
	BoundingBox(string const &);

	/// 0 0 0 0 is empty!
	bool empty() const;

	int xl;
	int yb;
	int xr;
	int yt;
};

bool operator==(BoundingBox const &, BoundingBox const &);
bool operator!=(BoundingBox const &, BoundingBox const &);

struct GParams
{
	GParams();

	/// How is the image to be displayed on the LyX screen?
	enum DisplayType {
		COLOR,
		GRAYSCALE,
		MONOCHROME,
		/// We aren't going to display it at all!
		NONE
	};

	DisplayType display;

	/// The image filename.
	string filename;

	/** Note that the BoundingBox is always relative to the BoundingBox
	 *  as stored in the EPS file.
	 *  Ie, bb.xl and bb.yb == 0 if that corner is not moved.
	 */
	BoundingBox bb;

	/** The size of the view inside lyx in pixels or the scaling of the
	 *  image.
	 */
	unsigned int width;
	unsigned int height;
	unsigned int scale;

	/// Rotation angle.
	int angle;
};

bool operator==(GParams const &, GParams const &);
bool operator!=(GParams const &, GParams const &);

} // namespace grfx

#endif // GRAPHICSPARAMS_H
