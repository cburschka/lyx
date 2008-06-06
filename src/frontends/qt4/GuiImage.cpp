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

	if (!original_.load(toqstr(filename.absFilename()))) {
		LYXERR(Debug::GRAPHICS, "Unable to open image");
		return false;
	}
	return true;
}


// This code is taken from KImageEffect::toGray
static QPixmap toGray(QPixmap const & pix)
{
	if (pix.width() == 0 || pix.height() == 0)
		return pix;

	QImage img = pix.toImage();
	int const pixels = img.depth() > 8 ?
		img.width() * img.height() : img.numColors();

	unsigned int *data = img.depth() > 8 ?
		reinterpret_cast<unsigned int *>(img.bits()) :
		reinterpret_cast<unsigned int *>(&img.colorTable()[0]);

	for(int i = 0; i < pixels; ++i){
		int const val = qGray(data[i]);
		data[i] = qRgba(val, val, val, qAlpha(data[i]));
	}
	return QPixmap::fromImage(img);
}


bool GuiImage::setPixmap(Params const & params)
{
	if (original_.isNull() || params.display == NoDisplay)
		return false;

	is_transformed_ = clip(params);
	is_transformed_ |= rotate(params);
	is_transformed_ |= scale(params);

	switch (params.display) {
	case GrayscaleDisplay: {
		transformed_ = is_transformed_
			? toGray(transformed_) :  toGray(original_);
		is_transformed_ = true;
		break;
	}

	case MonochromeDisplay: {
		QImage img = is_transformed_
			? transformed_.toImage() : original_.toImage();
		img.convertToFormat(img.format(), Qt::MonoOnly);
		transformed_ = QPixmap::fromImage(img);
		is_transformed_ = true;
		break;
	}

	default:
		break;
	}

	if (!is_transformed_)
		// Clear it out to save some memory.
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
