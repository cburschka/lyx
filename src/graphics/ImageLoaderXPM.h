// -*- C++ -*-
/* This file is part of
 * =================================================
 * 
 *          LyX, The Document Processor
 *          Copyright 1995 Matthias Ettrich.
 *          Copyright 1995-2001 The LyX Team.
 *
 * ================================================= */

#ifndef IMAGELOADER_XPM_H
#define IMAGELOADER_XPM_H

#ifdef __GNUG__
#pragma interface
#endif

#include "graphics/ImageLoader.h"

/** ImageLoaderXPM is an implementation of ImageLoader that can load XPM images by
 * using libXPM.
 *  
 *  @Author Baruch Even, <baruch.even@writeme.com>
 */
class ImageLoaderXPM : public ImageLoader {
public:
	/// c-tor.
	ImageLoaderXPM() {};
	/// d-tor.
	virtual ~ImageLoaderXPM() {};

	/// Return the list of loadable formats.
	virtual FormatList const loadableFormats() const;
	
protected:
	/// Verify that the file is one that we can handle.
	virtual bool isImageFormatOK(string const & filename) const;

	/// Do the actual image loading.
	virtual ImageLoader::Result runImageLoader(string const & filename);
};

#endif
