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

#ifndef XPM_RENDERER_H
#define XPM_RENDERER_H

#ifdef __GNUG__
#pragma interface
#endif

#include "graphics/Renderer.h"

///
class XPM_Renderer : public Renderer {
public:
	/// c-tor.
	XPM_Renderer();

	/// Load the XPM image and create a pixmap out of it.
	virtual bool renderImage();

private:
	/// Verify that filename is really an XPM file.
	virtual bool isImageFormatOK(string const & filename) const;
};

#endif
