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
#include "XPM_Renderer.h"
#include "frontends/support/LyXImage.h"

#include FORMS_H_LOCATION
#include XPM_H_LOCATION
#include <iostream>
#include <fstream>

#include "support/LAssert.h"
#include "debug.h"

using std::endl;
using std::ios;


XPM_Renderer::XPM_Renderer()
	: Renderer()
{}


bool XPM_Renderer::renderImage()
{
	Pixmap pixmap;
	Pixmap mask;
	XpmAttributes attrib;
	attrib.valuemask = 0;
	
	Display * display = fl_get_display();

//(BE 2000-08-05)
#warning This might be a dirty thing, but I dont know any other solution.
	Screen * screen = ScreenOfDisplay(fl_get_display(), fl_screen); //DefaultScreen(display);

	int status = XpmReadFileToPixmap(
			display, 
			XRootWindowOfScreen(screen), 
			const_cast<char *>(getFilename().c_str()), 
			&pixmap, &mask, &attrib);

	if (status != XpmSuccess) {
		lyxerr << "Error reading XPM file '" 
			<< XpmGetErrorString(status) 
			<< endl;
		return false;
	}
	
	// This should have been set by the XpmReadFileToPixmap call!
	Assert(attrib.valuemask & XpmSize);

	setPixmap(new LyXImage(pixmap), attrib.width, attrib.height);

	XpmFreeAttributes(&attrib);

	return true;
}


bool XPM_Renderer::isImageFormatOK(string const & filename) const
{
	std::ifstream is(filename.c_str(), ios::in);

	// The signature of the file without the spaces.
	static const char str[] = "/*XPM*/";
	const char * ptr = str;

	do {
		char c;
		is >> c;

		if (c != *ptr)
			return false;
		
		++ptr;
	} while (*ptr != '\0');

	return true;
}
