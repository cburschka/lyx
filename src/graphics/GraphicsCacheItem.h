// -*- C++ -*-
/*
 * \file GraphicsCacheItem.h
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author Baruch Even <baruch.even@writeme.com>
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 *
 * The graphics cache is a container of GCacheItems. Each GCacheItem, defined
 * here represents a separate image file. However, each file can be viewed in
 * different ways (different sizes, rotations etc), so each GCacheItem itself
 * contains a list of ModifiedItems, also defined here. Each ModifiedItem
 * has a GParams variable that defines the way it will be viewed. It also
 * contains a list of the graphics insets that refer to it, so calls through
 * the GCache to GCacheItem ultimately return the loading status and image
 * for that particular graphics inset.
 *
 * The graphics cache supports fully asynchronous:
 * file conversion to a loadable format;
 * file loading.
 *
 * Whether you get that, of course, depends on grfx::GConverter and on the
 * grfx::GImage-derived image class.
 *
 * Image modification (scaling, rotation etc) is blocking.
 */

#ifndef GRAPHICSCACHEITEM_H
#define GRAPHICSCACHEITEM_H

#ifdef __GNUG__
#pragma interface
#endif

#include "GraphicsTypes.h"
#include "LString.h"

#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

#include <boost/signals/signal0.hpp>
#include <boost/signals/signal1.hpp>
#include <boost/signals/connection.hpp>
#include <boost/signals/trackable.hpp>

class InsetGraphics;

namespace grfx {

/// A grfx::GCache item holder.
class GCacheItem : boost::noncopyable, public boost::signals::trackable {
public:
	///
	GCacheItem(string const & file);

	/// It's in the cache. Now start the loading process.
	void startLoading();

	/** Get the image associated with filename_.
	    If the image is not yet loaded, return a null pointer.
	 */
	ImagePtr const image() const { return image_; }

	/// How far have we got in loading the image?
	ImageStatus status() const { return status_; }

	/// This signal is emitted when the image loading status changes.
	boost::signal0<void> statusChanged;

	///
	string const & filename() const { return filename_; }

private:
	/** Start the image conversion process, checking first that it is
	 *  necessary. If it is necessary, then a conversion task is started.
	 *  GCacheItem asumes that the conversion is asynchronous and so
	 *  passes a Signal to the converting routine. When the conversion
	 *  is finished, this Signal is emitted, returning the converted
	 *  file to this->imageConverted.
	 *
	 *  If no file conversion is needed, then convertToDisplayFormat() calls
	 *  loadImage() directly.
	 *
	 *  convertToDisplayFormat() will set the loading status flag as
	 *  approriate through calls to setStatus().
	 */
	void convertToDisplayFormat();

	/** Load the image into memory. This is called either from
	 *  convertToDisplayFormat() direct or from imageConverted().
	 */
	void loadImage();

	/** Get a notification when the image conversion is done.
	 *  Connected to a signal on_finish_ which is passed to
	 *  GConverter::convert.
	 */
	void imageConverted(string const & file_to_load);

	/** Get a notification when the image loading is done.
	 *  Connected to a signal on_finish_ which is passed to
	 *  GImage::loadImage.
	 */
	void imageLoaded(bool);

	/** Sets the status of the loading process. Also notifies
	 *  listeners that the status has chacnged.
	 */
	void setStatus(ImageStatus new_status);

	/// The filename we refer too.
	string filename_;
	/// Is the file compressed?
	bool zipped_;
	/// If so, store the uncompressed file in this temporary file.
	string unzipped_filename_;
	/// What file are we trying to load?
	string file_to_load_;
	/** Should we delete the file after loading? True if the file is
	 *  the result of a conversion process.
	 */
	bool remove_loaded_file_;

	/// The image and its loading status.
	ImagePtr image_;
	///
	ImageStatus status_;

	/** A SignalLoadTypePtr is connected to this->imageLoaded and
	 *  then passed to ImagePtr::load.
	 *  When the image has been loaded, the signal is emitted.
	 *
	 *  We pass a shared_ptr because it is eminently possible for the
	 *  GCacheItem to be destructed before the loading is complete and
	 *  the signal must remain in scope. It doesn't matter if the slot
	 *  disappears, SigC takes care of that.
	 */
	typedef boost::signal1<void, bool> SignalLoadType;
	///
	typedef boost::shared_ptr<SignalLoadType> SignalLoadTypePtr;

	/// The connection of the signal passed to ImagePtr::loadImage.
	boost::signals::connection cl_;

	/** A SignalConvertTypePtr is connected to this->imageConverted and
	 *  then passed to GConverter::convert.
	 *  When the image has been converted to a loadable format, the signal
	 *  is emitted, returning the name of the loadable file to
	 *  imageConverted.
	 */
	typedef boost::signal1<void, string const &> SignalConvertType;
	///
	typedef boost::shared_ptr<SignalConvertType> SignalConvertTypePtr;

	/// The connection of the signal passed to GConverter::convert.
	boost::signals::connection cc_;
};

} // namespace grfx

#endif // GRAPHICSCACHEITEM_H
