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

namespace grfx {

struct Loader::Impl {
	///
	Impl(Loader &, GParams const &);
	///
	~Impl();
	///
	void setFile(string const & file);
	///
	void unsetOldFile();
	///
	void createPixmap();
	///
	void statusChanged();

	///
	Loader & parent_;
	/// The loading status of the image.
	ImageStatus status_;
	/** Must store a copy of the cached item to ensure that it is not
	 *  erased unexpectedly by the cache itself.
	 */
	GraphicPtr graphic_;
	///
	GParams params_;
	/// We modify a local copy of the image once it is loaded.
	ImagePtr image_;
};


Loader::Impl::Impl(Loader & parent, GParams const & params)
	: parent_(parent), status_(WaitingToLoad), params_(params)
{}


Loader::Impl::~Impl()
{
	unsetOldFile();
}


void Loader::Impl::setFile(string const & file)
{
	if (file.empty())
		return;

	GCache & gc = GCache::get();
	if (!gc.inCache(file))
		gc.add(file);

	// We /must/ make a local copy of this.
	graphic_ = gc.graphic(file);
	status_ = graphic_->status();

	if (status_ == Loaded) {
		createPixmap();
	}

	// It's easiest to do this without checking
	parent_.statusChanged();
}


void Loader::Impl::unsetOldFile()
{
	if (!graphic_.get())
		return;

	string const old_file = graphic_->filename();
	graphic_.reset();
	GCache::get().remove(old_file);

	status_ = WaitingToLoad;
	params_ = GParams();
	image_.reset();
}


void Loader::Impl::statusChanged()
{
	status_ = graphic_->status();
	if (status_ == Loaded)
		createPixmap();

	parent_.statusChanged();
}


void Loader::Impl::createPixmap()
{
	if (!graphic_.get() || image_.get() ||
	    params_.display == NoDisplay || status_ != Loaded)
		return;

	image_.reset(graphic_->image()->clone());

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
	: pimpl_(new Impl(*this, GParams()))
{}


Loader::Loader(string const & file, DisplayType type)
	: pimpl_(new Impl(*this, GParams()))
{
	pimpl_->params_.display = type;
	pimpl_->setFile(file);
}


Loader::Loader(string const & file, GParams const & params)
	: pimpl_(new Impl(*this, params))
{
	pimpl_->setFile(file);
}


Loader::~Loader()
{}


void Loader::reset(string const & file, DisplayType type)
{
	pimpl_->unsetOldFile();

	pimpl_->params_ = GParams();
	pimpl_->params_.display = type;
	pimpl_->setFile(file);
}


void Loader::reset(string const & file, GParams const & params)
{
	pimpl_->unsetOldFile();

	pimpl_->params_ = params;
	pimpl_->setFile(file);
}


void Loader::reset(GParams const & params)
{
	pimpl_->params_ = params;

	if (pimpl_->status_ == Loaded)
		pimpl_->createPixmap();
}


void Loader::startLoading()
{
	if (pimpl_->status_ != WaitingToLoad || !pimpl_->graphic_.get())
		return;

	pimpl_->graphic_->statusChanged.connect(
		boost::bind(&Loader::Impl::statusChanged,
			    pimpl_.get()));
	pimpl_->graphic_->startLoading();
}


string const & Loader::filename() const
{
	static string const empty;
	return pimpl_->graphic_.get() ? pimpl_->graphic_->filename() : empty;
}


ImageStatus Loader::status() const
{
	return pimpl_->status_;
}


GImage const * Loader::image() const
{
	return pimpl_->image_.get();
}

} // namespace grfx
