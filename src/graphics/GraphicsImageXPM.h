// -*- C++ -*-
/**
 *  \file GraphicsImageXPM.h
 *  Copyright 2002 the LyX Team
 *  Read the file COPYING
 *
 *  \author Baruch Even <baruch.even@writeme.com>
 *  \author Angus Leeming <leeming@lyx.org>
 *
 *  An instantiation of Image that makes use of libXPM to load and store
 *  the image in memory.
 */

#ifndef GRAPHICSIMAGEXPM_H
#define GRAPHICSIMAGEXPM_H

#include "GraphicsImage.h"
#include XPM_H_LOCATION
#include "support/smart_ptr.h"

#ifdef __GNUG__
#pragma interface
#endif

namespace grfx {

class ImageXPM : public Image
{
public:
	/// Access to this class is through this static method.
	static ImagePtr newImage();

	/// Return the list of loadable formats.
	static FormatList loadableFormats();

	///
	~ImageXPM();

	/// Create a copy
	Image * clone() const;

	///
	Pixmap getPixmap() const;

	/// Get the image width
	unsigned int getWidth() const;

	/// Get the image height
	unsigned int getHeight() const;

	bool isDrawable() const;

	/** Load the image file into memory.
	 *  In this case (ImageXPM), the process is blocking.
	 */
	void load(string const & filename);

	/** Generate the pixmap, based on the current state of the
	 *  xpm_image_ (clipped, rotated, scaled etc).
	 *  Uses the params to decide on color, grayscale etc.
	 *  Returns true if the pixmap is created.
	 */
	bool setPixmap(Params const & params);

	/// Clip the image using params.
	void clip(Params const & params);

	/// Rotate the image using params.
	void rotate(Params const & params);

	/// Scale the image using params.
	void scale(Params const & params);

private:
	/// Access to the class is through newImage() and clone.
	ImageXPM();
	///
	ImageXPM(ImageXPM const &);

	/** Contains the data read from file.
	 *  This class is a wrapper for a XpmImage struct, but all views
	 *  of a single file's data will share the same color table.
	 *  This is done by ensuring that the color table contains a "none"
	 *  c_color together with g_color and m_color entries for each c_color
	 *  entry when it is first stored.
	 */
	class Data
	{
	public:
		/// Default c-tor. Initialise everything to zero.
		Data();
		~Data();

		bool empty() const { return width_ == 0; }

		/** Wrap an XpmImage in a nice, clean C++ interface.
		 *  Empty the original XpmImage.
		 *  Does some analysis of the color table to ensure that
		 *  it is suitable for all future eventualities. (See above
		 *  description.)
		 */
		void reset(XpmImage & image);

		/// Reset the data struct with this data.
		void resetData(int width, int height, unsigned int * data);

		/** Returns a ptr to an initialised block of memory.
		 *  the data is initialised to the color "none" entry.
		 */
		unsigned int * initialisedData(int width, int height) const;

		/** Construct an XpmImage from the stored contents.
		 *  To pass to XpmCreatePixmapFromXpmImage.
		 *  Efficient, because we only copy the ptrs to the structs.
		 */
		XpmImage get() const;

		unsigned int width()   const { return width_; }
		unsigned int height()  const { return height_; }
		unsigned int cpp()     const { return cpp_; }
		unsigned int ncolors() const { return ncolors_; }
		unsigned int const * data() const
			{ return data_.get(); }
		XpmColor const * colorTable() const
			{ return colorTable_.get(); }

	private:
		unsigned int width_;
		unsigned int height_;
		unsigned int cpp_;
		unsigned int ncolors_;
		lyx::shared_c_ptr<unsigned int> data_;
		lyx::shared_c_ptr<XpmColor> colorTable_;

		unsigned int color_none_id() const;
	};

	Data image_;

	/// The pixmap itself.
	Pixmap pixmap_;

	/// Is the pixmap initialized?
	enum PixmapStatus {
		///
		PIXMAP_UNINITIALISED,
		///
		PIXMAP_FAILED,
		///
		PIXMAP_SUCCESS
	};

	PixmapStatus pixmap_status_;
};

} // namespace grfx

#endif // GRAPHICSIMAGEXPM_H
