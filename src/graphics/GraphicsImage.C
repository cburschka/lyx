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
	lyxerr[Debug::GRAPHICS]
		<< "GraphicsImage::getScaledDImensions()"
		<< "\n\tparams.scale       : " << params.scale
		<< "\n\tparams.width       : " << params.width
		<< "\n\tparams.height      : " << params.height
		<< "\n\tkeepLyXAspectRatio : " << params.keepLyXAspectRatio
		<< std::endl;
	if (params.width == 0 && params.height == 0 && params.scale == 0) {
		// original size or scale/custom without any input
		lyxerr[Debug::GRAPHICS]
			<< "\treturn with the original values!\n";
		return std::make_pair(getWidth(), getHeight());
	}
	
	typedef unsigned int dimension;
	dimension width  = 0;
	dimension height = 0;
	if (params.scale != 0) {
		// GraphicsParams::Scale 
		width  = dimension(double(getWidth())  * params.scale / 100.0);
		height = dimension(getHeight() * params.scale / 100.0);
		return std::make_pair(width, height);
	} 
	// GraphicsParams::WH
	width  = (params.width > 0) ? params.width : getWidth();
	height = (params.height > 0) ? params.height : getHeight(); 
	if (!params.keepLyXAspectRatio)
		return std::make_pair(width, height);

	// calculate aspect ratio
	float const rw  = getWidth();
	float const rh = getHeight();
	// there must be a width for the division
	float const ratio = (rw > 0.001) ? rh/rw : 1.0;
	lyxerr[Debug::GRAPHICS]
		<< "\tValue of LyXAspectRatio: " << ratio << std::endl;
	// there are now four different cases
	// w=0 & h=0 -> see above, no more possible at this place
	// w>0 & h=0 -> calculate h
	// w=0 & h>0 -> calculate w
	// w>0 & h>0 -> the greatest difference to the original
	//              value becomes the same
	if (params.width > 0 && params.height > 0) {
		// both widths are given and keepAspectRatio, too
		int const diff_width = abs(int(getWidth() - params.width));
		int const diff_height= abs(int(getHeight() - params.height));
		if (diff_width > diff_height)
			height = int(ratio * params.width);
		else
			width = int(ratio * params.height);
		return std::make_pair(width, height);
	}
	if (params.width > 0) {
		width = params.width;
		height = int(ratio * params.width);
		return std::make_pair(width, height);
	}
	if (params.height > 0) {
		height = params.height;
		width = int(ratio * params.height);
		return std::make_pair(width, height);
	}
	// all other cases ... kind of paranoia :-)
	return std::make_pair(getWidth(), getHeight());
}

} // namespace grfx

