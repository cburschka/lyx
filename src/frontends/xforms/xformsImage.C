/*
 * \file xformsImage.C
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author Angus Leeming, a.leeming@ic.ac.uk
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "xformsImage.h"
#include "graphics/GraphicsParams.h"
#include "LColor.h"
#include "converter.h"              // formats
#include "debug.h"
#include "support/LAssert.h"
#include "support/lyxfunctional.h"  // compare_memfun

#include FORMS_H_LOCATION

#ifdef HAVE_FLIMAGE_H
# include <flimage.h>
#else
# ifdef HAVE_X11_FLIMAGE_H
# include <X11/flimage.h>
# endif
#endif

#include <boost/tuple/tuple.hpp>

using std::find_if;

namespace {

void init_graphics();

unsigned int packedcolor(LColor::color c);

} // namespace anon


namespace grfx {

/// Access to this class is through this static method.
Image::ImagePtr xformsImage::newImage()
{
	init_graphics();

	ImagePtr ptr;
	ptr.reset(new xformsImage);
	return ptr;
}


/// Return the list of loadable formats.
Image::FormatList xformsImage::loadableFormats()
{
	static FormatList fmts;
	if (!fmts.empty())
		return fmts;

	init_graphics();

	// The formats recognised by LyX
	Formats::const_iterator begin = formats.begin();
	Formats::const_iterator end   = formats.end();

	lyxerr[Debug::GRAPHICS]
		<< "\nThe image loader can load the following directly:\n";

	// Don't forget the Fortran numbering used by xforms!
	int const nformats = flimage_get_number_of_formats();
	for (int i = 1; i <= nformats; ++i) {

		FLIMAGE_FORMAT_INFO const * info = flimage_get_format_info(i);
		string const formal_name =
			info->formal_name ? info->formal_name : string();
		string ext =
			info->extension   ? info->extension   : string();

		if (ext.empty() || ext == "gz")
			continue;

		if (ext == "rgb") ext = "sgi";

		lyxerr[Debug::GRAPHICS]
			<< formal_name << ", extension \"" << ext << "\"\n";

		Formats::const_iterator it =
			find_if(begin, end,
				lyx::compare_memfun(&Format::extension, ext));
		if (it != end)
			fmts.push_back(it->name());
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
	lyxerr[Debug::GRAPHICS] << '\n' << std::endl;

	return fmts;
}


xformsImage::xformsImage()
	: image_(0),
	  pixmap_(0),
	  pixmap_status_(PIXMAP_UNINITIALISED)
{}


xformsImage::xformsImage(xformsImage const & other)
	: Image(other),
	  image_(0),
	  pixmap_(0),
	  pixmap_status_(PIXMAP_UNINITIALISED)
{
	if (other.image_) {
		image_ = flimage_dup(other.image_);
		image_->u_vdata = this;
	}
}


xformsImage::~xformsImage()
{
	if (image_)
		flimage_free(image_);
	if (pixmap_)
		XFreePixmap(fl_get_display(), pixmap_);
}


Image * xformsImage::clone() const
{
	return new xformsImage(*this);
}


unsigned int xformsImage::getWidth() const
{
	if (!image_)
		return 0;

	// Why, oh why do we need such hacks?
	// Angus 12 July 2002
	return image_->w + 2;
}


unsigned int xformsImage::getHeight() const
{
	if (!image_)
		return 0;
	return image_->h;
}


Pixmap xformsImage::getPixmap() const
{
	if (!pixmap_status_ == PIXMAP_SUCCESS)
		return 0;
	return pixmap_;
}


void xformsImage::load(string const & filename)
{
	if (image_) {
		lyxerr[Debug::GRAPHICS]
			<< "Image is loaded already!" << std::endl;
		finishedLoading(false);
		return;
	}

	image_ = flimage_open(filename.c_str());
	if (!image_) {
		lyxerr[Debug::GRAPHICS]
			<< "Unable to open image" << std::endl;
		finishedLoading(false);
		return;
	}

	// Set this now and we won't need to bother again.
	image_->fill_color = packedcolor(LColor::graphicsbg);

	// Used by the callback routines to return to this
	image_->u_vdata = this;

	// Begin the reading process.
	flimage_read(image_);
}


bool xformsImage::setPixmap(Params const & params)
{
	if (!image_ || params.display == NoDisplay)
		return false;

	Display * display = fl_get_display();

	if (pixmap_ && pixmap_status_ == PIXMAP_SUCCESS)
		XFreePixmap(display, pixmap_);

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

	image_->xdisplay = display;
	Screen * screen  = ScreenOfDisplay(display, fl_screen);

	pixmap_ = flimage_to_pixmap(image_, XRootWindowOfScreen(screen));
	pixmap_status_ = pixmap_ ? PIXMAP_SUCCESS : PIXMAP_FAILED;

	return pixmap_status_ == PIXMAP_SUCCESS;
}


void xformsImage::clip(Params const & params)
{
	if (!image_)
		return;

	if (params.bb.empty())
		// No clipping is necessary.
		return;

	int const new_width  = params.bb.xr - params.bb.xl;
	int const new_height = params.bb.yt - params.bb.yb;

	// No need to check if the width, height are > 0 because the
	// Bounding Box would be empty() in this case.
	if (new_width > image_->w || new_height > image_->h)
		// Bounds are invalid.
		return;

	if (new_width == image_->w && new_height == image_->h)
		// Bounds are unchanged.
		return;

	int const xoffset_l = std::max(0, params.bb.xl);
	int const xoffset_r = std::max(0, image_->w - params.bb.xr);
	int const yoffset_t = std::max(0, image_->h - params.bb.yt);
	int const yoffset_b = std::max(0, params.bb.yb);

	flimage_crop(image_, xoffset_l, yoffset_t, xoffset_r, yoffset_b);
}


void xformsImage::rotate(Params const & params)
{
	if (!image_)
		return ;

	if (!params.angle)
		// No rotation is necessary.
		return;

	// The angle passed to flimage_rotate is the angle in one-tenth of a
	// degree units.

	// Work around xforms bug when params.angle == 270
	// the 'InternalError: bad special angle' error.
	// This bug fix is not needed in xforms 1.0 and greater.
	if (params.angle == 270) {
		flimage_rotate(image_,  900, FLIMAGE_SUBPIXEL);
		flimage_rotate(image_, 1800, FLIMAGE_SUBPIXEL);
	} else {
		flimage_rotate(image_, params.angle * 10, FLIMAGE_SUBPIXEL);
	}
}


void xformsImage::scale(Params const & params)
{
	if (!image_)
		return;

	unsigned int width;
	unsigned int height;
	boost::tie(width, height) = getScaledDimensions(params);

	if (width == getWidth() && height == getHeight())
		// No scaling needed
		return;

	flimage_scale(image_, width, height, FLIMAGE_SUBPIXEL);
}


void xformsImage::statusCB(string const & status_message)
{
	if (status_message.empty())
		return;

	if (prefixIs(status_message, "Done Reading")) {
		if (image_) {
			flimage_close(image_);
		}

		finishedLoading(true);
	}
}


void xformsImage::errorCB(string const & error_message)
{
	if (error_message.empty())
		return;

	if (image_) {
		flimage_close(image_);
	}

	finishedLoading(false);
}

} // namespace grfx


namespace {

extern "C" {

int status_report(FL_IMAGE * ob, const char *s)
{
	lyx::Assert(ob && ob->u_vdata);

	string const str = s ? strip(s) : string();
	if (str.empty())
		return 0;

	lyxerr[Debug::GRAPHICS]
		<< "xforms image loader. Status : " << str << std::endl;

	grfx::xformsImage * ptr =
		static_cast<grfx::xformsImage *>(ob->u_vdata);
	ptr->statusCB(str);

	return 0;
}


static void error_report(FL_IMAGE * ob, const char *s)
{
	lyx::Assert(ob && ob->u_vdata);

	string const str = s ? strip(s) : string();
	if (str.empty())
		return;

	lyxerr[Debug::GRAPHICS]
		<< "xforms image loader. Error : " << str << std::endl;

	grfx::xformsImage * ptr =
		static_cast<grfx::xformsImage *>(ob->u_vdata);
	ptr->errorCB(str);
}

} // extern "C"


void init_graphics()
{
	// Paranoia check
	static bool initialised = false;
	if (initialised)
		return;
	initialised = true;

	flimage_enable_bmp();
	flimage_enable_fits();
	flimage_enable_gif();
#ifdef HAVE_FLIMAGE_ENABLE_JPEG
	flimage_enable_jpeg();
#endif

	// xforms itself uses pngtopnm to convert to a loadable format.
	// We prefer to use our own conversion mechanism, therefore.
	// flimage_enable_png();

	flimage_enable_pnm();

#ifdef HAVE_FLIMAGE_ENABLE_PS
	// xforms recognises PS but not EPS
	flimage_enable_ps();
#endif

	flimage_enable_sgi();
	flimage_enable_tiff();
	flimage_enable_xbm();
	flimage_enable_xwd();
	flimage_enable_xpm();

	// xforms stores this permanently (does not make a copy) so
	// this should never be destroyed.
	static FLIMAGE_SETUP setup;
	setup.visual_cue    = status_report;
	setup.error_message = error_report;
	flimage_setup(&setup);
}


unsigned int packedcolor(LColor::color c)
{
	string const x11color = lcolor.getX11Name(c);

	Display * display = fl_get_display();
	Colormap cmap     = fl_state[fl_get_vclass()].colormap;
	XColor xcol;
	XColor ccol;
	if (XLookupColor(display, cmap, x11color.c_str(), &xcol, &ccol) == 0)
		// Unable to parse x11color.
		return FL_PACK(255,255,255);

	// Note that X stores the RGB values in the range 0 - 65535
	// whilst we require them in the range 0 - 255.
	unsigned int const r = xcol.red   / 256;
	unsigned int const g = xcol.green / 256;
	unsigned int const b = xcol.blue  / 256;

	return FL_PACK(r, g, b);
}

} // namespace anon
