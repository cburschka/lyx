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

#include "GraphicsCacheItem.h"

#include "graphics/XPM_Renderer.h"
#include "support/filetools.h"
#include "debug.h"
#include "support/LAssert.h"
#include <unistd.h> // unlink

#include <map>

#include FORMS_H_LOCATION

using std::endl;
using std::map;

GraphicsCacheItem::GraphicsCacheItem()
	: height_(-1), width_(-1), imageStatus_(Loading),
	  pixmap_(0), renderer(0)
{}

GraphicsCacheItem::~GraphicsCacheItem()
{
	if (imageStatus_ == Loaded) {
		XFreePixmap(fl_display, pixmap_);
	}

	delete renderer;
}

bool
GraphicsCacheItem::setFilename(string const & filename)
{
	imageStatus_ = Loading;

	renderer = new XPM_Renderer();
	if (renderXPM(filename))
		return true;
	
	return false;
}

/*** Callback method ***/

typedef map<string, GraphicsCacheItem*> CallbackMap;
static CallbackMap callbackMap;

void
callback(string cmd, int retval)
{
	lyxerr << "callback, cmd="<<cmd<<", retval="<<retval<<endl;

	GraphicsCacheItem * item = callbackMap[cmd];
	callbackMap.erase(cmd);
	
	item->imageConverted(retval);
}

void
GraphicsCacheItem::imageConverted(int retval)
{
	lyxerr << "imageConverted, retval="<<retval<<endl;

	if (retval) {
		imageStatus_ = ErrorConverting;
		return;
	}

	// Do the actual image loading from XPM to memory.
	loadXPMImage();	
}

/**********************/

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
	temp = ChangeExtension(filename , string());
	
	// Add some stuff to have it a unique temp file.
	xpmfile = TmpFileName(string(), temp);
	xpmfile = ChangeExtension(xpmfile, ".xpm");	
	
	command += xpmfile;

	// Set the callback mapping to point to us.
	callbackMap[command] = this;

	// Run the convertor.
	// There is a problem with running it asyncronously, it doesn't return
	// to call the callback, so until the Systemcalls mechanism is fixed
	// I use the syncronous method.
	lyxerr << "Launching convert to xpm, command="<<command<<endl;
//	syscall.startscript(Systemcalls::DontWait, command, &callback);
	syscall.startscript(Systemcalls::Wait, command, &callback);

	return true;
}

// This function gets called from the callback after the image has been
// converted successfully.
void
GraphicsCacheItem::loadXPMImage()
{
	if (! renderer->setFilename(xpmfile)) {
		return;
	}

	if (renderer->renderImage()) {
		pixmap_ = renderer->getPixmap();
		width_ = renderer->getWidth();
		height_ = renderer->getHeight();
		imageStatus_ = Loaded;
	} else {
		imageStatus_ = ErrorReading;
	}

	imageDone.emit();

	// remove the xpm file now.
	::unlink(xpmfile.c_str());
	// and remove the reference to the filename.
	xpmfile = string();
}
