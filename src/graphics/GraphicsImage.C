/*
 * \file GraphicsImage.C
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author Baruch Even <baruch.even@writeme.com>
 * \author Angus Leeming <leeming@lyx.org>
 * \author Herbert Voss <voss@lyx.org>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "GraphicsImage.h"
#include "GraphicsParams.h"
#include "debug.h"

using std::endl;
using std::abs;

namespace grfx {

// This is to be connected to a function that will return a new
// instance of a viable derived class.
boost::function0<Image::ImagePtr> Image::newImage;

/// Return the list of loadable formats.
boost::function0<Image::FormatList> Image::loadableFormats;


std::pair<unsigned int, unsigned int>
Image::getScaledDimensions(Params const & params) const
{
	unsigned int width = getWidth();
	unsigned int height = getHeight();

	// scale only when value makes sense, i.e. not zero
	if (params.scale) {
		width  = (width * params.scale) / 100;
		height = (height * params.scale) / 100;
	}

	lyxerr[Debug::GRAPHICS]
		<< "GraphicsImage::getScaledDImensions()"
		<< "\n\tparams.scale       : " << params.scale
		<< "\n\twidth              : " << width
		<< "\n\theight             : " << height
		<< std::endl;

	return std::make_pair(width, height);
}

} // namespace grfx

