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
#include "graphics/ImageLoader.h"
#include "support/syscall.h"

#include "sigc++/signal_system.h"
#ifdef SIGC_CXX_NAMESPACES
using SigC::Signal0;
#endif

class LyXImage;

/// A GraphicsCache item holder.
class GraphicsCacheItem_pimpl {
public:
	/// d-tor, frees the image structures.
	~GraphicsCacheItem_pimpl();
	
	/// Return a pixmap that can be displayed on X server.
	LyXImage * getImage() const { return image_; };

	typedef GraphicsCacheItem::ImageStatus ImageStatus;
	
	/// Is the pixmap ready for display?
	ImageStatus getImageStatus() const; 

	/** Get a notification when the image conversion is done.
	    used by an internal callback mechanism. */
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
	/// Is the pixmap loaded?
	ImageStatus imageStatus_;
	/// The image pixmap
	LyXImage * image_;
	/// The rendering object.
	ImageLoader * imageLoader;

	/// The system caller, runs the convertor.
	Systemcalls syscall;

	/// The reference count
	int refCount;
};

#endif
