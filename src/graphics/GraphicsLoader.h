// -*- C++ -*-
/**
 *  \file GraphicsLoader.h
 *  Copyright 2002 the LyX Team
 *  Read the file COPYING
 *
 *  \author Angus Leeming <leeming@lyx.org>
 *
 *  The public face of the graphics cache.
 *
 *  * The user supplies an image file and the display parameters.
 *  * He can change the file or the display parameters through a reset() method.
 *  * He must start the loading process explicitly with startLoading().
 *  * If he is connected through the connect() method, then he'll be informed
 *    when the loading status changes.
 *  * When (status() == Ready), he can use image() to access the loaded image
 *    and pass it to the Painter.
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
#include <boost/scoped_ptr.hpp>

class Inset;
class BufferView;

namespace grfx {

class Image;
class Params;

class Loader {
public:
	/// Must use the reset methods to make this instance usable.
	Loader();
	/// The image is not transformed, just displayed as-is.
	Loader(string const & file_with_path, DisplayType = ColorDisplay);
	/// The image is transformed before display.
	Loader(string const & file_with_path, Params const &);

	/// Define an empty d-tor out-of-line to keep boost::scoped_ptr happy.
	~Loader();

	/// The file can be changed, or the display params, or both.
	void reset(string const & file_with_path,
		   DisplayType = ColorDisplay) const;
	///
	void reset(string const & file_with_path, Params const &) const;
	///
	void reset(Params const &) const;

	/// Returns the absolute path of the loaded (loading?) file.
	string const & filename() const;
	///
	bool empty() const { return filename().empty(); }

	/// We are explicit about when we begin the loading process.
	void startLoading() const;

	/** starting loading of the image is conditional upon the
	 *  inset being visible or not.
	 */
	void startLoading(Inset const &, BufferView const &) const;

	/** Monitor any changes to the file.
	 *  There is no point monitoring the file before startLoading() is
	 *  invoked.
	 */
	void startMonitoring() const;
	///
	bool monitoring() const;
	/** Returns the check sum of filename() so that, for example, you can
	 *  ascertain whether to output a new PostScript version of the file
	 *  for a LaTeX run.
	 */
	unsigned long checksum() const;

	/// How far have we got in loading the image?
	ImageStatus status() const;

	/** Connect and you'll be informed when the loading status of the image
	 *  changes.
	 */
	typedef boost::signal0<void>::slot_type slot_type;
	///
	boost::signals::connection connect(slot_type const &) const;

	/** The loaded image with Pixmap set.
	 *  If the Pixmap is not yet set (see status() for why...), returns 0.
	 */
	Image const * image() const;

private:
	/// Use the Pimpl idiom to hide the internals.
	class Impl;
	/// The pointer never changes although *pimpl_'s contents may.
	boost::scoped_ptr<Impl> const pimpl_;
};

} // namespace grfx

#endif // GRAPHICSLOADER_H
