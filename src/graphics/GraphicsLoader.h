// -*- C++ -*-
/**
 *  \file GraphicsLoader.h
 *  Copyright 2002 the LyX Team
 *  Read the file COPYING
 *
 *  \author Angus Leeming <leeming@lyx.org>
 *
 *  The public view of the graphics cache.
 *  * The user supplies an image file and the display parameters.
 *  * He can change the file or the display parameters through a reset() method.
 *  * He must start the loading process explicitly with startLoading().
 *  * He receives a statusChanged signal when the loading status changes.
 *  * When (status() == Ready), he uses image() to access the loaded image
 *    and passes it to the Painter.
 *
 *  What could be simpler?
 */

#ifndef GRAPHICSLOADER_H
#define GRAPHICSLOADER_H

#ifdef __GNUG__
#pragma interface
#endif

#include "GraphicsTypes.h"
#include "LString.h"

#include <boost/signals/signal0.hpp>
#include <memory>

namespace grfx {

class GParams;

/** One image, one instance of grfx::Loader, although the image can be
 *  changed.
 */
class Loader {
public:
	/// Must use the reset methods to make this instance usable.
	Loader();
	/// The image is not transformed, just displayed as-is.
	Loader(string const & file_with_path, DisplayType = ColorDisplay);
	/// The image is transformed before display.
	Loader(string const & file_with_path, GParams const &);

	/// The file can be changed, or the display params, or both.
	void reset(string const & file_with_path, DisplayType = ColorDisplay);
	///
	void reset(string const & file_with_path, GParams const &);
	///
	void reset(GParams const &);

	/// Returns the absolute path of the loaded (loading?) file.
	string const & filename() const;
	///
	bool empty() const { return filename().empty(); }

	/// We are explicit about when we begin the loading process.
	void startLoading();

	///  How far have we got in loading the image?
	ImageStatus status() const;

	/// This signal is emitted when the image loading status changes.
	boost::signal0<void> statusChanged;

	/** The loaded image with Pixmap set.
	 *  If the Pixmap is not yet set (see status() for why...), returns 0.
	 */
	GImage const * image() const;

private:
	/// Use the Pimpl idiom to hide the internals.
	class Impl;
	/// The pointer never changes although *pimpl_'s contents may.
	std::auto_ptr<Impl> const pimpl_;
};

} // namespace grfx

#endif // GRAPHICSLOADER_H
