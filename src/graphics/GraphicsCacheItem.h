// -*- C++ -*-
/* This file is part of
 * =================================================
 * 
 *          LyX, The Document Processor
 *          Copyright 1995 Matthias Ettrich.
 *          Copyright 1995-2000 The LyX Team.
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
#include "graphics/Renderer.h"
#include "support/syscall.h"

#include "sigc++/signal_system.h"
#ifdef SIGC_CXX_NAMESPACES
using SigC::Signal0;
#endif

/* (Baruch Even 2000-08-05)
 * This has a major drawback: it is only designed for X servers, no easy
 * porting to non X-server based platform is offered right now, this is done
 * in order to get a first version out of the door.
 *
 * Later versions should consider how to do this with more platform 
 * independence, this will probably involve changing the Painter class too.
 */

/* (Baruch Even 2000-08-05)
 * This should be made reference counted, but for the sake of initial design
 * I'll forego that and just make a first version that actually works, though
 * it may fail or leak in real document, this is an initial design to try
 * ideas on and be a testbed.
 * It may just as well be scraped later on to create a better design based on
 * the results of working with the current design.
 */

/// A GraphicsCache item holder.
class GraphicsCacheItem {
public:
	/// d-tor, frees the image structures.
	~GraphicsCacheItem();
	
	/// Get the height of the image. Returns -1 on error.
	int getHeight() const { return height_; }	
	
	/// Get the width of the image. Returns -1 on error.
	int getWidth() const { return width_; }

	/// Return a pixmap that can be displayed on X server.
	Pixmap getImage() const { return pixmap_; }

	enum ImageStatus {
		Loading = 1,
		ErrorConverting,
		ErrorReading,
		Loaded
	};
	
	/// Is the pixmap ready for display?
	ImageStatus getImageStatus() const { return imageStatus_; }

	/// Get a notification when the image conversion is done.
	/// used by an internal callback mechanism.
	void imageConverted(int retval);

	/// A signal objects can connect to in order to know when the image
	/// has arrived.
	Signal0<void> imageDone;
private:
    /// Private c-tor so that only GraphicsCache can create an instance.
    GraphicsCacheItem();

	/// Set the filename this item will be pointing too.
	bool setFilename(string const & filename);

	/// Create an XPM file version of the image.
	bool renderXPM(string const & filename);

	/// Load the image from XPM to memory Pixmap
	void loadXPMImage();
	
    ///
    friend class GraphicsCache;

	/// The file name of the XPM file.
	string xpmfile;
	/// The image height
	int height_;
	/// The image width
	int width_;
	/// Is the pixmap loaded?
	ImageStatus imageStatus_;
	/// The image pixmap
	Pixmap pixmap_;
	/// The rendering object.
	Renderer * renderer;

	/// The system caller, runs the convertor.
	Systemcalls syscall;
};

#endif
