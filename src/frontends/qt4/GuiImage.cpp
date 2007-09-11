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

#include "debug.h"
#include "Format.h"

#include "graphics/GraphicsParams.h"

#include "support/FileName.h"
#include "support/lstrings.h"       // ascii_lowercase

#include <QPainter>
#include <QImage>
#include <QImageReader>

#include <boost/bind.hpp>
#include <boost/tuple/tuple.hpp>

using lyx::support::ascii_lowercase;

using boost::bind;

using std::endl;
using std::equal_to;
using std::find_if;
using std::string;

namespace lyx {
namespace graphics {

/// Access to this class is through this static method.
Image::ImagePtr GuiImage::newImage()
{
	ImagePtr ptr;
	ptr.reset(new GuiImage);
	return ptr;
}


/// Return the list of loadable formats.
Image::FormatList GuiImage::loadableFormats()
{
	static FormatList fmts;

	if (!fmts.empty())
		return fmts;

	// The formats recognised by LyX
	Formats::const_iterator begin = formats.begin();
	Formats::const_iterator end   = formats.end();


//	LYXERR(Debug::GRAPHICS)
//		<< "D:/msys/home/yns/src/lyx-devel/lib/images/banner.png mis of format: "
//		<< fromqstr(Pic.pictureFormat("D:/msys/home/yns/src/lyx-devel/lib/images/banner.png"))
//		<< endl;
//	if (Pic.pictureFormat("D:/msys/home/yns/src/lyx-devel/lib/images/banner.png"))
//		LYXERR(Debug::GRAPHICS)
//			<< "pictureFormat not returned NULL\n" << endl;
//			<< "Supported formats are: " << Pic.inputFormats() << endl;

	QList<QByteArray> qt_formats = QImageReader::supportedImageFormats ();

	LYXERR(Debug::GRAPHICS)
		<< "\nThe image loader can load the following directly:\n";

	if (qt_formats.empty())
		LYXERR(Debug::GRAPHICS)
			<< "\nQt4 Problem: No Format available!" << endl;

	for (QList<QByteArray>::const_iterator it =qt_formats.begin(); it != qt_formats.end(); ++it) {

		LYXERR(Debug::GRAPHICS) << (const char *) *it << ", ";

		string ext = ascii_lowercase((const char *) *it);

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
		LYXERR(Debug::GRAPHICS)
			<< "\nOf these, LyX recognises the following formats:\n";

		FormatList::const_iterator fbegin = fmts.begin();
		FormatList::const_iterator fend   = fmts.end();
		for (FormatList::const_iterator fit = fbegin; fit != fend; ++fit) {
			if (fit != fbegin)
				LYXERR(Debug::GRAPHICS) << ", ";
			LYXERR(Debug::GRAPHICS) << *fit;
		}
		LYXERR(Debug::GRAPHICS) << '\n' << endl;
	}

	return fmts;
}


GuiImage::GuiImage(GuiImage const & other)
	: Image(other), original_(other.original_),
	  transformed_(other.transformed_),
	  transformed_pixmap_(other.transformed_pixmap_)
{}


Image * GuiImage::clone_impl() const
{
	return new GuiImage(*this);
}


unsigned int GuiImage::getWidth_impl() const
{
	return transformed_.width();
}


unsigned int GuiImage::getHeight_impl() const
{
	return transformed_.height();
}


void GuiImage::load_impl(support::FileName const & filename)
{
	if (!original_.isNull()) {
		LYXERR(Debug::GRAPHICS)
			<< "Image is loaded already!" << endl;
		finishedLoading(false);
		return;
	}

	if (!original_.load(toqstr(filename.absFilename()))) {
		LYXERR(Debug::GRAPHICS)
			<< "Unable to open image" << endl;
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


bool GuiImage::setPixmap_impl(Params const & params)
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


void GuiImage::clip_impl(Params const & params)
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


void GuiImage::rotate_impl(Params const & params)
{
	if (transformed_.isNull())
		return;

	if (!params.angle)
		return;

	QMatrix m;
	m.rotate(-params.angle);

	transformed_ = transformed_.transformed(m);
}


void GuiImage::scale_impl(Params const & params)
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
