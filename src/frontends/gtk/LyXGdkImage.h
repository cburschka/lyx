// -*- C++ -*-
/**
 * \file GdkImage.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming (original Qt version)
 * \author John Levon (original Qt version)
 * \author John Spray
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYXGDKIMAGE_H
#define LYXGDKIMAGE_H


#include "graphics/GraphicsImage.h"

#include <gdkmm.h>

namespace lyx {
namespace graphics {

class LyXGdkImage : public Image {
public:
	/// Access to this class is through this static method.
	static ImagePtr newImage();

	/// Return the list of loadable formats.
	static FormatList loadableFormats();

	/// Retrieve the buffered pixmap.
	Glib::RefPtr<Gdk::Pixbuf> const & pixbuf() const {return transformed_;}

private:
	/// Create a copy
	virtual Image * clone_impl() const;
	/// Get the image width
	virtual unsigned int getWidth_impl() const;
	/// Get the image height
	virtual unsigned int getHeight_impl() const;
	// FIXME Is the image drawable ?
	virtual bool isDrawable_impl() const { return true; }
	/**
	 * Load the image file into memory.
	 * The process is asynchronous, so this method starts the loading.
	 * When finished, the Image::finishedLoading signal is emitted.
	 */
	virtual void load_impl(std::string const & filename);
	/**
	 * Finishes the process of modifying transformed_, using
	 * \c params to decide on color, grayscale etc.
	 * \returns true if successful.
	 */
	virtual bool setPixmap_impl(Params const & params);
	/// Clip the image using params.
	virtual void clip_impl(Params const & params);
	/// Rotate the image using params.
	virtual void rotate_impl(Params const & params);
	/// Scale the image using params.
	virtual void scale_impl(Params const & params);

	/// Access to the class is through newImage() and clone.
	LyXGdkImage();
	///
	LyXGdkImage(LyXGdkImage const &);

	Glib::RefPtr<Gdk::Pixbuf> original_;
	Glib::RefPtr<Gdk::Pixbuf> transformed_;
};

} // namespace graphics
} // namespace lyx

#endif // LYXGDKIMAGE_H
