/*
 * \file GraphicsImage.C
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author Baruch Even <baruch.even@writeme.com>
 * \author Angus Leeming <leeming@lyx.org>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "GraphicsImage.h"
#include "GraphicsParams.h"

namespace grfx {

// This is to be connected to a function that will return a new
// instance of a viable derived class.
boost::function0<Image::ImagePtr> Image::newImage;

/// Return the list of loadable formats.
boost::function0<Image::FormatList> Image::loadableFormats;


std::pair<unsigned int, unsigned int>
Image::getScaledDimensions(Params const & params) const
{
	if (params.scale == 0 && params.width == 0 && params.height == 0)
		// No scaling
		return std::make_pair(getWidth(), getHeight());

	typedef unsigned int dimension;
	dimension width  = 0;
	dimension height = 0;
	if (params.scale != 0) {
		width  = dimension(getWidth()  * params.scale / 100.0);
		height = dimension(getHeight() * params.scale / 100.0);
	} else {
		width  = params.width;
		height = params.height;

		if (width == 0) {
			width = height * getWidth() / getHeight();
		} else if (height == 0) {
			height = width * getHeight() / getWidth();
		}
	}

	if (width == 0 || height == 0)
		// Something is wrong!
		return std::make_pair(getWidth(), getHeight());

	return std::make_pair(width, height);
}

} // namespace grfx
