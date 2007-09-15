/**
 * \file GraphicsLoader.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GraphicsLoader.h"

#include "GraphicsCacheItem.h"
#include "GraphicsImage.h"
#include "GraphicsParams.h"
#include "LoaderQueue.h"

#include <boost/bind.hpp>


using std::string;


namespace lyx {

using support::FileName;

namespace graphics {

class Loader::Impl : public boost::signals::trackable {
public:
	///
	Impl();
	///
	~Impl();
	///
	void resetFile(FileName const &);
	///
	void resetParams(Params const &);
	///
	void createPixmap();
	///
	void startLoading();
	///
	Params const & params() const { return params_; }

	/// The loading status of the image.
	ImageStatus status_;
	/** Must store a copy of the cached item to ensure that it is not
	 *  erased unexpectedly by the cache itself.
	 */
	Cache::ItemPtr cached_item_;
	/// We modify a local copy of the image once it is loaded.
	Image::ImagePtr image_;
	/// This signal is emitted when the image loading status changes.
	boost::signal<void()> signal_;
	/// The connection of the signal StatusChanged 	
	boost::signals::connection sc_;

private:
	///
	void statusChanged();
	///
	void checkedLoading();

	///
	Params params_;
};


Loader::Loader()
	: pimpl_(new Impl)
{}


Loader::Loader(FileName const & file, DisplayType type)
	: pimpl_(new Impl)
{
	reset(file, type);
}


Loader::Loader(FileName const & file, Params const & params)
	: pimpl_(new Impl)
{
	reset(file, params);
}


Loader::Loader(Loader const & other)
	: pimpl_(new Impl)
{
	Params const & params = other.pimpl_->params();
	reset(params.filename, params);
}


Loader::~Loader()
{}


Loader & Loader::operator=(Loader const & other)
{
	if (this != &other) {
		Params const & params = other.pimpl_->params();
		reset(params.filename, params);
	}
	return *this;
}


void Loader::reset(FileName const & file, DisplayType type) const
{
	Params params;
	params.display = type;
	pimpl_->resetParams(params);

	pimpl_->resetFile(file);
	pimpl_->createPixmap();
}


void Loader::reset(FileName const & file, Params const & params) const
{
	pimpl_->resetParams(params);
	pimpl_->resetFile(file);
	pimpl_->createPixmap();
}


void Loader::reset(Params const & params) const
{
	pimpl_->resetParams(params);
	pimpl_->createPixmap();
}


void Loader::startLoading() const
{
	if (pimpl_->status_ != WaitingToLoad || !pimpl_->cached_item_.get())
		return;
	pimpl_->startLoading();
}


void Loader::startMonitoring() const
{
	if (!pimpl_->cached_item_.get())
		return;

	pimpl_->cached_item_->startMonitoring();
}


bool Loader::monitoring() const
{
	if (!pimpl_->cached_item_.get())
		return false;

	return pimpl_->cached_item_->monitoring();
}


unsigned long Loader::checksum() const
{
	if (!pimpl_->cached_item_.get())
		return 0;

	return pimpl_->cached_item_->checksum();
}


FileName const & Loader::filename() const
{
	static FileName const empty;
	return pimpl_->cached_item_.get() ?
		pimpl_->cached_item_->filename() : empty;
}


ImageStatus Loader::status() const
{
	return pimpl_->status_;
}


boost::signals::connection Loader::connect(slot_type const & slot) const
{
	return pimpl_->signal_.connect(slot);
}


Image const * Loader::image() const
{
	return pimpl_->image_.get();
}


Loader::Impl::Impl()
	: status_(WaitingToLoad)
{
}


Loader::Impl::~Impl()
{
	resetFile(FileName());
}


void Loader::Impl::resetFile(FileName const & file)
{
	FileName const old_file = cached_item_.get() ?
		cached_item_->filename() : FileName();

	if (file == old_file)
		return;

	// If monitoring() the current file, should continue to monitor the
	// new file.
	bool continue_monitoring = false;

	if (!old_file.empty()) {
		continue_monitoring = cached_item_->monitoring();
		// cached_item_ is going to be reset, so the connected
		// signal needs to be disconnected.
		sc_.disconnect();
		cached_item_.reset();
		Cache::get().remove(old_file);
	}

	status_ = cached_item_.get() ? cached_item_->status() : WaitingToLoad;
	image_.reset();

	if (cached_item_.get() || file.empty())
		return;

	Cache & gc = Cache::get();
	if (!gc.inCache(file))
		gc.add(file);

	// We /must/ make a local copy of this.
	cached_item_ = gc.item(file);
	status_ = cached_item_->status();

	if (continue_monitoring && !cached_item_->monitoring())
		cached_item_->startMonitoring();

	sc_ = cached_item_->connect(boost::bind(&Impl::statusChanged, this));
}


void Loader::Impl::resetParams(Params const & params)
{
	if (params == params_)
		return;

	params_ = params;
	status_ = cached_item_.get() ? cached_item_->status() : WaitingToLoad;
	image_.reset();
}


void Loader::Impl::statusChanged()
{
	status_ = cached_item_.get() ? cached_item_->status() : WaitingToLoad;
	createPixmap();
	signal_();
}


void Loader::Impl::createPixmap()
{
	if (!cached_item_.get() ||
	    params_.display == NoDisplay || status_ != Loaded)
		return;

	image_.reset(cached_item_->image()->clone());

	// These do nothing if there's nothing to do
	image_->clip(params_);
	image_->rotate(params_);
	image_->scale(params_);

	bool const success = image_->setPixmap(params_);

	if (success) {
		status_ = Ready;
	} else {
		image_.reset();
		status_ = ErrorGeneratingPixmap;
	}
}

void Loader::Impl::startLoading()
{
	if (status_ != WaitingToLoad)
		return;

	LoaderQueue::get().touch(cached_item_);
}


} // namespace graphics
} // namespace lyx
