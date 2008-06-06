/**
 * \file GraphicsCacheItem.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Baruch Even
 * \author Herbert Voﬂ
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GraphicsCacheItem.h"

#include "GraphicsCache.h"
#include "GraphicsConverter.h"
#include "GraphicsImage.h"

#include "ConverterCache.h"
#include "Format.h"

#include "support/debug.h"
#include "support/FileName.h"
#include "support/filetools.h"
#include "support/FileMonitor.h"

#include <boost/bind.hpp>

using namespace std;
using namespace lyx::support;

namespace lyx {

namespace graphics {

class CacheItem::Impl : public boost::signals::trackable {
public:

	///
	Impl(FileName const & file);

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

	/** Sets the status of the loading process. Also notifies
	 *  listeners that the status has changed.
	 */
	void setStatus(ImageStatus new_status);

	/** Can be invoked directly by the user, but is also connected to the
	 *  FileMonitor and so is invoked when the file is changed
	 *  (if monitoring is taking place).
	 */
	void startLoading();

	/** If we are asked to load the file for a second or further time,
	 *  (because the file has changed), then we'll have to first reset
	 *  many of the variables below.
	 */
	void reset();

	/// The filename we refer too.
	FileName const filename_;
	///
	FileMonitor const monitor_;

	/// Is the file compressed?
	bool zipped_;
	/// If so, store the uncompressed file in this temporary file.
	FileName unzipped_filename_;
	/// The target format
	string to_;
	/// What file are we trying to load?
	FileName file_to_load_;
	/** Should we delete the file after loading? True if the file is
	 *  the result of a conversion process.
	 */
	bool remove_loaded_file_;

	/// The image and its loading status.
	boost::shared_ptr<Image> image_;
	///
	ImageStatus status_;

	/// This signal is emitted when the image loading status changes.
	boost::signal<void()> statusChanged;

	/// The connection of the signal ConvProcess::finishedConversion,
	boost::signals::connection cc_;

	///
	boost::scoped_ptr<Converter> converter_;
};


CacheItem::CacheItem(FileName const & file)
	: pimpl_(new Impl(file))
{}


CacheItem::~CacheItem()
{
	delete pimpl_;
}


FileName const & CacheItem::filename() const
{
	return pimpl_->filename_;
}


void CacheItem::startLoading() const
{
	pimpl_->startLoading();
}


void CacheItem::startMonitoring() const
{
	if (!pimpl_->monitor_.monitoring())
		pimpl_->monitor_.start();
}


bool CacheItem::monitoring() const
{
	return pimpl_->monitor_.monitoring();
}


unsigned long CacheItem::checksum() const
{
	return pimpl_->monitor_.checksum();
}


Image const * CacheItem::image() const
{
	return pimpl_->image_.get();
}


ImageStatus CacheItem::status() const
{
	return pimpl_->status_;
}


boost::signals::connection CacheItem::connect(slot_type const & slot) const
{
	return pimpl_->statusChanged.connect(slot);
}


//------------------------------
// Implementation details follow
//------------------------------


CacheItem::Impl::Impl(FileName const & file)
	: filename_(file),
	  monitor_(file, 2000),
	  zipped_(false),
	  remove_loaded_file_(false),
	  status_(WaitingToLoad)
{
	monitor_.connect(boost::bind(&Impl::startLoading, this));
}


void CacheItem::Impl::startLoading()
{
	if (status_ != WaitingToLoad)
		reset();

	convertToDisplayFormat();
}


void CacheItem::Impl::reset()
{
	zipped_ = false;
	if (!unzipped_filename_.empty())
		unzipped_filename_.removeFile();
	unzipped_filename_.erase();

	if (remove_loaded_file_ && !file_to_load_.empty())
		file_to_load_.removeFile();
	remove_loaded_file_ = false;
	file_to_load_.erase();
	to_.erase();

	if (image_.get())
		image_.reset();

	status_ = WaitingToLoad;

	if (cc_.connected())
		cc_.disconnect();

	if (converter_.get())
		converter_.reset();
}


void CacheItem::Impl::setStatus(ImageStatus new_status)
{
	if (status_ == new_status)
		return;

	status_ = new_status;
	statusChanged();
}


void CacheItem::Impl::imageConverted(bool success)
{
	string const text = success ? "succeeded" : "failed";
	LYXERR(Debug::GRAPHICS, "Image conversion " << text << '.');

	file_to_load_ = converter_.get() ?
		FileName(converter_->convertedFile()) : FileName();
	converter_.reset();
	cc_.disconnect();

	success = !file_to_load_.empty() && file_to_load_.isReadableFile();

	if (!success) {
		LYXERR(Debug::GRAPHICS, "Unable to find converted file!");
		setStatus(ErrorConverting);

		if (zipped_)
			unzipped_filename_.removeFile();

		return;
	}

	// Add the converted file to the file cache
	ConverterCache::get().add(filename_, to_, file_to_load_);

	loadImage();
}


