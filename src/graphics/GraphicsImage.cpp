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

#include "support/debug.h"


namespace lyx {
namespace graphics {

// This is to be connected to a function that will return a new
// instance of a viable derived class.
boost::function<Image *()> Image::newImage;

Dimension Image::scaledDimension(Params const & params) const
{
	// scale only when value > 0
	unsigned int w = width();
	unsigned int h = height();
	if (params.scale) {
		w = (w * params.scale) / 100;
		h = (h * params.scale) / 100;
	}

	LYXERR(Debug::GRAPHICS, "graphics::Image::getScaledDimensions()"
		<< "\n\tparams.scale       : " << params.scale
		<< "\n\twidth              : " << w
		<< "\n\theight             : " << h);

	return Dimension(w, h, 0);
}

} // namespace graphics
} // namespace lyx
