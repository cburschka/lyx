/**
 * \file QLImage.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QLImage.h"
#include "qt_helpers.h"

#include "debug.h"
#include "format.h"

#include "graphics/GraphicsParams.h"

#include "support/lstrings.h"       // lowercase
#include "support/lyxfunctional.h"  // compare_memfun

#include <qimage.h>
#include <qpainter.h>

#include <boost/tuple/tuple.hpp>

using lyx::support::lowercase;

using std::endl;
using std::find_if;
using std::string;


namespace lyx {
namespace graphics {

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

	if (lyxerr.debugging()) {
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
	}

	return fmts;
}


QLImage::QLImage()
	: Image()
{
}


QLImage::QLImage(QLImage const & other)
	: Image(other), original_(other.original_),
	  transformed_(other.original_)
{}


QLImage::~QLImage()
{
}


Image * QLImage::clone_impl() const
{
	return new QLImage(*this);
}


unsigned int QLImage::getWidth_impl() const
{
	return transformed_.width();
}


unsigned int QLImage::getHeight_impl() const
{
	return transformed_.height();
}


void QLImage::load_impl(string const & filename)
{
	if (!original_.isNull()) {
		lyxerr[Debug::GRAPHICS]
			<< "Image is loaded already!" << endl;
		finishedLoading(false);
		return;
	}

	if (!original_.load(toqstr(filename))) {
		lyxerr[Debug::GRAPHICS]
			<< "Unable to open image" << endl;
		finishedLoading(false);
		return;
	}
	transformed_ = original_;
	finishedLoading(true);
}


namespace {

// This code is taken from KImageEffect::toGray
QImage & toGray(QImage & img)
{
	if (img.width() == 0 || img.height() == 0)
		return img;

	int const pixels = img.depth() > 8 ?
		img.width() * img.height() : img.numColors();

	unsigned int * const data = img.depth() > 8 ?
		(unsigned int *)img.bits() :
		(unsigned int *)img.colorTable();

	for(int i = 0; i < pixels; ++i){
		int const val = qGray(data[i]);
		data[i] = qRgba(val, val, val, qAlpha(data[i]));
	}
	return img;
}

} // namespace anon


bool QLImage::setPixmap_impl(Params const & params)
{
	if (original_.isNull() || params.display == NoDisplay)
		return false;

	switch (params.display) {
	case GrayscaleDisplay: {
		toGray(transformed_);
		break;
	}

	case MonochromeDisplay: {
		transformed_.convertDepth(transformed_.depth(), Qt::MonoOnly);
		break;
	}

	default:
		break;
	}

	return true;
}


void QLImage::clip_impl(Params const & params)
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


void QLImage::rotate_impl(Params const & params)
{
	if (transformed_.isNull())
		return;

	if (!params.angle)
		return;

	QWMatrix m;
	m.rotate(-params.angle);

	transformed_.setAlphaBuffer(true);
	transformed_ = transformed_.xForm(m);
}


void QLImage::scale_impl(Params const & params)
{
	if (transformed_.isNull())
		return;

	unsigned int width;
	unsigned int height;
	boost::tie(width, height) = getScaledDimensions(params);

	if (width == getWidth() && height == getHeight())
		return;

	QWMatrix m;
	m.scale(double(width) / getWidth(), double(height) / getHeight());
	transformed_ = transformed_.xForm(m);
}

} // namespace graphics
} // lyx
