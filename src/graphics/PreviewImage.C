/**
 *  \file PreviewImage.C
 *  Copyright 2002 the LyX Team
 *  Read the file COPYING
 *
 * \author Angus Leeming <leeming@lyx.org>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "PreviewImage.h"
#include "PreviewLoader.h"
#include "GraphicsImage.h"
#include "GraphicsLoader.h"

#include "support/lyxlib.h"

#include <boost/bind.hpp>
#include <boost/signals/trackable.hpp>


namespace grfx {

struct PreviewImage::Impl : public boost::signals::trackable {
	///
	Impl(PreviewImage & p, PreviewLoader & l,
	     string const & s, string const & f, double af);
	///
	~Impl();
	///
	Image const * image(Inset const &, BufferView const &);
	///
	void statusChanged();

	///
	PreviewImage const & parent_;
	///
	PreviewLoader & ploader_;
	///
	Loader iloader_;
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


string const & PreviewImage::snippet() const
{
	return pimpl_->snippet_;
}


int PreviewImage::ascent() const
{
	Image const * const image = pimpl_->iloader_.image();
	if (!image)
		return 0;

	return int(pimpl_->ascent_frac_ * double(image->getHeight()));
}


int PreviewImage::descent() const
{
	Image const * const image = pimpl_->iloader_.image();
	if (!image)
		return 0;

	// Avoids rounding errors.
	return image->getHeight() - ascent();
}


int PreviewImage::width() const
{
	Image const * const image = pimpl_->iloader_.image();
	return image ? image->getWidth() : 0;
}


Image const * PreviewImage::image(Inset const & inset,
				  BufferView const & bv) const
{
	return pimpl_->image(inset, bv);
}


PreviewImage::Impl::Impl(PreviewImage & p, PreviewLoader & l,
			 string const & s,
			 string const & bf,
			 double af)
	: parent_(p), ploader_(l), iloader_(bf),
	  snippet_(s), ascent_frac_(af)
{
	iloader_.statusChanged.connect(
		boost::bind(&Impl::statusChanged, this));
}


PreviewImage::Impl::~Impl()
{
	lyx::unlink(iloader_.filename());
}


Image const * PreviewImage::Impl::image(Inset const & inset,
					BufferView const & bv)
{
	if (iloader_.status() == WaitingToLoad)
		iloader_.startLoading(inset, bv);

	return iloader_.image();
}


void PreviewImage::Impl::statusChanged()
{
	switch (iloader_.status()) {
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
		//lyx::unlink(iloader_.filename());
		ploader_.remove(snippet_);
		break;

	case Ready:
		lyx::unlink(iloader_.filename());
		ploader_.imageReady(parent_);
		break;
	}
}

} // namespace grfx
