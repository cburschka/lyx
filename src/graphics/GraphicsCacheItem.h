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

class GraphicsCacheItem_pimpl;
class LyXImage;

/// A GraphicsCache item holder.
class GraphicsCacheItem {
public:
	/// d-tor, frees the image structures.
	~GraphicsCacheItem();
	/// copy c-tor.
	GraphicsCacheItem(GraphicsCacheItem const &);
	/// Assignment operator.
	GraphicsCacheItem & operator=(GraphicsCacheItem const &);
	
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

	/// Create another copy of the object.
	GraphicsCacheItem * Clone() const;
	
private:
	/// Private c-tor so that only GraphicsCache can create an instance.
	GraphicsCacheItem();

	/// internal copy mechanism.
	void copy(GraphicsCacheItem const &);
	/// internal destroy mechanism.
	void destroy();

	/// Set the filename this item will be pointing too.
	bool setFilename(string const & filename);

	///
	friend class GraphicsCache;

	///
	GraphicsCacheItem_pimpl * pimpl;

	/** The filename we refer too.
	    This is used when removing ourselves from the cache.
	*/
	string filename_;
};

#endif
