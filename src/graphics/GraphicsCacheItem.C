/* This file is part of
 * =================================================
 * 
 *          LyX, The Document Processor
 *          Copyright 1995 Matthias Ettrich.
 *          Copyright 1995-2001 The LyX Team.
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
#include "converter.h"
#include "support/filetools.h"
#include "support/lyxlib.h"
#include "lyx_gui_misc.h"
#include "debug.h"
#include "support/LAssert.h"
#include "gettext.h"

using std::endl;

GraphicsCacheItem::GraphicsCacheItem(string const & filename)
	: imageStatus_(GraphicsCacheItem::Loading)
{
	filename_ = filename;
	
	bool success = convertImage(filename);
	// For now we do it synchronously
	if (success) 
		imageConverted(0);
	else
		imageStatus_ = ErrorConverting;
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

	// Do the actual image loading from file to memory.
	loadImage();	
}


namespace {

string const findTargetFormat(string const & from)
{
	typedef ImageLoader::FormatList FormatList;
	FormatList formats = ImageLoaderXPM().loadableFormats();
	lyx::Assert(formats.size() > 0); // There must be a format to load from.
	
	FormatList::const_iterator iter = formats.begin();
	FormatList::const_iterator end  = formats.end();

	for (; iter != end; ++iter) {
		if (converters.IsReachable(from, *iter))
			break;
	}
	if (iter == end) {
		// We do not know how to convert the image to something loadable.
		lyxerr << "ERROR: Do not know how to convert image." << std::endl;

		string const first(_("Cannot convert image to display format"));
		string const second1(_("Need converter from "));
		string const second2(_(" to "));
		string const second(second1 + from + second2 + formats[0]);

		WriteAlert(first, second);
		
		return string();
	}

	return (*iter);
}

} // anon namespace

	
bool
GraphicsCacheItem::convertImage(string const & filename)
{
	string const from = GetExtension(filename);
	string const to = findTargetFormat(from);
	if (to.empty()) 
		return false;
	
	// Take only the filename part of the file, without path or extension.
	string temp = OnlyFilename(filename);
	temp = ChangeExtension(filename, string());
	
	// Add some stuff to have it a unique temp file.
	// This tempfile is deleted in loadImage after it is loaded to memory.
	tempfile = lyx::tempName(string(), temp);
	// Remove the temp file, we only want the name...
	lyx::unlink(tempfile);

	converters.Convert(0, filename, tempfile, from, to);

	return true;
}


// This function gets called from the callback after the image has been
// converted successfully.
void
GraphicsCacheItem::loadImage()
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
