/**
 *  \file PreviewImage.C
 *  Copyright 2002 the LyX Team
 *  Read the file COPYING
 *
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "PreviewImage.h"
#include "PreviewLoader.h"
#include "GraphicsImage.h"
#include "GraphicsLoader.h"

#include "debug.h"

#include "support/lyxlib.h"

#include <boost/bind.hpp>
#include <boost/signals/trackable.hpp>


namespace grfx {

struct PreviewImage::Impl : public boost::signals::trackable {
	///
	Impl(PreviewImage & p, PreviewLoader & l,
	     string const & s, string const & f, double af);
	///
	void startLoading();	
	///
	Image const * image() const { return iloader_->image(); }
	///
	void statusChanged();

	///
	PreviewImage const & parent_;
	///
	PreviewLoader & ploader_;
	///
	boost::scoped_ptr<Loader> const iloader_;
	///
	string const snippet_;
	///
	double const ascent_frac_;
};


PreviewImage::PreviewImage(PreviewLoader & l,
			   string const & s,
			   string const & f,
			   double af)
	: pimpl_(new Impl(*this, l, s, f, af))
{}


PreviewImage::~PreviewImage()
{}


void PreviewImage::startLoading()
{
	return pimpl_->startLoading();
}


string const & PreviewImage::snippet() const
{
	return pimpl_->snippet_;
}

int PreviewImage::ascent() const
{
	Image const * const image = pimpl_->image();
	if (!image)
		return 0;

	return int(pimpl_->ascent_frac_ * double(image->getHeight()));
}


int PreviewImage::descent() const
{
	Image const * const image = pimpl_->image();
	if (!image)
		return 0;

	return int((1.0 - pimpl_->ascent_frac_) * double(image->getHeight()));
}


int PreviewImage::width() const
{
	Image const * const image = pimpl_->image();
	return image ? image->getWidth() : 0;
}


Image const * PreviewImage::image() const
{
	return pimpl_->image();
}


PreviewImage::Impl::Impl(PreviewImage & p, PreviewLoader & l,
			 string const & s,
			 string const & bf,
			 double af)
	: parent_(p), ploader_(l), iloader_(new Loader(bf)),
	  snippet_(s), ascent_frac_(af)
{}


void PreviewImage::Impl::startLoading()
{
	if (iloader_->status() != WaitingToLoad)
		return;

	iloader_->statusChanged.connect(
		boost::bind(&Impl::statusChanged, this));
	iloader_->startLoading();
}


void PreviewImage::Impl::statusChanged()
{
	switch (iloader_->status()) {
	case WaitingToLoad:
	case Loading:
	case Converting:
	case Loaded:
	case ScalingEtc:
		break;

	case ErrorNoFile:
	case ErrorConverting:
	case ErrorLoading:
	case ErrorGeneratingPixmap:
	case ErrorUnknown:
		//lyx::unlink(iloader_->filename());
		ploader_.remove(snippet_);
		break;

	case Ready:
		lyx::unlink(iloader_->filename());
		ploader_.imageReady(parent_);
		break;
	}
}

} // namespace grfx
