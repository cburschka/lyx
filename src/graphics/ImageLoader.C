// -*- C++ -*-
/* This file is part of
 * =================================================
 * 
 *          LyX, The Document Processor
 *          Copyright 1995 Matthias Ettrich.
 *          Copyright 1995-2000 The LyX Team.
 *
 * ================================================= */

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>
#include "ImageLoader.h"
#include "frontends/support/LyXImage.h"

#include "support/filetools.h"

ImageLoader::ImageLoader()
		: image_(0)
{
}

ImageLoader::~ImageLoader()
{
	freeImage();
}

bool ImageLoader::isImageFormatOK(string const & /*filename*/) const
{
	return false;
}

void ImageLoader::setImage(LyXImage * image)
{
	freeImage();

	image_ = image;
}

void ImageLoader::freeImage()
{
	delete image_;
	image_ = 0;
}

ImageLoader::FormatList const
ImageLoader::loadableFormats() const
{
	return FormatList();
}

ImageLoader::Result
ImageLoader::loadImage(string const & filename)
{
	// Make sure file exists and is readable.
	if (! IsFileReadable(filename)) {
		lyxerr << "No XPM file found." << endl;
		return NoFile;
	}

	// Verify that the file format is correct.
	if (! isImageFormatOK(filename)) {
		lyxerr << "File format incorrect." << endl;
		return ImageFormatUnknown;
	}

	freeImage();

	return runImageLoader(filename);
}

