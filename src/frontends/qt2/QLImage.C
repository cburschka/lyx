/*
 * \file QLImage.C
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author Angus Leeming, a.leeming@ic.ac.uk
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "QLImage.h"
#include "graphics/GraphicsParams.h"
#include "converter.h"
#include "debug.h"
#include "support/LAssert.h"
#include "support/lyxfunctional.h"  // compare_memfun

#include <qimage.h>
#include <qwmatrix.h>
#include <qpainter.h>

#include <boost/tuple/tuple.hpp>

using std::find_if;
using std::endl;

namespace grfx {

/// Access to this class is through this static method.
Image::ImagePtr QLImage::newImage()
{
	ImagePtr ptr;
	ptr.reset(new QLImage);
	return ptr;
}


/// Return the list of loadable formats.
Image::FormatList QLImage::loadableFormats()
{
	static FormatList fmts;
 
	if (!fmts.empty())
		return fmts;

	// The formats recognised by LyX
	Formats::const_iterator begin = formats.begin();
	Formats::const_iterator end   = formats.end();

	lyxerr[Debug::GRAPHICS]
		<< "\nThe image loader can load the following directly:\n";

	QStrList qt_formats = QImageIO::inputFormats();
 
	QStrListIterator it(qt_formats);

	for (; it.current(); ++it) {
	   lyxerr[Debug::GRAPHICS] << it.current() << endl;

		string ext = lowercase(it.current());
	 
		// special case
		if (ext == "jpeg")
			ext = "jpg";

		Formats::const_iterator fit =
			find_if(begin, end, lyx::compare_memfun(&Format::extension, ext));
		if (fit != end)
			fmts.push_back(fit->name());
	}

	lyxerr[Debug::GRAPHICS]
		<< "\nOf these, LyX recognises the following formats:\n";

	FormatList::const_iterator fbegin = fmts.begin();
	FormatList::const_iterator fend   = fmts.end();
	for (FormatList::const_iterator fit = fbegin; fit != fend; ++fit) {
		if (fit != fbegin)
			lyxerr[Debug::GRAPHICS] << ", ";
		lyxerr[Debug::GRAPHICS] << *fit;
	}
	lyxerr[Debug::GRAPHICS] << '\n' << endl;

	return fmts;
}


QLImage::QLImage()
	: Image()
{
}


QLImage::QLImage(QLImage const & other)
	: Image(other), pixmap_(other.pixmap_), xformed_pixmap_(other.xformed_pixmap_)
{
}


QLImage::~QLImage()
{
}


Image * QLImage::clone() const
{
	return new QLImage(*this);
}


unsigned int QLImage::getWidth() const
{
	return xformed_pixmap_.width();
}


unsigned int QLImage::getHeight() const
{
	return xformed_pixmap_.height();
}


void QLImage::load(string const & filename)
{
	if (!pixmap_.isNull()) {
		lyxerr[Debug::GRAPHICS]
			<< "Image is loaded already!" << endl;
		finishedLoading(false);
		return;
	}

	if (!pixmap_.load(filename.c_str())) {
		lyxerr[Debug::GRAPHICS]
			<< "Unable to open image" << endl;
		finishedLoading(false);
		return;
	}
	lyxerr[Debug::GRAPHICS] << "just Loaded." << endl; 
	xformed_pixmap_ = pixmap_;
	lyxerr[Debug::GRAPHICS] << "pixmap isNull " << pixmap_.isNull()
		<< " xformed_pixmap_ isNull " << xformed_pixmap_.isNull() << endl;
	finishedLoading(true); 
}


bool QLImage::setPixmap(Params const & params)
{
	lyxerr[Debug::GRAPHICS] << "pixmap isNull " << pixmap_.isNull()
		<< " xformed_pixmap_ isNull " << xformed_pixmap_.isNull() << endl;
	if (pixmap_.isNull() || params.display == NoDisplay)
		return false;

	lyxerr[Debug::GRAPHICS] << "setPixmap()" << endl;
 
// FIXME
#if 0  
	int color_key;
	switch (params.display) {
	case MonochromeDisplay:
		color_key = FL_IMAGE_MONO;
		break;
	case GrayscaleDisplay:
		color_key = FL_IMAGE_GRAY;
		break;
	case ColorDisplay:
	default: // NoDisplay cannot happen!
		color_key = FL_IMAGE_RGB;
		break;
	}

	if (color_key != FL_IMAGE_RGB) {
		flimage_convert(image_, color_key, 0);
	}

	unsigned int fill = packedcolor(LColor::graphicsbg);
	if (fill != image_->fill_color) {
		// the background color has changed.
		// Note that in grayscale/monochrome images the background is
		// grayed also, so this call will have no visible effect. Sorry!
		flimage_replace_pixel(image_, image_->fill_color, fill);
		image_->fill_color = fill;
	}
#endif 

	return true;
}


void QLImage::clip(Params const & params)
{
	lyxerr << "clip isNull " << pixmap_.isNull() << ", " <<xformed_pixmap_.isNull() << endl; 
	if (xformed_pixmap_.isNull())
		return;

	if (params.bb.empty())
		// No clipping is necessary.
		return;

	int const new_width  = params.bb.xr - params.bb.xl;
	int const new_height = params.bb.yt - params.bb.yb;

	// No need to check if the width, height are > 0 because the
	// Bounding Box would be empty() in this case.
	if (new_width > pixmap_.width() || new_height > pixmap_.height()) {
		// Bounds are invalid.
		return;
	}

	if (new_width == pixmap_.width() && new_height == pixmap_.height())
		return;

	int const xoffset_l = std::max(0, int(params.bb.xl));
	int const yoffset_t = std::max(0, pixmap_.height() - int(params.bb.yt));

	xformed_pixmap_.resize(new_width, new_height);
	QPainter p;
	p.begin(&xformed_pixmap_);
	p.drawPixmap(0, 0, pixmap_, xoffset_l, yoffset_t, new_width, new_height);
	p.end();
}


void QLImage::rotate(Params const & params)
{
	lyxerr << "rotate isNull " << pixmap_.isNull() << ", " <<xformed_pixmap_.isNull() << endl; 
	if (xformed_pixmap_.isNull())
		return;

	if (!params.angle)
		return;

	// The angle passed to flimage_rotate is the angle in one-tenth of a
	// degree units.

	lyxerr[Debug::GRAPHICS] << "rotating image by " << params.angle << " degrees" << endl;
 
	QWMatrix m;
	m.rotate(-params.angle);
	xformed_pixmap_ = xformed_pixmap_.xForm(m);
}


void QLImage::scale(Params const & params)
{
	lyxerr << "scale isNull " << pixmap_.isNull() << ", " <<xformed_pixmap_.isNull() << endl; 
	if (xformed_pixmap_.isNull())
		return;

	unsigned int width;
	unsigned int height;
	boost::tie(width, height) = getScaledDimensions(params);

	if (width == getWidth() && height == getHeight())
		return;

	lyxerr[Debug::GRAPHICS] << "resizing image to " << width << "(" <<
		(double(width)/getWidth()) << ")," << height << "(" <<
		(double(height)/getHeight()) << ")" << endl;
	QWMatrix m;
	m.scale(double(width) / getWidth(), double(height) / getHeight());
	xformed_pixmap_ = xformed_pixmap_.xForm(m);
}

} // namespace grfx
