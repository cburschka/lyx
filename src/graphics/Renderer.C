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

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>
#include "Renderer.h"

#include FORMS_H_LOCATION
#include "support/filetools.h"


Renderer::Renderer()
	: width_(0), height_(0), pixmapLoaded_(false)
{}
	
Renderer::~Renderer()
{
	freePixmap();
}

bool Renderer::setFilename(string const & filename)
{
	// Make sure file exists and is readable.
	if (! IsFileReadable(filename)) {
		return false;
	}
	
	// Verify that the file format is correct.
	if (! isImageFormatOK(filename)) {
		return false;
	}
			
	filename_ = filename;
	return true;
}

bool Renderer::renderImage()
{
	return false;
}

bool Renderer::isImageFormatOK(string const & /*filename*/) const
{
	return false;
}

void Renderer::setPixmap(Pixmap pixmap, unsigned int width, unsigned int height)
{
	freePixmap();

	pixmap_ = pixmap;
	width_ = width;
	height_ = height;
	pixmapLoaded_ = true;
}

Pixmap Renderer::getPixmap() const
{ return pixmap_; }

unsigned int Renderer::getWidth() const
{ return width_; }

unsigned int Renderer::getHeight() const
{ return height_; }

string const & Renderer::getFilename() const
{ return filename_; }

void Renderer::freePixmap()
{
	if (pixmapLoaded_)
		XFreePixmap(fl_display, pixmap_);
}
