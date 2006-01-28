/**
 * \file LyXGdkImage.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming (original Qt version)
 * \author John Levon (original Qt version)
 * \author John Spray
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

// Too hard to make concept checks work with this file
#ifdef _GLIBCXX_CONCEPT_CHECKS
#undef _GLIBCXX_CONCEPT_CHECKS
#endif
#ifdef _GLIBCPP_CONCEPT_CHECKS
#undef _GLIBCPP_CONCEPT_CHECKS
#endif



#include "LyXGdkImage.h"

#include "debug.h"
#include "format.h"

#include "graphics/GraphicsParams.h"

#include "support/lstrings.h"       // lowercase

#include <boost/bind.hpp>
#include <boost/tuple/tuple.hpp>

using lyx::support::lowercase;

using boost::bind;

using std::endl;
using std::equal_to;
using std::find_if;
using std::string;


namespace lyx {
namespace graphics {

/// Access to this class is through this static method.
Image::ImagePtr LyXGdkImage::newImage()
{
	ImagePtr ptr;
	ptr.reset(new LyXGdkImage);
	return ptr;
}


/// Return the list of loadable formats.
Image::FormatList LyXGdkImage::loadableFormats()
{
	static FormatList fmts;

	if (!fmts.empty())
		return fmts;

	// The formats recognised by LyX
	Formats::const_iterator begin = formats.begin();
	Formats::const_iterator end   = formats.end();

	lyxerr[Debug::GRAPHICS]
		<< "\nThe image loader can load the following directly:\n";

	Gdk::Pixbuf::SListHandle_PixbufFormat gdkformats = Gdk::Pixbuf::get_formats();
	Gdk::Pixbuf::SListHandle_PixbufFormat::iterator it = gdkformats.begin();
	Gdk::Pixbuf::SListHandle_PixbufFormat::iterator gdk_end = gdkformats.end();
	
	for (; it != gdk_end; ++it) {
		Gdk::PixbufFormat thisformat = (*it);
		lyxerr[Debug::GRAPHICS] << thisformat.get_name() << endl;

		std::vector<Glib::ustring> extensions = thisformat.get_extensions();
		std::vector<Glib::ustring>::const_iterator ext_end = extensions.end();
		std::vector<Glib::ustring>::iterator ext_it = extensions.begin();
		for (; ext_it != ext_end; ++ext_it) {
			std::string ext = lowercase(*ext_it);
			Formats::const_iterator fit =
				find_if(begin, end,
					bind(equal_to<string>(),
					     bind(&Format::extension, _1),
					     ext));
			if (fit != end)
				fmts.push_back(fit->name());
		}
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


LyXGdkImage::LyXGdkImage()
	: Image()
{
}


LyXGdkImage::LyXGdkImage(LyXGdkImage const & other)
	: Image(other), original_(other.original_),
	  transformed_(other.transformed_)
{}


Image * LyXGdkImage::clone_impl() const
{
	return new LyXGdkImage(*this);
}


unsigned int LyXGdkImage::getWidth_impl() const
{
	return transformed_->get_width();
}


unsigned int LyXGdkImage::getHeight_impl() const
{
	return transformed_->get_height();
}


void LyXGdkImage::load_impl(string const & filename)
{
	if (original_) {
		lyxerr[Debug::GRAPHICS]
			<< "Image is loaded already!" << endl;
		finishedLoading(false);
		return;
	}

	original_ = Gdk::Pixbuf::create_from_file(filename);

	if (!original_){
		lyxerr[Debug::GRAPHICS]
			<< "Unable to open image" << endl;
		finishedLoading(false);
		return;
	}

	transformed_ = original_;
	finishedLoading(true);
}

/*
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
*/

bool LyXGdkImage::setPixmap_impl(Params const & params)
{
	if (!original_ || params.display == NoDisplay)
		return false;

	// TODO: implement grayscale and monochrome
	switch (params.display) {
	case GrayscaleDisplay: {
		//toGray(transformed_);
		//transformed_->saturate_and_pixelate (dest, 0.0, false);
		break;
	}

	case MonochromeDisplay: {
		//transformed_.convertDepth(transformed_.depth(), Qt::MonoOnly);
		break;
	}

	default:
		break;
	}

	return true;
}


void LyXGdkImage::clip_impl(Params const & params)
{
	if (!transformed_)
		return;

	if (params.bb.empty())
		// No clipping is necessary.
		return;

	int const new_width  = params.bb.xr - params.bb.xl;
	int const new_height = params.bb.yt - params.bb.yb;

	// No need to check if the width, height are > 0 because the
	// Bounding Box would be empty() in this case.
	if (new_width > original_->get_width() || new_height > original_->get_height()) {
		// Bounds are invalid.
		return;
	}

	if (new_width == original_->get_width() && new_height == original_->get_height())
		return;

	int const xoffset_l = params.bb.xl;
	int const yoffset_t = (original_->get_height() > int(params.bb.yt) ?
			       original_->get_height() - params.bb.yt : 0);

	transformed_ = Gdk::Pixbuf::create_subpixbuf(original_,
		xoffset_l, yoffset_t, new_width, new_height);
}


void LyXGdkImage::rotate_impl(Params const & params)
{
	if (!transformed_)
		return;

	if (!params.angle)
		return;

	// TODO: allow free rotation
	// Temporarily commented out for old gtkmm versions
/*
	Gdk::PixbufRotation rotation = Gdk::PIXBUF_ROTATE_NONE;
	if (params.angle == 90.0)
		rotation = Gdk::PIXBUF_ROTATE_COUNTERCLOCKWISE;
	else if (params.angle == 180.0)
		rotation = Gdk::PIXBUF_ROTATE_UPSIDEDOWN;
	else if (params.angle == 270.0)
		rotation = Gdk::PIXBUF_ROTATE_CLOCKWISE;
		
		
	transformed_ = transformed_->rotate_simple(rotation);
	*/
}


void LyXGdkImage::scale_impl(Params const & params)
{
	if (!transformed_)
		return;

	unsigned int width;
	unsigned int height;
	boost::tie(width, height) = getScaledDimensions(params);

	if (width == getWidth() && height == getHeight())
		return;

	transformed_ = transformed_->scale_simple(
		width, height, Gdk::INTERP_BILINEAR);
}

} // namespace graphics
} // lyx
