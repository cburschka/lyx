/**
 * \file GraphicsCacheItem.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Baruch Even
 * \author Herbert Voß
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
#include "support/lassert.h"
#include "support/unique_ptr.h"

#include "support/bind.h"
#include "support/TempFile.h"

using namespace std;
using namespace lyx::support;

namespace lyx {

namespace graphics {

class CacheItem::Impl : public boost::signals2::trackable {
public:

	///
	Impl(FileName const & file);

	/**
	 *  If no file conversion is needed, then tryDisplayFormat() calls
	 *  loadImage() directly.
	 * \return true if a conversion is necessary and no error occurred. 
	 */
	bool tryDisplayFormat(FileName & filename, string & from);

	/** Start the image conversion process, checking first that it is
	 *  necessary. If it is necessary, then a conversion task is started.
	 *  CacheItem asumes that the conversion is asynchronous and so
	 *  passes a Signal to the converting routine. When the conversion
	 *  is finished, this Signal is emitted, returning the converted
	 *  file to this->imageConverted.
	 *
	 *  convertToDisplayFormat() will set the loading status flag as
	 *  approriate through calls to setStatus().
	 */
	void convertToDisplayFormat();

	/** Load the image into memory. This is called either from
	 *  convertToDisplayFormat() direct or from imageConverted().
	 */
	bool loadImage();

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
	std::shared_ptr<Image> image_;
	///
	ImageStatus status_;

	/// This signal is emitted when the image loading status changes.
	boost::signals2::signal<void()> statusChanged;

	/// The connection of the signal ConvProcess::finishedConversion,
	boost::signals2::connection cc_;

	///
	unique_ptr<Converter> converter_;
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


bool CacheItem::tryDisplayFormat() const
{
	if (pimpl_->status_ != WaitingToLoad)
		pimpl_->reset();
	FileName filename;
	string from;
	bool const conversion_needed = pimpl_->tryDisplayFormat(filename, from);
	bool const success = status() == Loaded && !conversion_needed;
	if (!success)
		pimpl_->reset();
	return success;
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


boost::signals2::connection CacheItem::connect(slot_type const & slot) const
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
	monitor_.connect(bind(&Impl::startLoading, this));
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

	if (image_)
		image_.reset();

	status_ = WaitingToLoad;

	if (cc_.connected())
		cc_.disconnect();

	if (converter_)
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

	file_to_load_ = converter_ ? FileName(converter_->convertedFile())
		                       : FileName();
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

	setStatus(loadImage() ? Loaded : ErrorLoading);
}


// This function gets called from the callback after the image has been
// converted successfully.
bool CacheItem::Impl::loadImage()
{
	LYXERR(Debug::GRAPHICS, "Loading image.");

	image_.reset(newImage());

	bool success = image_->load(file_to_load_);
	string const text = success ? "succeeded" : "failed";
	LYXERR(Debug::GRAPHICS, "Image loading " << text << '.');

	// Clean up after loading.
	if (zipped_)
		unzipped_filename_.removeFile();

	if (remove_loaded_file_ && unzipped_filename_ != file_to_load_)
		file_to_load_.removeFile();

	return success;
}


static string const findTargetFormat(string const & from)
{
	typedef vector<string> FormatList;
	FormatList const & formats = Cache::get().loadableFormats();

	 // There must be a format to load from.
	LASSERT(!formats.empty(), return string());

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


bool CacheItem::Impl::tryDisplayFormat(FileName & filename, string & from)
{
	// First, check that the file exists!
	filename_.refresh();
	if (!filename_.isReadableFile()) {
		if (status_ != ErrorNoFile) {
			status_ = ErrorNoFile;
			LYXERR(Debug::GRAPHICS, "\tThe file is not readable");
		}
		return false;
	}

	zipped_ = formats.isZippedFile(filename_);
	if (zipped_) {
		string tempname = unzippedFileName(filename_.toFilesystemEncoding());
		string const ext = getExtension(tempname);
		tempname = changeExtension(tempname, "") + "-XXXXXX";
		if (!ext.empty())
			tempname = addExtension(tempname, ext);
		TempFile tempfile(tempname);
		tempfile.setAutoRemove(false);
		unzipped_filename_ = tempfile.name();
		if (unzipped_filename_.empty()) {
			status_ = ErrorConverting;
			LYXERR(Debug::GRAPHICS, "\tCould not create temporary file.");
			return false;
		}
		filename = unzipFile(filename_, unzipped_filename_.toFilesystemEncoding());
	} else {
		filename = filename_;
	}

	docstring const displayed_filename = makeDisplayPath(filename_.absFileName());
	LYXERR(Debug::GRAPHICS, "[CacheItem::Impl::convertToDisplayFormat]\n"
		<< "\tAttempting to convert image file: " << filename
		<< "\n\twith displayed filename: " << to_utf8(displayed_filename));

	from = formats.getFormatFromFile(filename);
	if (from.empty()) {
		status_ = ErrorConverting;
		LYXERR(Debug::GRAPHICS, "\tCould not determine file format.");
	}
	LYXERR(Debug::GRAPHICS, "\n\tThe file contains " << from << " format data.");
	to_ = findTargetFormat(from);

	if (from == to_) {
		// No conversion needed!
		LYXERR(Debug::GRAPHICS, "\tNo conversion needed (from == to)!");
		file_to_load_ = filename;
		status_ = loadImage() ? Loaded : ErrorLoading;
		return false;
	}

	if (ConverterCache::get().inCache(filename, to_)) {
		LYXERR(Debug::GRAPHICS, "\tNo conversion needed (file in file cache)!");
		file_to_load_ = ConverterCache::get().cacheName(filename, to_);
		status_ = loadImage() ? Loaded : ErrorLoading;
		return false;
	}
	return true;
}


void CacheItem::Impl::convertToDisplayFormat()
{
	LYXERR(Debug::GRAPHICS, "\tConverting it to " << to_ << " format.");

	// Make a local copy in case we unzip it
	FileName filename;
	string from;
	if (!tryDisplayFormat(filename, from)) {
		// The image status has changed, tell it to the outside world.
		statusChanged();
		return;
	}

	// We will need a conversion, tell it to the outside world.
	setStatus(Converting);

	// Add some stuff to create a uniquely named temporary file.
	// This file is deleted in loadImage after it is loaded into memory.
	TempFile tempfile("CacheItem");
	tempfile.setAutoRemove(false);
	FileName const to_file_base = tempfile.name();
	remove_loaded_file_ = true;

	// Connect a signal to this->imageConverted and pass this signal to
	// the graphics converter so that we can load the modified file
	// on completion of the conversion process.
	converter_ = make_unique<Converter>(filename, to_file_base.absFileName(),
	                                    from, to_);
	converter_->connect(bind(&Impl::imageConverted, this, _1));
	converter_->startConversion();
}

} // namespace graphics
} // namespace lyx
