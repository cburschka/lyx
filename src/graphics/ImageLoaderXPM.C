/* This file is part of
 * =================================================
 * 
 *          LyX, The Document Processor
 *          Copyright 1995 Matthias Ettrich.
 *          Copyright 1995-2001 The LyX Team.
 *
 * ================================================= */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ImageLoaderXPM.h"
#include "ColorHandler.h"
#include "lyxrc.h"
#include "debug.h"

#include "frontends/support/LyXImage.h"
#include "frontends/GUIRunTime.h"

#include "support/filetools.h"
#include "support/LAssert.h"

#include XPM_H_LOCATION
#include <iostream>
#include <fstream>


using std::ifstream;
using std::endl;
using std::ios;


bool ImageLoaderXPM::isImageFormatOK(string const & filename) const
{
	ifstream is(filename.c_str(), ios::in);

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
	Display * display = GUIRunTime::x11Display();

	//(BE 2000-08-05)
	// This might be a dirty thing, but I dont know any other solution.
	Screen * screen = ScreenOfDisplay(display, GUIRunTime::x11Screen());

	Pixmap pixmap;
	Pixmap mask;

	// If the pixmap contains a transparent colour, then set it to the
	// colour of the background (Angus 21 Sep 2001)
	XpmColorSymbol xpm_col;
	xpm_col.name = 0;
	xpm_col.value = "none";
	xpm_col.pixel = lyxColorHandler->colorPixel(LColor::graphicsbg);

	XpmAttributes attrib;
	attrib.valuemask = XpmCloseness | XpmColorSymbols;

	attrib.closeness = 10000;

	attrib.numsymbols = 1;
	attrib.colorsymbols = &xpm_col;

	// Set color_key to monochrome, grayscale or color
	// (Angus 21 Sep 2001)
	int color_key = 0;
	if (lyxrc.display_graphics == "color") {
		color_key = XPM_COLOR;

	} else if (lyxrc.display_graphics == "gray") {
		color_key = XPM_GRAY;

	} else if (lyxrc.display_graphics == "mono") {
		color_key = XPM_MONO;
	}

	// If setting color_key failed, then fail gracefully!
	if (color_key != 0) {
		attrib.valuemask |= XpmColorKey;
		attrib.color_key = color_key;

	} else {
		lyxerr << "Warning in ImageLoaderXPM::runImageLoader"
		       << "lyxrc.display_graphics == \""
		       << lyxrc.display_graphics
		       << "\""
		       << endl;
	}		

	// Load up the pixmap
	int status = XpmReadFileToPixmap(
			display, 
			XRootWindowOfScreen(screen), 
			const_cast<char *>(filename.c_str()), 
			&pixmap, &mask, &attrib);

	if (status != XpmSuccess) {
		lyxerr << "Error reading XPM file '" 
		       << XpmGetErrorString(status) << "'"
		       << endl;
		return ErrorWhileLoading;
	}

	// This should have been set by the XpmReadFileToPixmap call!
	lyx::Assert(attrib.valuemask & XpmSize);

	setImage(new LyXImage(pixmap, attrib.width, attrib.height));

	XpmFreeAttributes(&attrib);

	return OK;
}
