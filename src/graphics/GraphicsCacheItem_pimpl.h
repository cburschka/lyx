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

#ifndef GRAPHICSCACHEITEM_PIMPL_H
#define GRAPHICSCACHEITEM_PIMPL_H

#include <config.h>

#ifdef __GNUG__
#pragma interface
#endif

#include "graphics/GraphicsCacheItem.h"

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

/// A GraphicsCache item holder.
class GraphicsCacheItem_pimpl {
public:
	/// d-tor, frees the image structures.
	~GraphicsCacheItem_pimpl();
	
	/// Get the height of the image. Returns -1 on error.
	int getHeight() const; 
	
	/// Get the width of the image. Returns -1 on error.
	int getWidth() const;

	/// Return a pixmap that can be displayed on X server.
	Pixmap getImage() const; 

	typedef GraphicsCacheItem::ImageStatus ImageStatus;
	
	/// Is the pixmap ready for display?
	ImageStatus getImageStatus() const; 

	/// Get a notification when the image conversion is done.
	/// used by an internal callback mechanism.
	void imageConverted(int retval);

private:
    /// Private c-tor so that only GraphicsCache can create an instance.
    GraphicsCacheItem_pimpl();

	/// Set the filename this item will be pointing too.
	bool setFilename(string const & filename);

	/// Create an XPM file version of the image.
	bool renderXPM(string const & filename);

	/// Load the image from XPM to memory Pixmap
	void loadXPMImage();
	
    ///
    friend class GraphicsCacheItem;

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

	/// The reference count
	int refCount;
};

#endif
