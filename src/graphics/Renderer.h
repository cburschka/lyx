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

#ifndef RENDERER_H
#define RENDERER_H

#include <config.h>

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"
#include "X11/Xlib.h"
#include "support/utility.hpp"

/** Renderer is a base class that is used to take an image format, and
    render it into a Pixmap in order to be able to display it later on
    in LyX. Essentially it's job is to load an image format and create
    a Pixmap from it. It also needs to do various transforms on the
    image, like Rotation, Resize and color reduction.
 */
class Renderer : public noncopyable {
public:
	/// c-tor.
	Renderer();
	/// d-tor.
	virtual ~Renderer();

	/// Set the filename that we will render
	bool setFilename(string const & filename);

	/// Render the image, doing the necessary transforms.
	virtual bool renderImage() = 0;

	/// Get the last rendered pixmap.
	Pixmap getPixmap() const;

	/// Get the width of the pixmap.
	unsigned int getWidth() const;

	/// Get the height of the pixmap.
	unsigned int getHeight() const;

protected:
	/// Verify that the file is one that we can handle.
	virtual bool isImageFormatOK(string const & filename) const = 0;

	/// Set the pixmap.
	void setPixmap(Pixmap pixmap, unsigned int width, unsigned int height);
	///
	string const & getFilename() const;

private:
	/// Free the loaded pixmap
	void freePixmap();
	
	/// The filename of the image file that we are responsible for.
	string filename_;
	/// The last rendered pixmap.
	Pixmap pixmap_;
	/// The width of the rendered pixmap.
	unsigned int width_;
	/// The height of the rendered pixmap.
	unsigned int height_;
	/// is Pixmap ready?
	bool pixmapLoaded_;
};

#endif