// This function gets called from the callback after the image has been
// converted successfully.
void CacheItem::Impl::loadImage()
{
	setStatus(Loading);
	LYXERR(Debug::GRAPHICS, "Loading image.");

	image_.reset(Image::newImage());

	bool success = image_->load(file_to_load_);
	string const text = success ? "succeeded" : "failed";
	LYXERR(Debug::GRAPHICS, "Image loading " << text << '.');

	// Clean up after loading.
	if (zipped_)
		unzipped_filename_.removeFile();

	if (remove_loaded_file_ && unzipped_filename_ != file_to_load_)
		file_to_load_.removeFile();

	if (!success) {
		setStatus(ErrorLoading);
		return;
	}

	// Inform the outside world.
	setStatus(Loaded);
}


static string const findTargetFormat(string const & from)
{
	typedef vector<string> FormatList;
	FormatList const & formats = Cache::get().loadableFormats();

	 // There must be a format to load from.
	LASSERT(!formats.empty(), /**/);

	// Use the standard converter if we don't know the format to load
	// from.
	if (from.empty())
		return string("ppm");

	// First ascertain if we can load directly with no conversion
	FormatList::const_iterator it  = formats.begin();
	FormatList::const_iterator end = formats.end();
	for (; it != end; ++it) {
		if (from == *it)
			return *it;
	}

	// So, we have to convert to a loadable format. Can we?
	it = formats.begin();
	for (; it != end; ++it) {
		if (lyx::graphics::Converter::isReachable(from, *it))
			return *it;
		else
			LYXERR(Debug::GRAPHICS, "Unable to convert from " << from
				<< " to " << *it);
	}

	// Failed! so we have to try to convert it to PPM format
	// with the standard converter
	return string("ppm");
}


void CacheItem::Impl::convertToDisplayFormat()
{
	setStatus(Converting);

	// First, check that the file exists!
	if (!filename_.isReadableFile()) {
		if (status_ != ErrorNoFile) {
			setStatus(ErrorNoFile);
			LYXERR(Debug::GRAPHICS, "\tThe file is not readable");
		}
		return;
	}

	// Make a local copy in case we unzip it
	FileName filename;
	zipped_ = filename_.isZippedFile();
	if (zipped_) {
		unzipped_filename_ = FileName::tempName(
			filename_.toFilesystemEncoding());
		if (unzipped_filename_.empty()) {
			setStatus(ErrorConverting);
			LYXERR(Debug::GRAPHICS, "\tCould not create temporary file.");
			return;
		}
		filename = unzipFile(filename_, unzipped_filename_.toFilesystemEncoding());
	} else {
		filename = filename_;
	}

	docstring const displayed_filename = makeDisplayPath(filename_.absFilename());
	LYXERR(Debug::GRAPHICS, "[CacheItem::Impl::convertToDisplayFormat]\n"
		<< "\tAttempting to convert image file: " << filename
		<< "\n\twith displayed filename: " << to_utf8(displayed_filename));

	string const from = formats.getFormatFromFile(filename);
	if (from.empty()) {
		setStatus(ErrorConverting);
		LYXERR(Debug::GRAPHICS, "\tCould not determine file format.");
	}
	LYXERR(Debug::GRAPHICS, "\n\tThe file contains " << from << " format data.");
	to_ = findTargetFormat(from);

	if (from == to_) {
		// No conversion needed!
		LYXERR(Debug::GRAPHICS, "\tNo conversion needed (from == to)!");
		file_to_load_ = filename;
		loadImage();
		return;
	}

	if (ConverterCache::get().inCache(filename, to_)) {
		LYXERR(Debug::GRAPHICS, "\tNo conversion needed (file in file cache)!");
		file_to_load_ = ConverterCache::get().cacheName(filename, to_);
		loadImage();
		return;
	}

	LYXERR(Debug::GRAPHICS, "\tConverting it to " << to_ << " format.");

	// Add some stuff to create a uniquely named temporary file.
	// This file is deleted in loadImage after it is loaded into memory.
	FileName const to_file_base = FileName::tempName("CacheItem");
	remove_loaded_file_ = true;

	// Remove the temp file, we only want the name...
	// FIXME: This is unsafe!
	to_file_base.removeFile();

	// Connect a signal to this->imageConverted and pass this signal to
	// the graphics converter so that we can load the modified file
	// on completion of the conversion process.
	converter_.reset(new Converter(filename, to_file_base.absFilename(), from, to_));
	converter_->connect(boost::bind(&Impl::imageConverted, this, _1));
	converter_->startConversion();
}

} // namespace graphics
} // namespace lyx
