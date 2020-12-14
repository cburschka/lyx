// -*- C++ -*-
/**
 * \file GraphicsLoader.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 *
 * The public face of the graphics cache.
 *
 *  * The user supplies an image file and the display parameters.
 *  * He can change the file or the display parameters through a reset() method.
 *  * He must start the loading process explicitly with startLoading().
 *  * If he is connected through the connect() method, then he'll be informed
 *    when the loading status changes.
 *  * When (status() == Ready), he can use image() to access the loaded image
 *    and pass it to the Painter.
 *
 * What could be simpler?
 */

#ifndef GRAPHICSLOADER_H
#define GRAPHICSLOADER_H

#include "GraphicsTypes.h"

#include "support/signals.h"

namespace lyx {

namespace support { class FileName; }

namespace graphics {

class Image;
class Params;

class Loader {
public:
	/// Must use the reset methods to make this instance usable.
	Loader(support::FileName const & doc_file);
	/// The image is not transformed, just displayed as-is.
	Loader(support::FileName const & doc_file, support::FileName const & file_with_path, bool display = true);
	/// The image is transformed before display.
	Loader(support::FileName const & doc_file, support::FileName const & file_with_path, Params const &);
	///
	Loader(support::FileName const & doc_file, Loader const &);
	///
	Loader(Loader const & other);
	/// Needed for the pimpl
	~Loader();

	Loader & operator=(Loader const &);

	/// The file can be changed, or the display params, or both.
	void reset(support::FileName const & file_with_path, bool display = true) const;
	///
	void reset(support::FileName const & file_with_path, Params const &) const;
	///
	void reset(Params const &) const;

	/// Returns the absolute path of the loaded (loading?) file.
	support::FileName const & filename() const;
	///

	/** starting loading of the image is done by a urgency-based
	 *  decision. Here we only call LoaderQueue::touch to request it.
	 */
	void startLoading() const;

	/** Tries to reload the image.
	 */
	void reload() const;

	/** Monitor any changes to the file.
	 *  There is no point monitoring the file before startLoading() is
	 *  invoked.
	 */
	void startMonitoring() const;
	///
	bool monitoring() const;
	///
	void checkModifiedAsync() const;

	/// How far have we got in loading the image?
	ImageStatus status() const;

	/** Connect and you'll be informed when the loading status of the image
	 *  changes.
	 */
	typedef signal<void()> sig;
	typedef sig::slot_type slot;
	///
	connection connect(slot const &) const;

	/** The loaded image with Pixmap set.
	 *  If the Pixmap is not yet set (see status() for why...), returns 0.
	 */
	Image const * image() const;

	double displayPixelRatio() const;
	void setDisplayPixelRatio(double scale);

private:
	/// Use the Pimpl idiom to hide the internals.
	class Impl;
	/// The pointer never changes although *pimpl_'s contents may.
	Impl * pimpl_;
};

} // namespace graphics
} // namespace lyx

#endif // GRAPHICSLOADER_H
