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

#include <config.h>

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
	void imageConverted(int retval);

private:
	bool convertImage(string const & filename);
	void loadImage();

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
