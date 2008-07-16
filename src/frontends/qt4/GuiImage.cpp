/**
 * \file GuiImage.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiImage.h"
#include "qt_helpers.h"

#include "Format.h"

#include "graphics/GraphicsParams.h"

#include "support/debug.h"
#include "support/FileName.h"
#include "support/lstrings.h"       // ascii_lowercase

#include <QPainter>
#include <QImage>
#include <QImageReader>

using namespace std;
using namespace lyx::support;

namespace lyx {
namespace graphics {

/// Access to this class is through this static method.
Image * GuiImage::newImage()
{
	return new GuiImage;
}


GuiImage::GuiImage(GuiImage const & other)
	: Image(other), original_(other.original_),
	  transformed_(other.transformed_), is_transformed_(other.is_transformed_)
{}


Image * GuiImage::clone() const
{
	return new GuiImage(*this);
}


unsigned int GuiImage::width() const
{
	return is_transformed_ ? transformed_.width() : original_.width();
}


unsigned int GuiImage::height() const
{
	return is_transformed_ ? transformed_.height() : original_.height();
}


bool GuiImage::load(FileName const & filename)
{
	if (!original_.isNull()) {
		LYXERR(Debug::GRAPHICS, "Image is loaded already!");
		return false;
	}

	fname_ = toqstr(filename.absFilename());

	if (!original_.load(fname_)) {
		LYXERR(Debug::GRAPHICS, "Unable to open image");
		return false;
	}
	return true;
}


bool GuiImage::setPixmap(Params const & params)
{
	if (!params.display)
		return false;

	if (original_.isNull()) {
		if (original_.load(fname_))
			return false;
	}
		
	is_transformed_ = clip(params);
	is_transformed_ |= rotate(params);
	is_transformed_ |= scale(params);

	// Clear the pixmap to save some memory.
	if (is_transformed_)
		original_ = QPixmap();
	else
		transformed_ = QPixmap();

	return true;
}


bool GuiImage::clip(Params const & params)
{
	if (params.bb.empty())
		// No clipping is necessary.
		return false;

	int const new_width  = params.bb.xr - params.bb.xl;
	int const new_height = params.bb.yt - params.bb.yb;

	// No need to check if the width, height are > 0 because the
	// Bounding Box would be empty() in this case.
	if (new_width > original_.width() || new_height > original_.height()) {
		// Bounds are invalid.
		return false;
	}

	if (new_width == original_.width() && new_height == original_.height())
		return false;

	int const xoffset_l = params.bb.xl;
	int const yoffset_t = (original_.height() > int(params.bb.yt) ?
			       original_.height() - params.bb.yt : 0);

	transformed_ = original_.copy(xoffset_l, yoffset_t,
				      new_width, new_height);
	return true;
}


bool GuiImage::rotate(Params const & params)
{
	if (!params.angle)
		return false;

	if (!is_transformed_)
		transformed_ = original_;

	QMatrix m;
	m.rotate(-params.angle);
	transformed_ = transformed_.transformed(m);
	return true;
}


bool GuiImage::scale(Params const & params)
{
	Dimension dim = scaledDimension(params);

	if (dim.width() == width() && dim.height() == height())
		return false;

	if (!is_transformed_)
		transformed_ = original_;

	QMatrix m;
	m.scale(double(dim.width()) / width(), double(dim.height()) / height());
	transformed_ = transformed_.transformed(m);

	return true;
}

} // namespace graphics
} // lyx
