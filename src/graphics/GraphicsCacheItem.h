// -*- C++ -*-
/**
 * \file GraphicsCacheItem.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Baruch Even 
 * \author Angus Leeming 
 *
 * Full author contact details are available in file CREDITS
 *
 * The graphics cache is a container of grfx::CacheItems.
 * Each grfx::CacheItem, defined here represents a separate image file.
 *
 * The routines here can be used to load the graphics file into memory at
 * which point (status() == grfx::Loaded).
 * The user is then free to access image() in order to copy it and to then
 * transform the copy (rotate, scale, clip) and to generate the pixmap.
 *
 * The graphics cache supports fully asynchronous:
 * file conversion to a loadable format;
 * file loading.
 *
 * Whether you get that, of course, depends on grfx::Converter and on the
 * grfx::Image-derived image class.
 */

#ifndef GRAPHICSCACHEITEM_H
#define GRAPHICSCACHEITEM_H

#ifdef __GNUG__
#pragma interface
#endif

#include "GraphicsTypes.h"
#include "LString.h"

#include <boost/utility.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/signals/signal0.hpp>

class InsetGraphics;

namespace grfx {

class Image;
class Converter;

/// A grfx::Cache item holder.
class CacheItem : boost::noncopyable {
public:
	///
	CacheItem(string const & file);

	/// Define an empty d-tor out-of-line to keep boost::scoped_ptr happy.
	~CacheItem();

	///
	string const & filename() const;

	/// It's in the cache. Now start the loading process.
	void startLoading() const;

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

	/** Get the image associated with filename().
	 *  If the image is not yet loaded, returns 0.
	 *  This routine returns a pointer to const; if you want to modify it,
	 *  create a copy and modify that.
	 */
	Image const * image() const;

	/// How far have we got in loading the image?
	ImageStatus status() const;

	/** Connect and you'll be informed when the loading status of the image
	 *  changes.
	 */
	typedef boost::signal0<void>::slot_type slot_type;
	///
	boost::signals::connection connect(slot_type const &) const;

private:
	/// Use the Pimpl idiom to hide the internals.
	class Impl;

	/// The pointer never changes although *pimpl_'s contents may.
	boost::scoped_ptr<Impl> const pimpl_;
};

} // namespace grfx

#endif // GRAPHICSCACHEITEM_H
