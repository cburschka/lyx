/*
 * \file GraphicsImageXPM.C
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author Baruch Even <baruch.even@writeme.com>
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "GraphicsImageXPM.h"
#include "GraphicsParams.h"
#include "ColorHandler.h"
#include "debug.h"
#include "frontends/GUIRunTime.h" // x11Display, x11Screen
#include "support/filetools.h"    // IsFileReadable
#include "support/lstrings.h"
#include "Lsstream.h"
#include <iomanip>                // std::setfill, etc
#include <cmath>                  // cos, sin
#include <cstdlib>                // malloc, free

namespace grfx {

/// Access to this class is through this static method.
ImagePtr GImageXPM::newImage()
{
	ImagePtr ptr;
	ptr.reset(new GImageXPM());
	return ptr;
}


/// Return the list of loadable formats.
GImage::FormatList GImageXPM::loadableFormats()
{
	FormatList formats(1);
	formats[0] = "xpm";
	return formats;
}


GImageXPM::GImageXPM()
	: pixmap_(0),
	  pixmap_status_(PIXMAP_UNINITIALISED)
{}


GImageXPM::GImageXPM(GImageXPM const & other)
	: GImage(other),
	  image_(other.image_),
	  pixmap_(0),
	  pixmap_status_(PIXMAP_UNINITIALISED)
{}


GImageXPM::~GImageXPM()
{
	if (pixmap_)
		XFreePixmap(GUIRunTime::x11Display(), pixmap_);
}


GImage * GImageXPM::clone() const
{
	return new GImageXPM(*this);
}


unsigned int GImageXPM::getWidth() const
{
	return image_.width();
}


unsigned int GImageXPM::getHeight() const
{
	return image_.height();
}


Pixmap GImageXPM::getPixmap() const
{
	if (!pixmap_status_ == PIXMAP_SUCCESS)
		return 0;
	return pixmap_;
}


void GImageXPM::load(string const & filename, GImage::SignalTypePtr on_finish)
{
	if (filename.empty()) {
		on_finish->emit(false);
		return;
	}

	if (!image_.empty()) {
		lyxerr[Debug::GRAPHICS]
			<< "Image is loaded already!" << std::endl;
		on_finish->emit(false);
		return;
	}

	XpmImage * xpm_image = new XpmImage;

	int const success =
		XpmReadFileToXpmImage(const_cast<char *>(filename.c_str()),
				      xpm_image, 0);

	switch (success) {
	case XpmOpenFailed:
		lyxerr[Debug::GRAPHICS]
			<< "No XPM image file found." << std::endl;
		break;

	case XpmFileInvalid:
		lyxerr[Debug::GRAPHICS]
			<< "File format is invalid" << std::endl;
		break;

	case XpmNoMemory:
		lyxerr[Debug::GRAPHICS]
			<< "Insufficient memory to read in XPM file"
			<< std::endl;
		break;
	}

	if (success != XpmSuccess) {
		XpmFreeXpmImage(xpm_image);
		delete xpm_image;

		lyxerr[Debug::GRAPHICS]
			<< "Error reading XPM file '"
			<< XpmGetErrorString(success) << "'"
			<< std::endl;
	} else {
		image_.reset(*xpm_image);
	}

	on_finish->emit(success == XpmSuccess);
}


bool GImageXPM::setPixmap(GParams const & params)
{
	if (image_.empty() || params.display == GParams::NONE) {
		return false;
	}

	Display * display = GUIRunTime::x11Display();

	if (pixmap_ && pixmap_status_ == PIXMAP_SUCCESS)
		XFreePixmap(display, pixmap_);

	//(BE 2000-08-05)
	// This might be a dirty thing, but I dont know any other solution.
	Screen * screen = ScreenOfDisplay(display, GUIRunTime::x11Screen());

	Pixmap pixmap;
	Pixmap mask;

	XpmAttributes attrib;

	// Allow libXPM lots of leeway when trying to allocate colors.
	attrib.closeness = 10000;
	attrib.valuemask = XpmCloseness;

	// The XPM file format allows multiple pixel colours to be defined
	// as c_color, g_color or m_color.
	switch (params.display) {
	case GParams::MONOCHROME:
		attrib.color_key = XPM_MONO;
		break;
	case GParams::GRAYSCALE:
		attrib.color_key = XPM_GRAY;
		break;
	case GParams::COLOR:
	default: // NONE cannot happen!
		attrib.color_key = XPM_COLOR;
		break;
	}

	attrib.valuemask |= XpmColorKey;

	// Set the color "none" entry to the color of the background.
	XpmColorSymbol xpm_col;
	xpm_col.name = 0;
	xpm_col.value = "none";
	xpm_col.pixel = lyxColorHandler->colorPixel(LColor::graphicsbg);

	attrib.numsymbols = 1;
	attrib.colorsymbols = &xpm_col;
	attrib.valuemask |= XpmColorSymbols;

	// Load up the pixmap
	XpmImage xpm_image = image_.get();
	int const status =
		XpmCreatePixmapFromXpmImage(display,
					    XRootWindowOfScreen(screen),
					    &xpm_image,
					    &pixmap, &mask, &attrib);

	XpmFreeAttributes(&attrib);

	if (status != XpmSuccess) {
		lyxerr << "Error creating pixmap from xpm_image '"
		       << XpmGetErrorString(status) << "'"
		       << std::endl;
		pixmap_status_ = PIXMAP_FAILED;
		return false;
	}

	pixmap_ = pixmap;
	pixmap_status_ = PIXMAP_SUCCESS;
	return true;
}


void GImageXPM::clip(GParams const & params)
{
	if (image_.empty())
		return;

	if (params.bb.empty())
		// No clipping is necessary.
		return;

	typedef unsigned int dimension;

	dimension const new_width  = params.bb.xr - params.bb.xl;
	dimension const new_height = params.bb.yt - params.bb.yb;

	if (new_width > image_.width() || new_height > image_.height())
		// Bounds are invalid.
		return;

	if (new_width == image_.width() && new_height == image_.height())
		// Bounds are unchanged.
		return;

	dimension * new_data = image_.initialisedData(new_width, new_height);
	dimension const * old_data = image_.data();

	dimension * it = new_data;
	dimension const * start_row = old_data;
	for (size_t row = params.bb.yb; row < params.bb.yt; ++row) {
		dimension const * begin = start_row + params.bb.xl;
		dimension const * end   = start_row + params.bb.xr;
		it = std::copy(begin, end, it);
		start_row += image_.width();
	}

	image_.resetData(new_width, new_height, new_data);
}


void GImageXPM::rotate(GParams const & params)
{
	if (image_.empty())
		return ;

	if (!params.angle)
		// No rotation is necessary.
		return;

	// Ascertain the bounding box of the rotated image
	// Rotate about the bottom-left corner
	static double const pi = 3.14159265358979323846;
	double const angle = double(params.angle) * pi / 180.0;
	double const cos_a = cos(angle);
	double const sin_a = sin(angle);

	// (0, 0)
	double max_x = 0; double min_x = 0;
	double max_y = 0; double min_y = 0;

	// (old_xpm->width, 0)
	double x_rot = cos_a * image_.width();
	double y_rot = sin_a * image_.width();
	max_x = std::max(max_x, x_rot); min_x = std::min(min_x, x_rot);
	max_y = std::max(max_y, y_rot); min_y = std::min(min_y, y_rot);

	// (image_.width, image_.height)
	x_rot = cos_a * image_.width() - sin_a * image_.height();
	y_rot = sin_a * image_.width() + cos_a * image_.height();
	max_x = std::max(max_x, x_rot); min_x = std::min(min_x, x_rot);
	max_y = std::max(max_y, y_rot); min_y = std::min(min_y, y_rot);

	// (0, image_.height)
	x_rot = - sin_a * image_.height();
	y_rot =   cos_a * image_.height();
	max_x = std::max(max_x, x_rot); min_x = std::min(min_x, x_rot);
	max_y = std::max(max_y, y_rot); min_y = std::min(min_y, y_rot);

	typedef unsigned int dimension;

	dimension const new_width  = 1 + int(max_x - min_x); // round up!
	dimension const new_height = 1 + int(max_y - min_y);

	dimension * new_data = image_.initialisedData(new_width, new_height);
	dimension const * old_data = image_.data();

	// rotate the data
	for (dimension y_old = 0; y_old < image_.height(); ++y_old) {
		for (dimension x_old = 0; x_old < image_.width(); ++x_old) {
			double const x_pos = cos_a*x_old - sin_a*y_old - min_x;
			double const y_pos = sin_a*x_old + cos_a*y_old - min_y;

			// ensure that there are no rounding errors
			dimension x_new = (x_pos > 0) ? dimension(x_pos) : 0;
			dimension y_new = (y_pos > 0) ? dimension(y_pos) : 0;
			x_new = std::min(new_width  - 1, x_new);
			y_new = std::min(new_height - 1, y_new);

			size_t const id_old = x_old + image_.width() * y_old;
			size_t const id_new = x_new + new_width * y_new;

			new_data[id_new] = old_data[id_old];
		}
	}

	image_.resetData(new_width, new_height, new_data);
}


void GImageXPM::scale(GParams const & params)
{
	if (image_.empty())
		return;

	typedef unsigned int dimension;

	// boost::tie produces horrible compilation errors on my machine
	// Angus 25 Feb 2002
	std::pair<dimension, dimension> d = getScaledDimensions(params);
	dimension const new_width  = d.first;
	dimension const new_height = d.second;
	if (new_width == getWidth() && new_height == getHeight())
		// No scaling needed
		return;

	dimension * new_data = image_.initialisedData(new_width, new_height);
	dimension const * old_data = image_.data();

	double const x_scale = double(image_.width())  / double(new_width);
	double const y_scale = double(image_.height()) / double(new_height);

	// A very simple scaling routine.
	// Ascertain the old pixel corresponding to the new one.
	// There is no dithering at all here.
	for (dimension x_new = 0; x_new < new_width; ++x_new) {
		dimension x_old = dimension(x_new * x_scale);

		for (dimension y_new = 0; y_new < new_height; ++y_new) {
			dimension y_old = dimension(y_new * y_scale);

			size_t const id_old = x_old + image_.width() * y_old;
			size_t const id_new = x_new + new_width * y_new;

			new_data[id_new] = old_data[id_old];
		}
	}

	image_.resetData(new_width, new_height, new_data);
}

} // namespace grfx


namespace {

void free_color_table(XpmColor * colorTable, size_t size);

void copy_color_table(XpmColor const * in, size_t size, XpmColor * out);

bool contains_color_none(XpmImage const & image);

string const unique_color_string(XpmImage const & image);

// create a copy (using malloc and strcpy). If (!in) return 0;
char * clone_c_string(char const * in);

// Given a string of the form #ff0571 create appropriate grayscale and
// monochrome colors.
void mapcolor(char const * c_color, char ** g_color_ptr, char ** m_color_ptr);

} // namespace anon


namespace grfx {

GImageXPM::Data::Data()
	: width_(0), height_(0), cpp_(0), ncolors_(0)
{}


GImageXPM::Data::~Data()
{
	if (colorTable_.unique())
		free_color_table(colorTable_.get(), ncolors_);
}


void GImageXPM::Data::reset(XpmImage & image)
{
	width_ = image.width;
	height_ = image.height;
	cpp_ = image.cpp;

	// Move the data ptr into this store and free up image.data
	data_.reset(image.data);
	image.data = 0;

	// Don't just store the color table, but check first that it contains
	// all that we require of it.
	// The idea is to store the color table in a shared_ptr and for all
	// modified images to use the same table.
	// It must, therefore, have a c_color "none" entry and g_color and
	// m_color entries corresponding to each and every c_color entry
	// (except "none"!)

	// 1. Create a copy of the color table.
	// Add a c_color "none" entry to the table if it isn't already there.
	bool const add_color = !contains_color_none(image);

	if (add_color) {

		ncolors_ = 1 + image.ncolors;
		size_t const mem_size = sizeof(XpmColor) * ncolors_;
		XpmColor * table = static_cast<XpmColor *>(malloc(mem_size));

		copy_color_table(image.colorTable, image.ncolors, table);

		XpmColor & color = table[ncolors_ - 1];
		color.symbolic = 0;
		color.m_color  = 0;
		color.g_color  = 0;
		color.g4_color = 0;
		color.string =
			clone_c_string(unique_color_string(image).c_str());
		color.c_color = clone_c_string("none");

		free_color_table(image.colorTable, image.ncolors);
		colorTable_.reset(table);

	} else {

		// Just move the pointer across
		ncolors_ = image.ncolors;
		colorTable_.reset(image.colorTable);
		image.colorTable = 0;
	}

	// Clean-up the remaining entries of image.
	image.width = 0;
	image.height = 0;
	image.cpp = 0;
	image.ncolors = 0;

	// 2. Ensure that the color table has g_color and m_color entries
	XpmColor * table = colorTable_.get();
	string buggy_color;

	for (size_t i = 0; i < ncolors_; ++i) {
		XpmColor & entry = table[i];
		if (!entry.c_color)
			continue;

		// A work-around for buggy XPM files that may be created by
		// ImageMagick's convert.
		string c_color = entry.c_color;
		if (c_color[0] == '#' && c_color.size() > 7) {
			if (buggy_color.empty())
				buggy_color = c_color;

			c_color = c_color.substr(0, 7);
			free(entry.c_color);
			entry.c_color = clone_c_string(c_color.c_str());
		}

		// If the c_color is defined and the equivalent
		// grayscale or monochrome ones are not, then define them.
		mapcolor(entry.c_color, &entry.g_color, &entry.m_color);
	}

	if (!buggy_color.empty()) {
		lyxerr << "The XPM file contains silly colors, "
		       << "an example being \""
		       << buggy_color << "\".\n"
		       << "This was cropped to \""
		       << buggy_color.substr(0, 7)
		       << "\" so you can see something!\n"
		       << "If this file was created by ImageMagick's convert,\n"
		       << "then upgrading may cure the problem."
		       << std::endl;
	}
}


XpmImage GImageXPM::Data::get() const
{
	XpmImage image;
	image.width = width_;
	image.height = height_;
	image.cpp = cpp_;
	image.ncolors = ncolors_;
	image.data = data_.get();
	image.colorTable = colorTable_.get();
	return image;
}


void GImageXPM::Data::resetData(int w, int h, unsigned int * d)
{
	width_  = w;
	height_ = h;
	data_.reset(d);
}

unsigned int * GImageXPM::Data::initialisedData(int w, int h) const
{
	size_t const data_size = w * h;

	size_t const mem_size  = sizeof(unsigned int) * data_size;
	unsigned int * ptr = static_cast<unsigned int *>(malloc(mem_size));

	unsigned int none_id = color_none_id();
	std::fill(ptr, ptr + data_size, none_id);

	return ptr;
}


unsigned int GImageXPM::Data::color_none_id() const
{
	XpmColor * table = colorTable_.get();
	for (size_t i = 0; i < ncolors_; ++i) {
		char const * const color = table[i].c_color;
		if (color && lowercase(color) == "none")
			return uint(i);
	}
	return 0;
}

} // namespace grfx

namespace {

// Given a string of the form #ff0571 create appropriate grayscale and
// monochrome colors.
void mapcolor(char const * c_color, char ** g_color_ptr, char ** m_color_ptr)
{
	if (!c_color)
		return;

	char * g_color = *g_color_ptr;
	char * m_color = *m_color_ptr;

	if (g_color && m_color)
		// Already filled.
		return;

	Display * display = GUIRunTime::x11Display();
	Colormap cmap     = GUIRunTime::x11Colormap();
	XColor xcol;
	XColor ccol;
	if (XLookupColor(display, cmap, c_color, &xcol, &ccol) == 0)
		// Unable to parse c_color.
		return;

	// Note that X stores the RGB values in the range 0 - 65535
	// whilst we require them in the range 0 - 255.
	int const r = xcol.red   / 256;
	int const g = xcol.green / 256;
	int const b = xcol.blue  / 256;

	// This gives a good match to a human's RGB to luminance conversion.
	// (From xv's Postscript code --- Mike Ressler.)
	int const gray = int((0.32 * r) + (0.5 * g) + (0.18 * b));

	ostringstream gray_stream;
	gray_stream << "#" << std::setbase(16) << std::setfill('0')
		    << std::setw(2) << gray
		    << std::setw(2) << gray
		    << std::setw(2) << gray;

	int const mono = (gray < 128) ? 0 : 255;
	ostringstream mono_stream;
	mono_stream << "#" << std::setbase(16) << std::setfill('0')
		    << std::setw(2) << mono
		    << std::setw(2) << mono
		    << std::setw(2) << mono;

	// This string is going into an XpmImage struct, so create copies that
	// libXPM can free successfully.
	if (!g_color)
		*g_color_ptr = clone_c_string(gray_stream.str().c_str());
	if (!m_color)
		*m_color_ptr = clone_c_string(mono_stream.str().c_str());
}


void copy_color_table(XpmColor const * in, size_t size, XpmColor * out)
{
	for (size_t i = 0; i < size; ++i) {
		out[i].string   = clone_c_string(in[i].string);
		out[i].symbolic = clone_c_string(in[i].symbolic);
		out[i].m_color  = clone_c_string(in[i].m_color);
		out[i].g_color  = clone_c_string(in[i].g_color);
		out[i].g4_color = clone_c_string(in[i].g4_color);
		out[i].c_color  = clone_c_string(in[i].c_color);
	}
}


void free_color_table(XpmColor * table, size_t size)
{
	for (size_t i = 0; i < size; ++i) {
		free(table[i].string);
		free(table[i].symbolic);
		free(table[i].m_color);
		free(table[i].g_color);
		free(table[i].g4_color);
		free(table[i].c_color);
	}
	// Don't free the table itself. Let the shared_c_ptr do that.
	// free(table);
}


char * clone_c_string(char const * in)
{
	if (!in)
		return 0;

	// Don't forget the '\0'
	char * out = static_cast<char *>(malloc(strlen(in) + 1));
	return strcpy(out, in);
}


bool contains_color_none(XpmImage const & image)
{
	for (size_t i = 0; i < image.ncolors; ++i) {
		char const * const color = image.colorTable[i].c_color;
		if (color && lowercase(color) == "none")
			return true;
	}
	return false;
}


string const unique_color_string(XpmImage const & image)
{
	string id(image.cpp, ' ');

	for(;;) {
		bool found_it = false;
		for (size_t i = 0; i < image.ncolors; ++i) {
			string const c_id = image.colorTable[i].string;
			if (c_id == id) {
				found_it = true;
				break;
			}
		}

		if (!found_it)
			return id;

		// Loop over the printable characters in the ASCII table.
		// Ie, count from char 32 (' ') to char 126 ('~')
		// A base 94 counter!
		string::size_type current_index = id.size() - 1;
		bool continue_loop = true;
		while(continue_loop) {
			continue_loop = false;

			if (id[current_index] == 126) {
				continue_loop = true;
				if (current_index == 0)
					// Unable to find a unique string
					return image.colorTable[0].string;

				id[current_index] = 32;
				current_index -= 1;
			} else {
				id[current_index] += 1;
				// Note that '"' is an illegal char in this
				// context
				if (id[current_index] == '"')
					id[current_index] += 1;
			}
		}
		if (continue_loop)
			// Unable to find a unique string
			return string();
	}
}

} // namespace anon
