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
#include "ImageLoaderXPM.h"
#include "frontends/support/LyXImage.h"
#include "support/filetools.h"

#include FORMS_H_LOCATION
#include XPM_H_LOCATION
#include <iostream>
#include <fstream>

#include "support/LAssert.h"
#include "debug.h"

using std::endl;
using std::ios;

bool ImageLoaderXPM::isImageFormatOK(string const & filename) const
{
	std::ifstream is(filename.c_str(), ios::in);

	// The signature of the file without the spaces.
	static char const str[] = "/*XPM*/";
	char const * ptr = str;

	for (; *ptr != '\0'; ++ptr) {
		char c;
		is >> c;

		if (c != *ptr)
			return false;
	}

	return true;
}

ImageLoaderXPM::FormatList const 
ImageLoaderXPM::loadableFormats() const 
{
	FormatList formats;
	formats.push_back("xpm");

	return formats;
}
	
ImageLoader::Result 
ImageLoaderXPM::runImageLoader(string const & filename)
{
	Display * display = fl_get_display();

//(BE 2000-08-05)
#ifdef WITH_WARNINGS
#warning This might be a dirty thing, but I dont know any other solution.
#endif
	Screen * screen = ScreenOfDisplay(display, fl_screen);

	Pixmap pixmap;
	Pixmap mask;
	XpmAttributes attrib;
	attrib.valuemask = 0;
	
	int status = XpmReadFileToPixmap(
			display, 
			XRootWindowOfScreen(screen), 
			const_cast<char *>(filename.c_str()), 
			&pixmap, &mask, &attrib);

	if (status != XpmSuccess) {
		lyxerr << "Error reading XPM file '" 
			<< XpmGetErrorString(status) 
			<< endl;
		return ErrorWhileLoading;
	}
	
	// This should have been set by the XpmReadFileToPixmap call!
	Assert(attrib.valuemask & XpmSize);

	setImage(new LyXImage(pixmap, attrib.width, attrib.height));

	XpmFreeAttributes(&attrib);

	return OK;
}
