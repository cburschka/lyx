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

#include <map>

#include FORMS_H_LOCATION

#ifdef __GNUG__
#pragma implementation
#endif

#include "GraphicsCacheItem.h"
#include "GraphicsCacheItem_pimpl.h"

#include "frontends/support/LyXImage.h"
#include "ImageLoaderXPM.h"
#include "support/filetools.h"
#include "debug.h"
#include "support/LAssert.h"

using std::endl;
using std::map;


GraphicsCacheItem_pimpl::GraphicsCacheItem_pimpl()
	: imageStatus_(GraphicsCacheItem::Loading),
	  image_(0), imageLoader(0), refCount(0)
{}


GraphicsCacheItem_pimpl::~GraphicsCacheItem_pimpl()
{
	delete image_; image_ = 0;
	delete imageLoader; imageLoader = 0;
}


bool
GraphicsCacheItem_pimpl::setFilename(string const & filename)
{
	imageLoader = new ImageLoaderXPM();
	imageStatus_ = GraphicsCacheItem::Loading;
	
	if (renderXPM(filename))
		return true;
	
	return false;
}


/*** Callback method ***/

typedef map<string, GraphicsCacheItem_pimpl*> CallbackMap;
static CallbackMap callbackMap;


static
void callback(string cmd, int retval)
{
	lyxerr << "callback, cmd=" << cmd << ", retval=" << retval << endl;

	GraphicsCacheItem_pimpl * item = callbackMap[cmd];
	callbackMap.erase(cmd);
	
	item->imageConverted(retval);
}


void
GraphicsCacheItem_pimpl::imageConverted(int retval)
{
	lyxerr << "imageConverted, retval=" << retval << endl;

	if (retval) {
		lyxerr << "(GraphicsCacheItem_pimpl::imageConverter) "
			"Error converting image." << endl;
		imageStatus_ = GraphicsCacheItem::ErrorConverting;
		return;
	}

	// Do the actual image loading from XPM to memory.
	loadXPMImage();	
}

/**********************/

bool
GraphicsCacheItem_pimpl::renderXPM(string const & filename)
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
	xpmfile = lyx::tempName(string(), temp);
#warning When is this tempfile unlinked? (Lgb)
	xpmfile = ChangeExtension(xpmfile, ".xpm");	
	
	command += xpmfile;

	// Set the callback mapping to point to us.
	callbackMap[command] = this;

	// Run the convertor.
	// There is a problem with running it asyncronously, it doesn't return
	// to call the callback, so until the Systemcalls mechanism is fixed
	// I use the syncronous method.
	lyxerr << "Launching convert to xpm, command=" << command << endl;
//	syscall.startscript(Systemcalls::DontWait, command, &callback);
	syscall.startscript(Systemcalls::Wait, command, &callback);

	return true;
}


// This function gets called from the callback after the image has been
// converted successfully.
void
GraphicsCacheItem_pimpl::loadXPMImage()
{
	lyxerr << "Loading XPM Image... ";
	
	if (imageLoader->loadImage(xpmfile)) {
		lyxerr << "Success." << endl;
		image_ = imageLoader->getImage();
		imageStatus_ = GraphicsCacheItem::Loaded;
	} else {
		lyxerr << "Fail." << endl;
		imageStatus_ = GraphicsCacheItem::ErrorReading;
	}

	// remove the xpm file now.
	lyx::unlink(xpmfile);
	// and remove the reference to the filename.
	xpmfile = string();
}
