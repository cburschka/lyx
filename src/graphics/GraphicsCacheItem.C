/*
 * \file GraphicsCacheItem.C
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author Baruch Even <baruch.even@writeme.com>
 * \author Herbert Voss <voss@lyx.org>
 * \author Angus Leeming <leeming@lyx.org>
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

#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/signals/connection.hpp>
#include <boost/signals/trackable.hpp>

using std::endl;

namespace grfx {

struct CacheItem::Impl : public boost::signals::trackable {

	///
	Impl(CacheItem &, string const & file);

	/** Start the image conversion process, checking first that it is
	 *  necessary. If it is necessary, then a conversion task is started.
	 *  CacheItem asumes that the conversion is asynchronous and so
	 *  passes a Signal to the converting routine. When the conversion
	 *  is finished, this Signal is emitted, returning the converted
	 *  file to this->imageConverted.
	 *
	 *  If no file conversion is needed, then convertToDisplayFormat() calls
	 *  loadImage() directly.
	 *
	 *  convertToDisplayFormat() will set the loading status flag as
	 *  approriate through calls to setStatus().
	 */
	void convertToDisplayFormat();

	/** Load the image into memory. This is called either from
	 *  convertToDisplayFormat() direct or from imageConverted().
	 */
	void loadImage();

	/** Get a notification when the image conversion is done.
	 *  Connected to a signal on_finish_ which is passed to
	 *  Converter::convert.
	 */
	void imageConverted(bool);

	/** Get a notification when the image loading is done.
	 *  Connected to a signal on_finish_ which is passed to
	 *  grfx::Image::loadImage.
	 */
	void imageLoaded(bool);

	/** Sets the status of the loading process. Also notifies
	 *  listeners that the status has chacnged.
	 */
	void setStatus(ImageStatus new_status);

	///
	CacheItem & parent_;

	/// The filename we refer too.
	string filename_;
	/// Is the file compressed?
	bool zipped_;
	/// If so, store the uncompressed file in this temporary file.
	string unzipped_filename_;
	/// What file are we trying to load?
	string file_to_load_;
	/** Should we delete the file after loading? True if the file is
	 *  the result of a conversion process.
	 */
	bool remove_loaded_file_;

	/// The image and its loading status.
	boost::shared_ptr<Image> image_;
	///
	ImageStatus status_;

	/// The connection to the signal Image::finishedLoading
	boost::signals::connection cl_;

	/// The connection of the signal ConvProcess::finishedConversion,
	boost::signals::connection cc_;

	///
	boost::scoped_ptr<Converter> converter_;
};


CacheItem::CacheItem(string const & file)
	: pimpl_(new Impl(*this, file))
{}

		 
CacheItem::~CacheItem()
{}


string const & CacheItem::filename() const
{
	return pimpl_->filename_;
}


void CacheItem::startLoading()
{
	if (pimpl_->status_ != WaitingToLoad)
		return;

	pimpl_->convertToDisplayFormat();
}


Image const * CacheItem::image() const
{
	return pimpl_->image_.get();
}


ImageStatus CacheItem::status() const
{
	return pimpl_->status_;
}


//------------------------------
// Implementation details follow
//------------------------------


CacheItem::Impl::Impl(CacheItem & p, string const & file)
	: parent_(p), filename_(file), zipped_(false),
	  remove_loaded_file_(false), status_(WaitingToLoad)
{}


void CacheItem::Impl::setStatus(ImageStatus new_status)
{
	if (status_ == new_status)
		return;

	status_ = new_status;
	parent_.statusChanged();
}


void CacheItem::Impl::imageConverted(bool success)
{
	string const text = success ? "succeeded" : "failed";
	lyxerr[Debug::GRAPHICS] << "Image conversion " << text << "." << endl;

	file_to_load_ = converter_.get() ?
		converter_->convertedFile() : string();
	converter_.reset();
	cc_.disconnect();
	
	success = !file_to_load_.empty() && IsFileReadable(file_to_load_);
	lyxerr[Debug::GRAPHICS] << "Unable to find converted file!" << endl;

	if (!success) {
		setStatus(ErrorConverting);

		if (zipped_)
			lyx::unlink(unzipped_filename_);

		return;
	}

	loadImage();
}


// This function gets called from the callback after the image has been
// converted successfully.
void CacheItem::Impl::loadImage()
{
	setStatus(Loading);
	lyxerr[Debug::GRAPHICS] << "Loading image." << endl;

	image_ = Image::newImage();

	cl_.disconnect();
	cl_ = image_->finishedLoading.connect(
		boost::bind(&Impl::imageLoaded, this, _1));
	image_->load(file_to_load_);
}


void CacheItem::Impl::imageLoaded(bool success)
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

} // namespace grfx


namespace {

string const findTargetFormat(string const & from)
{
	typedef grfx::Image::FormatList FormatList;
	FormatList const formats = grfx::Image::loadableFormats();

	// There must be a format to load from.
	lyx::Assert(!formats.empty());

	// First ascertain if we can load directly with no conversion
	FormatList::const_iterator it1 = formats.begin();
	FormatList::const_iterator end = formats.end();
	for (; it1 != end; ++it1) {
		if (from == *it1)
			return *it1;
	}

	// So, we have to convert to a loadable format. Can we?
	FormatList::const_iterator it2 = formats.begin();
	for (; it2 != end; ++it2) {
		if (grfx::Converter::isReachable(from, *it2))
			return *it2;
	}

	// Failed! so we have to try to convert it to XPM format
	// with the standard converter
	return string("xpm");
}

} // anon namespace


namespace grfx {

void CacheItem::Impl::convertToDisplayFormat()
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
	string const to = findTargetFormat(from);

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
	converter_.reset(new Converter(filename, to_file_base, from, to));
	converter_->finishedConversion.connect(
		boost::bind(&Impl::imageConverted, this, _1));
	converter_->startConversion();
}

} // namespace grfx
