// -*- C++ -*-
/* This file is part of
 * =================================================
 * 
 *          LyX, The Document Processor
 *          Copyright 1995 Matthias Ettrich.
 *          Copyright 1995-2001 The LyX Team.
 *
 *          This file Copyright 2000 Baruch Even
 * ================================================= */

#ifndef GRAPHICSCACHEITEM_H
#define GRAPHICSCACHEITEM_H

#ifdef __GNUG__
#pragma interface
#endif

#include XPM_H_LOCATION
#include "LString.h"

#include <boost/utility.hpp>
#include <boost/smart_ptr.hpp>

#include <sigc++/signal_system.h>

class LyXImage;

/// A GraphicsCache item holder.
class GraphicsCacheItem : boost::noncopyable {
public:
	/// c-tor
	GraphicsCacheItem(string const & filename);
	/// d-tor, frees the image structures.
	~GraphicsCacheItem();
	
	/// Return a pixmap that can be displayed on X server.
	LyXImage * getImage() const; 
	///
	enum ImageStatus {
		///
		Loading = 1,
		///
		ErrorConverting,
		///
		ErrorReading,
		///
		UnknownError,
		///
		Loaded
	};
	
	/// Is the pixmap ready for display?
	ImageStatus getImageStatus() const; 

	/** Get a notification when the image conversion is done.
	    used by an internal callback mechanism.
	*/
	void imageConverted(bool success);

private:
	/** Start image conversion process, checks first that it is necessary
	 *  if necessary will start an (a)synchronous task and notify upon
	 *  completion by calling imageConverted(bool) where true is for success
	 *  and false is for a failure.
	 *
	 *  Returns a bool to denote success or failure of starting the conversion
	 *  task.
	 */
	bool convertImage(string const & filename);

	/// Load the image into memory, this gets called from imageConverted(bool).
	void loadImage();

	/// Sets the status of the image, in the future will also notify listeners
	/// that the status is updated.
	void setStatus(ImageStatus new_status);

	/** The filename we refer too.
	    This is used when removing ourselves from the cache.
	*/
	string filename_;
	/// The temporary file that we use
	string tempfile;
	/// The image status
	ImageStatus imageStatus_;
	/// The image (if it got loaded)
	boost::scoped_ptr<LyXImage> image_;
};

#endif
