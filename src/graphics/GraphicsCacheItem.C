/* This file is part of
 * =================================================
 * 
 *          LyX, The Document Processor
 *          Copyright 1995 Matthias Ettrich.
 *          Copyright 1995-2001 The LyX Team.
 *
 * \author Baruch Even
 * \author Herbert Voss <voss@lyx.org>
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
#include "lyxfunc.h"

using std::endl;

/*
 * The order of conversion:
 *
 * The c-tor calls convertImage()
 * 
 * convertImage() verifies that we need to do conversion, if not it will just
 * call the loadImage()
 * if conversion is needed, it will initiate the conversion.
 *
 * When the conversion is completed imageConverted() is called, which in turn
 * calls loadImage().
 *
 * Since we currently do everything synchronously, convertImage() calls
 * imageConverted() right after it does the call to the conversion process.
*/

GraphicsCacheItem::GraphicsCacheItem(string const & filename)
	: imageStatus_(GraphicsCacheItem::Loading)
{
	filename_ = filename;
	
	bool success = convertImage(filename);
	if (! success) // Conversion failed miserably (couldn't even start).
		setStatus(ErrorConverting);
}


GraphicsCacheItem::~GraphicsCacheItem()
{}


GraphicsCacheItem::ImageStatus 
GraphicsCacheItem::getImageStatus() const { return imageStatus_; }


void GraphicsCacheItem::setStatus(ImageStatus new_status)
{
	imageStatus_ = new_status;
}


LyXImage * 
GraphicsCacheItem::getImage() const { return image_.get(); }


void GraphicsCacheItem::imageConverted(bool success)
{
	// Debug output
	string text = "succeeded";
	if (!success)
		text = "failed";
	lyxerr << "imageConverted, conversion " << text	<< "." << endl;

	if (! success) {
		lyxerr << "(GraphicsCacheItem::imageConverter) "
			"Error converting image." << endl;
		setStatus(GraphicsCacheItem::ErrorConverting);
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
		if (converters.isReachable(from, *iter))
			break;
	}
	if (iter == end) {
		// We do not know how to convert the image to something loadable.
		lyxerr << "ERROR: Do not know how to convert image." << std::endl;
		return string();
	}
	return (*iter);
}

} // anon namespace

	
bool GraphicsCacheItem::convertImage(string const & filename)
{
	setStatus(GraphicsCacheItem::Converting);
	string filename_ = string(filename);
	lyxerr << "try to convert image file: " << filename_ << endl;
// maybe that other zip extensions also be useful, especially the
// ones that may be declared in texmf/tex/latex/config/graphics.cfg.
// for example:
/* -----------snip-------------
          {\DeclareGraphicsRule{.pz}{eps}{.bb}{}%
           \DeclareGraphicsRule{.eps.Z}{eps}{.eps.bb}{}%
           \DeclareGraphicsRule{.ps.Z}{eps}{.ps.bb}{}%
           \DeclareGraphicsRule{.ps.gz}{eps}{.ps.bb}{}%
           \DeclareGraphicsRule{.eps.gz}{eps}{.eps.bb}{}}}%
   -----------snip-------------*/

	lyxerr << "GetExtension: " << GetExtension(filename_) << endl;
	bool const zipped = zippedFile(filename_);
	if (zipped)
	    filename_ = unzipFile(filename_);
	string const from = getExtFromContents(filename_);	// get the type
	lyxerr << "GetExtFromContents: " << from << endl;
	string const to = findTargetFormat(from);
	lyxerr << "from: " << from << " -> " << to << endl;
	if (to.empty()) 
		return false;
	// manage zipped files. unzip them first into the tempdir
	if (from == to) {
		// No conversion needed!
		// Saves more than just time: prevents the deletion of
		// the "to" file after loading when it's the same as the "from"!
		tempfile = filename_;
		loadImage();	
		return true;
	}
	// Take only the filename part of the file, without path or extension.
	string temp = OnlyFilename(filename_);
	temp = ChangeExtension(filename_, string());
	
	// Add some stuff to have it a unique temp file.
	// This tempfile is deleted in loadImage after it is loaded to memory.
	tempfile = lyx::tempName(string(), temp);
	// Remove the temp file, we only want the name...
	lyx::unlink(tempfile);
	bool result = converters.convert(0, filename_, tempfile, from, to);
	tempfile.append(".xpm");
	// For now we are synchronous
	imageConverted(result);
	// Cleanup after the conversion.
	lyx::unlink(tempfile);
	if (zipped)
	    lyx::unlink(filename_);
	tempfile = string();
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
		setStatus(GraphicsCacheItem::Loaded);
	} else {
		lyxerr << "Loading " << tempfile << "Failed" << endl;
		setStatus(GraphicsCacheItem::ErrorReading);
	}
}
