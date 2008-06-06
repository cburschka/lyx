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
	  transformed_(other.transformed_),
	  transformed_pixmap_(other.transformed_pixmap_)
{}


Image * GuiImage::clone() const
{
	return new GuiImage(*this);
}


unsigned int GuiImage::width() const
{
	return transformed_.width();
}


unsigned int GuiImage::height() const
{
	return transformed_.height();
}


void GuiImage::load(FileName const & filename)
{
	if (!original_.isNull()) {
		LYXERR(Debug::GRAPHICS, "Image is loaded already!");
		finishedLoading(false);
		return;
	}

	if (!original_.load(toqstr(filename.absFilename()))) {
		LYXERR(Debug::GRAPHICS, "Unable to open image");
		finishedLoading(false);
		return;
	}
	transformed_ = original_;
	finishedLoading(true);
}


// This code is taken from KImageEffect::toGray
static QImage & toGray(QImage & img)
{
	if (img.width() == 0 || img.height() == 0)
		return img;

	int const pixels = img.depth() > 8 ?
		img.width() * img.height() : img.numColors();

	unsigned int *data = img.depth() > 8 ?
		reinterpret_cast<unsigned int *>(img.bits()) :
		reinterpret_cast<unsigned int *>(&img.colorTable()[0]);

	for(int i = 0; i < pixels; ++i){
		int const val = qGray(data[i]);
		data[i] = qRgba(val, val, val, qAlpha(data[i]));
	}
	return img;
}


bool GuiImage::setPixmap(Params const & params)
{
	if (original_.isNull() || params.display == NoDisplay)
		return false;

	switch (params.display) {
	case GrayscaleDisplay: {
		toGray(transformed_);
		break;
	}

	case MonochromeDisplay: {
		transformed_.convertToFormat(transformed_.format(), Qt::MonoOnly);
		break;
	}

	default:
		break;
	}

	transformed_pixmap_ = QPixmap::fromImage(transformed_);
	return true;
}


void GuiImage::clip(Params const & params)
{
	if (transformed_.isNull())
		return;

	if (params.bb.empty())
		// No clipping is necessary.
		return;

	int const new_width  = params.bb.xr - params.bb.xl;
	int const new_height = params.bb.yt - params.bb.yb;

	// No need to check if the width, height are > 0 because the
	// Bounding Box would be empty() in this case.
	if (new_width > original_.width() || new_height > original_.height()) {
		// Bounds are invalid.
		return;
	}

	if (new_width == original_.width() && new_height == original_.height())
		return;

	int const xoffset_l = params.bb.xl;
	int const yoffset_t = (original_.height() > int(params.bb.yt) ?
			       original_.height() - params.bb.yt : 0);

	transformed_ = original_.copy(xoffset_l, yoffset_t,
				      new_width, new_height);
}


void GuiImage::rotate(Params const & params)
{
	if (transformed_.isNull())
		return;

	if (!params.angle)
		return;

	QMatrix m;
	m.rotate(-params.angle);

	transformed_ = transformed_.transformed(m);
}


void GuiImage::scale(Params const & params)
{
	if (transformed_.isNull())
		return;

	Dimension dim = scaledDimension(params);

	if (dim.width() == width() && dim.height() == height())
		return;

	QMatrix m;
	m.scale(double(dim.width()) / width(), double(dim.height()) / height());
	transformed_ = transformed_.transformed(m);
}

} // namespace graphics
} // lyx
