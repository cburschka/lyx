// -*- C++ -*-
/**
 * \file GraphicsImage.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Baruch Even
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 *
 * An abstract base class for the images themselves.
 * Allows the user to retrieve the pixmap, once loaded and to issue commands
 * to modify it.
 *
 * The boost::functions newImage and loadableFormats are connected to the
 * appropriate derived classes elsewhere, allowing the graphics cache to
 * access them without knowing anything about their instantiation.
 *
 * The loading process can be asynchronous, but cropping, rotating and
 * scaling block execution.
 */

#ifndef GRAPHICSIMAGE_H
#define GRAPHICSIMAGE_H

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/signal.hpp>

#include <vector>
#include <utility>

namespace lyx {
namespace graphics {

class Params;

class Image {
public:
	/** This is to be connected to a function that will return a new
	 *  instance of a viable derived class.
	 */
	typedef boost::shared_ptr<Image> ImagePtr;
	///
	static boost::function<ImagePtr()> newImage;

	/// Return the list of loadable formats.
	typedef std::vector<std::string> FormatList;
	///
	static boost::function<FormatList()> loadableFormats;

	///
	virtual ~Image() {}

	/// Create a copy
	Image * clone() const;

	/// Get the image width
	unsigned int getWidth() const;

	/// Get the image height
	unsigned int getHeight() const;

	/// Is the image drawable ?
	bool isDrawable() const;

	/** At the end of the loading process inform the outside world
	 *  by emitting a signal
	 */
	typedef boost::signal<void(bool)> SignalType;
	///
	SignalType finishedLoading;

	/** Start loading the image file.
	 *  The caller should expect this process to be asynchronous and
	 *  so should connect to the "finished" signal above.
	 */
	void load(std::string const & filename);

	/** Generate the pixmap.
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

protected:
	/// Must define default c-tor explicitly as we define a copy c-tor.
	Image() {}
	/// Don't copy the signal finishedLoading
	Image(Image const &) {}

	/** Uses the params to ascertain the dimensions of the scaled image.
	 *  Returned as make_pair(width, height).
	 *  If something goes wrong, returns make_pair(getWidth(), getHeight())
	 */
	std::pair<unsigned int, unsigned int>
	getScaledDimensions(Params const & params) const;

private:
	/// Create a copy
	virtual Image * clone_impl() const = 0;
	/// Get the image width
	virtual unsigned int getWidth_impl() const = 0;

	/// Get the image height
	virtual unsigned int getHeight_impl() const = 0;

	/// is the image drawable ?
	virtual bool isDrawable_impl() const = 0;

	/** Start loading the image file.
	 *  The caller should expect this process to be asynchronous and
	 *  so should connect to the "finished" signal above.
	 */
	virtual void load_impl(std::string const & filename) = 0;

	/** Generate the pixmap.
	 *  Uses the params to decide on color, grayscale etc.
	 *  Returns true if the pixmap is created.
	 */
	virtual bool setPixmap_impl(Params const & params) = 0;

	/// Clip the image using params.
	virtual void clip_impl(Params const & params) = 0;

	/// Rotate the image using params.
	virtual void rotate_impl(Params const & params) = 0;

	/// Scale the image using params.
	virtual void scale_impl(Params const & params) = 0;
};


inline
Image * Image::clone() const
{
	return clone_impl();
}


inline
unsigned int Image::getWidth() const
{
	return getWidth_impl();
}


inline
unsigned int Image::getHeight() const
{
	return getHeight_impl();
}


inline
bool Image::isDrawable() const
{
	return isDrawable_impl();
}


inline
void Image::load(std::string const & filename)
{
	return load_impl(filename);
}


inline
bool Image::setPixmap(Params const & params)
{
	return setPixmap_impl(params);
}


inline
void Image::clip(Params const & params)
{
	return clip_impl(params);
}


inline
void Image::rotate(Params const & params)
{
	return rotate_impl(params);
}


inline
void Image::scale(Params const & params)
{
	return scale_impl(params);
}

} // namespace graphics
} // namespace lyx

#endif // GRAPHICSIMAGE_H
