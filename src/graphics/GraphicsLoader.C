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

#include "BufferView.h"

#include "GraphicsCache.h"
#include "GraphicsCacheItem.h"
#include "GraphicsImage.h"
#include "GraphicsParams.h"
#include "GraphicsSupport.h"

#include "frontends/LyXView.h"
#include "frontends/Timeout.h"

#include <boost/weak_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/signals/trackable.hpp>

#include <list>

namespace grfx {

struct Loader::Impl : boost::signals::trackable {
	///
	Impl(Params const &);
	///
	~Impl();
	///
	void resetFile(string const &);
	///
	void resetParams(Params const &);
	///
	void createPixmap();

	///
	void startLoading(Inset const &, BufferView const &);

	/// The loading status of the image.
	ImageStatus status_;
	/** Must store a copy of the cached item to ensure that it is not
	 *  erased unexpectedly by the cache itself.
	 */
	Cache::ItemPtr cached_item_;
	/// We modify a local copy of the image once it is loaded.
	Image::ImagePtr image_;
	/// This signal is emitted when the image loading status changes.
	boost::signal0<void> signal_;

private:
	///
	void statusChanged();
	///
	void checkedLoading();

	///
	Params params_;

	///
	Timeout timer;
	// Multiple Insets can share the same image
	typedef std::list<Inset const *> InsetList;
	///
	InsetList insets;
	///
	boost::weak_ptr<BufferView const> view;
};


Loader::Loader()
	: pimpl_(new Impl(Params()))
{}


Loader::Loader(string const & file, DisplayType type)
	: pimpl_(new Impl(Params()))
{
	reset(file, type);
}


Loader::Loader(string const & file, Params const & params)
	: pimpl_(new Impl(params))
{
	reset(file, params);
}


Loader::~Loader()
{}


void Loader::reset(string const & file, DisplayType type) const
{
	Params params;
	params.display = type;
	pimpl_->resetParams(params);

	pimpl_->resetFile(file);
	pimpl_->createPixmap();
}


void Loader::reset(string const & file, Params const & params) const
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
	pimpl_->cached_item_->startLoading();
}


void Loader::startLoading(Inset const & inset, BufferView const & bv) const
{
	if (pimpl_->status_ != WaitingToLoad || !pimpl_->cached_item_.get())
		return;
	pimpl_->startLoading(inset, bv);
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


boost::signals::connection Loader::connect(slot_type const & slot) const
{
	return pimpl_->signal_.connect(slot);
}


Image const * Loader::image() const
{
	return pimpl_->image_.get();
}


Loader::Impl::Impl(Params const & params)
	: status_(WaitingToLoad), params_(params),
	  timer(2000, Timeout::ONETIME)
{
	timer.timeout.connect(boost::bind(&Impl::checkedLoading, this));
}


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

	// If monitoring() the current file, should continue to monitor the
	// new file.
	bool continue_monitoring = false;

	if (!old_file.empty()) {
		continue_monitoring = cached_item_->monitoring();
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

	cached_item_->connect(boost::bind(&Impl::statusChanged, this));
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


void Loader::Impl::startLoading(Inset const & inset, BufferView const & bv)
{
	if (status_ != WaitingToLoad || timer.running())
		return;

	InsetList::const_iterator it  = insets.begin();
	InsetList::const_iterator end = insets.end();
	it = std::find(it, end, &inset);
	if (it == end)
		insets.push_back(&inset);
	view = bv.owner()->view();

	timer.start();
}


namespace {

struct FindVisibleInset {

	FindVisibleInset(std::list<VisibleParagraph> const & vps) : vps_(vps) {}

	bool operator()(Inset const * inset_ptr)
	{
		if (!inset_ptr)
			return false;
		return isInsetVisible(*inset_ptr, vps_);
	}

private:
	std::list<VisibleParagraph> const & vps_;
};

} // namespace anon


void Loader::Impl::checkedLoading()
{
	if (insets.empty() || !view.get())
		return;

	std::list<VisibleParagraph> const vps =
		getVisibleParagraphs(*view.get());

	InsetList::const_iterator it  = insets.begin();
	InsetList::const_iterator end = insets.end();

	it = std::find_if(it, end, FindVisibleInset(vps));

	// One of the insets is visible, so start loading the image.
	if (it != end)
		cached_item_->startLoading();
}


} // namespace grfx
