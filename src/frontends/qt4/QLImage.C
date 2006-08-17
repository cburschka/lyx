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

#ifndef QT3_SUPPORT
// #define QT3_SUPPORT
#endif

#include <config.h>

#include "QLImage.h"
#include "qt_helpers.h"

#include "debug.h"
#include "format.h"

#include "graphics/GraphicsParams.h"

#include "support/lstrings.h"       // lowercase

#include <QPainter>
#include <QPictureIO>
#include <QPicture>
#include <QImage>
#include <QImageReader>

#include <boost/bind.hpp>
#include <boost/tuple/tuple.hpp>

using lyx::support::lowercase;

using boost::bind;

using std::endl;
using std::equal_to;
using std::find_if;
using std::string;

QPictureIO StaticPicture;

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


//	lyxerr[Debug::GRAPHICS]
//		<< "D:/msys/home/yns/src/lyx-devel/lib/images/banner.ppm is of format: "
//		<< fromqstr(Pic.pictureFormat("D:/msys/home/yns/src/lyx-devel/lib/images/banner.ppm"))
//		<< endl;
//	if (Pic.pictureFormat("D:/msys/home/yns/src/lyx-devel/lib/images/banner.ppm"))
//		lyxerr[Debug::GRAPHICS]
//			<< "pictureFormat not returned NULL\n" << endl;
//			<< "Supported formats are: " << Pic.inputFormats() << endl;

	QList<QByteArray> qt_formats = QImageReader::supportedImageFormats ();

	lyxerr[Debug::GRAPHICS]
		<< "\nThe image loader can load the following directly:\n";

	if (qt_formats.empty())
		lyxerr[Debug::GRAPHICS]
			<< "\nQt4 Problem: No Format available!" << endl;

	for (QList<QByteArray>::const_iterator it =qt_formats.begin(); it != qt_formats.end(); ++it) {

		lyxerr[Debug::GRAPHICS] << (const char *) *it << ", ";

		string ext = lowercase((const char *) *it);

		// special case
		if (ext == "jpeg")
			ext = "jpg";

		Formats::const_iterator fit =
			find_if(begin, end,
				bind(equal_to<string>(),
				     bind(&Format::extension, _1),
				     ext));
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
	  transformed_(other.transformed_),
	  transformed_pixmap_(other.transformed_pixmap_)
{}


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

	// FIXME this code used to be like this:
	//
	//unsigned int * const data = img.depth() > 8 ?
	//	(unsigned int *)img.bits() :
	//	(unsigned int *)img.jumpTable();
	// 
	// But Qt doc just say use bits...
	unsigned int * const data = (unsigned int *)img.bits();

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
		transformed_.convertToFormat(transformed_.format(), Qt::MonoOnly);
		break;
	}

	default:
		break;
	}

	transformed_pixmap_ = QPixmap::fromImage(transformed_);
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

	QMatrix m;
	m.rotate(-params.angle);

	// FIXME: alpha chanel detection is automautic for monochrome
	// and 8-bit images. For 32 bit, is something like still necessary?
	//transformed_.setAlphaBuffer(true);
	transformed_ = transformed_.transformed(m);
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

	QMatrix m;
	m.scale(double(width) / getWidth(), double(height) / getHeight());
	transformed_ = transformed_.transformed(m);
}

} // namespace graphics
} // lyx
