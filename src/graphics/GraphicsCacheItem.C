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

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "graphics/GraphicsCache.h"
#include "graphics/GraphicsCacheItem.h"
#include "frontends/support/LyXImage.h"
#include "graphics/ImageLoaderXPM.h"
#include "support/filetools.h"
#include "support/lyxlib.h"
#include "support/syscall.h"

#include "debug.h"

using std::endl;

GraphicsCacheItem::GraphicsCacheItem(string const & filename)
	: imageStatus_(GraphicsCacheItem::Loading)
{
	filename_ = filename;
	
	renderXPM(filename);
	// For now we do it synchronously
	imageConverted(0);
}


GraphicsCacheItem::~GraphicsCacheItem()
{}


GraphicsCacheItem::ImageStatus 
GraphicsCacheItem::getImageStatus() const { return imageStatus_; }


LyXImage * 
GraphicsCacheItem::getImage() const { return image_.get(); }


void
GraphicsCacheItem::imageConverted(int retval)
{
	lyxerr << "imageConverted, retval=" << retval << endl;

	if (retval) {
		lyxerr << "(GraphicsCacheItem::imageConverter) "
			"Error converting image." << endl;
		imageStatus_ = GraphicsCacheItem::ErrorConverting;
		return;
	}

	// Do the actual image loading from XPM to memory.
	loadXPMImage();	
}

	
bool
GraphicsCacheItem::renderXPM(string const & filename)
{
	// Create the command to do the conversion, this depends on ImageMagicks
	// convert program.
	string command = "convert ";
	command += filename;
	command += " XPM:";

	// Take only the filename part of the file, without path or extension.
	string temp = OnlyFilename(filename);
	temp = ChangeExtension(filename, string());
	
	// Add some stuff to have it a unique temp file.
	// This tempfile is deleted in loadXPMImage after it is loaded to memory.
	tempfile = lyx::tempName(string(), temp);
	// Remove the temp file, we only want the name...
	lyx::unlink(tempfile);
	tempfile = ChangeExtension(tempfile, ".xpm");	
	
	command += tempfile;

	// Run the convertor.
	lyxerr << "Launching convert to xpm, command=" << command << endl;
	Systemcalls syscall;
	syscall.startscript(Systemcalls::Wait, command);

	return true;
}


// This function gets called from the callback after the image has been
// converted successfully.
void
GraphicsCacheItem::loadXPMImage()
{
	lyxerr << "Loading XPM Image... ";

	ImageLoaderXPM imageLoader;
	if (imageLoader.loadImage(tempfile) == ImageLoader::OK) {
		lyxerr << "Success." << endl;
		image_.reset(imageLoader.getImage());
		imageStatus_ = GraphicsCacheItem::Loaded;
	} else {
		lyxerr << "Loading " << tempfile << "Failed" << endl;
		imageStatus_ = GraphicsCacheItem::ErrorReading;
	}

	// remove the xpm file now.
	lyx::unlink(tempfile);
	// and remove the reference to the filename.
	tempfile = string();
}
