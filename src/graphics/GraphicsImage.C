/**
 * \file GraphicsImage.C
 * Read the file COPYING
 *
 * \author Baruch Even 
 * \author Angus Leeming 
 * \author Herbert Voss 
 *
 * Full author contact details available in file CREDITS
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
	// scale only when value > 0
	unsigned int width;
	unsigned int height;
	if (params.scale) {
		width  = (getWidth() * params.scale) / 100;
		height = (getHeight() * params.scale) / 100;
	} else {
		width = getWidth();
		height = getHeight();
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

