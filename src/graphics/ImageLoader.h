// -*- C++ -*-
/* This file is part of
 * =================================================
 * 
 *          LyX, The Document Processor
 *          Copyright 1995 Matthias Ettrich.
 *          Copyright 1995-2000 The LyX Team.
 *
 * ================================================= */

#ifndef IMAGELOADER_H
#define IMAGELOADER_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"
#include "boost/utility.hpp"
#include <vector>

class LyXImage;

/** ImageLoader is a base class for all image loaders. An ImageLoader instance is
 *  platform dependent, and knows how to load some image formats into a memory
 *  representation (LyXImage).
 *
 *  It may do the image loading asynchronously.
 *  
 *  @Author Baruch Even, <baruch.even@writeme.com>
 */
class ImageLoader : public boost::noncopyable {
public:
	/// Errors that can be returned from this class.
	enum Result {
		OK = 0,			
		ImageFormatUnknown, // This loader doesn't know how to load this file.
		NoFile,				// File doesn't exists.
		ErrorWhileLoading  // Unknown error when loading.
	};
	
	/// A list of supported formats.
	typedef std::vector<string> FormatList;
	
	/// c-tor.
	ImageLoader();
	/// d-tor.
	virtual ~ImageLoader();

	/// Start loading the image file.
	ImageLoader::Result loadImage(string const & filename);

	/** Get the last rendered pixmap. Returns 0 if no image is ready.
	 *  
	 *  It is a one time operation, that is, after you get the image
	 *  you are completely responsible to destroy it and the ImageLoader
	 *  will not know about the image.
	 *
	 *  This way we avoid deleting the image if you still use it and the 
	 *  ImageLoader is destructed, and if you don't use it we get to 
	 *  destruct the image to avoid memory leaks.
	 */
	LyXImage * getImage();

	/// Return the list of loadable formats.
	virtual FormatList const loadableFormats() const;
	
protected:
	/// Verify that the file is one that we can handle.
	virtual bool isImageFormatOK(string const & filename) const = 0;

	/// Do the actual image loading.
	virtual Result runImageLoader(string const & filename) = 0;

	/// Set the image that was loaded.
	void setImage(LyXImage * image);
	
private:
	/// Free the loaded image.
	void freeImage();
	
	/// The loaded image. An auto_ptr would be great here, but it's not
	/// available everywhere (gcc 2.95.2 doesnt have it).
	LyXImage * image_;
};

#endif
