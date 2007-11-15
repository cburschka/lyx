/**
 * \file GraphicsImage.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Baruch Even
 * \author Angus Leeming
 * \author Herbert Voﬂ
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GraphicsImage.h"
#include "GraphicsParams.h"
#include "debug.h"


namespace lyx {
namespace graphics {

// This is to be connected to a function that will return a new
// instance of a viable derived class.
boost::function<Image::ImagePtr()> Image::newImage;

/// Return the list of loadable formats.
boost::function<Image::FormatList()> Image::loadableFormats;


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

	LYXERR(Debug::GRAPHICS, "graphics::Image::getScaledDimensions()"
		<< "\n\tparams.scale       : " << params.scale
		<< "\n\twidth              : " << width
		<< "\n\theight             : " << height);

	return std::make_pair(width, height);
}

} // namespace graphics
} // namespace lyx
