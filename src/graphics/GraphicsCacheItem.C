/*
 * \file GraphicsCacheItem.C
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author Baruch Even <baruch.even@writeme.com>
 * \author Herbert Voss <voss@lyx.org>
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "graphics/GraphicsCacheItem.h"
#include "graphics/GraphicsImage.h"
#include "graphics/GraphicsConverter.h"

#include "debug.h"

#include "support/LAssert.h"
#include "support/filetools.h"

#include <boost/bind.hpp>

using std::endl;


namespace grfx {

GCacheItem::GCacheItem(string const & file)
	: filename_(file), zipped_(false),
	  remove_loaded_file_(false), status_(WaitingToLoad)
{}


void GCacheItem::startLoading()
{
	if (status() != WaitingToLoad)
		return;

	convertToDisplayFormat();
}


void GCacheItem::setStatus(ImageStatus new_status)
{
	if (status_ == new_status)
		return;

	status_ = new_status;
	statusChanged();
}


void GCacheItem::imageConverted(string const & file_to_load)
{
	bool const success =
		(!file_to_load.empty() && IsFileReadable(file_to_load));

	string const text = success ? "succeeded" : "failed";
	lyxerr[Debug::GRAPHICS] << "Image conversion " << text << "." << endl;

	if (!success) {
		setStatus(ErrorConverting);

		if (zipped_)
			lyx::unlink(unzipped_filename_);

		return;
	}

	cc_.disconnect();

	// Do the actual image loading from file to memory.
	file_to_load_ = file_to_load;

	loadImage();
}


// This function gets called from the callback after the image has been
// converted successfully.
void GCacheItem::loadImage()
{
	setStatus(Loading);
	lyxerr[Debug::GRAPHICS] << "Loading image." << endl;

	// Connect a signal to this->imageLoaded and pass this signal to
	// GImage::loadImage.
	SignalLoadTypePtr on_finish;
	on_finish.reset(new SignalLoadType);
	cl_ = on_finish->connect(boost::bind(&GCacheItem::imageLoaded, this, _1));

	image_ = GImage::newImage();
	image_->load(file_to_load_, on_finish);
}


void GCacheItem::imageLoaded(bool success)
{
	string const text = success ? "succeeded" : "failed";
	lyxerr[Debug::GRAPHICS] << "Image loading " << text << "." << endl;

	// Clean up after loading.
	if (zipped_)
		lyx::unlink(unzipped_filename_);

	if (remove_loaded_file_ && unzipped_filename_ != file_to_load_)
		lyx::unlink(file_to_load_);

	cl_.disconnect();

	if (!success) {
		setStatus(ErrorLoading);
		return;
	}

	setStatus(Loaded);
}


namespace {

string const findTargetFormat(string const & from)
{
	typedef GImage::FormatList FormatList;
	FormatList const formats = GImage::loadableFormats();

	// There must be a format to load from.
	lyx::Assert(!formats.empty());

	// First ascertain if we can load directly with no conversion
	FormatList::const_iterator it1  = formats.begin();
	FormatList::const_iterator end = formats.end();
	for (; it1 != end; ++it1) {
		if (from == *it1)
			return *it1;
	}

	// So, we have to convert to a loadable format. Can we?
	grfx::GConverter const & graphics_converter = grfx::GConverter::get();

	FormatList::const_iterator it2  = formats.begin();
	for (; it2 != end; ++it2) {
		if (graphics_converter.isReachable(from, *it2))
			return *it2;
	}

	// Failed! so we have to try to convert it to XPM format
	// with the standard converter
	return string("xpm");
}

} // anon namespace


void GCacheItem::convertToDisplayFormat()
{
	setStatus(Converting);
	// Make a local copy in case we unzip it
	string const filename = zippedFile(filename_) ?
		unzipFile(filename_) : filename_; 
	string const displayed_filename = MakeDisplayPath(filename_);
	lyxerr[Debug::GRAPHICS] << "[GrahicsCacheItem::convertToDisplayFormat]\n"
		<< "\tAttempting to convert image file: " << filename
		<< "\n\twith displayed filename: " << displayed_filename
		<< endl;

	// First, check that the file exists!
	if (!IsFileReadable(filename)) {
		setStatus(ErrorNoFile);
		lyxerr[Debug::GRAPHICS] << "\tThe file is not readable" << endl;
		return;
	}

	string from = getExtFromContents(filename);
	// Some old ps-files make problems, so we do not need direct
	// loading of an ps-file
	if (from == "ps") {
		lyxerr[Debug::GRAPHICS] 
		<< "\n\tThe file contains PostScript format data.\n" 
		<< "\tchanging it to eps-format to get it converted to xpm\n";
		from = "eps";
	} else
		lyxerr[Debug::GRAPHICS] 
			<< "\n\tThe file contains " << from << " format data." << endl;
	string const to = grfx::findTargetFormat(from);

	if (from == to) {
		// No conversion needed!
		lyxerr[Debug::GRAPHICS] << "\tNo conversion needed (from == to)!" << endl;
		file_to_load_ = filename;
		loadImage();
		return;
	}

	lyxerr[Debug::GRAPHICS] << "\tConverting it to " << to << " format." << endl;
	// Take only the filename part of the file, without path or extension.
	string const temp = ChangeExtension(OnlyFilename(filename), string());

	// Add some stuff to create a uniquely named temporary file.
	// This file is deleted in loadImage after it is loaded into memory.
	string const to_file_base = lyx::tempName(string(), temp);
	remove_loaded_file_ = true;

	// Remove the temp file, we only want the name...
	lyx::unlink(to_file_base);

	// Connect a signal to this->imageConverted and pass this signal to
	// the graphics converter so that we can load the modified file
	// on completion of the conversion process.
	SignalConvertTypePtr on_finish;
	on_finish.reset(new SignalConvertType);
	cc_ = on_finish->connect(boost::bind(&GCacheItem::imageConverted, this, _1));

	GConverter & graphics_converter = GConverter::get();
	graphics_converter.convert(filename, to_file_base, from, to, on_finish);
}

} // namespace grfx
