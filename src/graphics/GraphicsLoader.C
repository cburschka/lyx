/*
 * \file GraphicsLoader.C
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author Angus Leeming <leeming@lyx.org>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "GraphicsLoader.h"
#include "GraphicsCache.h"
#include "GraphicsCacheItem.h"
#include "GraphicsImage.h"
#include "GraphicsParams.h"

#include <boost/bind.hpp>
#include <boost/signals/trackable.hpp>

namespace grfx {

struct Loader::Impl : boost::signals::trackable {
	///
	Impl(Loader &, Params const &);
	///
	~Impl();
	///
	void resetFile(string const &);
	///
	void resetParams(Params const &);
	///
	void createPixmap();

	/// The loading status of the image.
	ImageStatus status_;
	/** Must store a copy of the cached item to ensure that it is not
	 *  erased unexpectedly by the cache itself.
	 */
	Cache::ItemPtr cached_item_;
	/// We modify a local copy of the image once it is loaded.
	Image::ImagePtr image_;

private:
	///
	void statusChanged();
	
	///
	Params params_;
	///
	Loader & parent_;
};


Loader::Impl::Impl(Loader & parent, Params const & params)
	: status_(WaitingToLoad), params_(params), parent_(parent)
{}


Loader::Impl::~Impl()
{
	resetFile(string());
}


void Loader::Impl::resetFile(string const & file)
{
	string const old_file = cached_item_.get() ?
		cached_item_->filename() : string();

	if (file == old_file)
		return;

	if (!old_file.empty()) {
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

	cached_item_->statusChanged.connect(
		boost::bind(&Impl::statusChanged, this));
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
	parent_.statusChanged();
}


void Loader::Impl::createPixmap()
{
	if (!cached_item_.get() || image_.get() ||
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


Loader::Loader()
	: pimpl_(new Impl(*this, Params()))
{}


Loader::Loader(string const & file, DisplayType type)
	: pimpl_(new Impl(*this, Params()))
{
	reset(file, type);
}


Loader::Loader(string const & file, Params const & params)
	: pimpl_(new Impl(*this, params))
{
	reset(file, params);
}


Loader::~Loader()
{}


void Loader::reset(string const & file, DisplayType type)
{
	Params params;
	params.display = type;
	pimpl_->resetParams(params);

	pimpl_->resetFile(file);
	pimpl_->createPixmap();
}


void Loader::reset(string const & file, Params const & params)
{
	pimpl_->resetParams(params);
	pimpl_->resetFile(file);
	pimpl_->createPixmap();
}


void Loader::reset(Params const & params)
{
	pimpl_->resetParams(params);
	pimpl_->createPixmap();
}


void Loader::startLoading()
{
	if (pimpl_->status_ != WaitingToLoad || !pimpl_->cached_item_.get())
		return;
	pimpl_->cached_item_->startLoading();
}


string const & Loader::filename() const
{
	static string const empty;
	return pimpl_->cached_item_.get() ?
		pimpl_->cached_item_->filename() : empty;
}


ImageStatus Loader::status() const
{
	return pimpl_->status_;
}


Image const * Loader::image() const
{
	return pimpl_->image_.get();
}

} // namespace grfx
