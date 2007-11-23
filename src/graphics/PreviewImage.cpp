/**
 * \file PreviewImage.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "PreviewImage.h"
#include "GraphicsImage.h"
#include "GraphicsLoader.h"
#include "PreviewLoader.h"

#include "support/FileName.h"
#include "support/lyxlib.h"

#include <boost/bind.hpp>

using std::string;


namespace lyx {

using support::FileName;

namespace graphics {

class PreviewImage::Impl : public boost::signals::trackable {
public:
	///
	Impl(PreviewImage & p, PreviewLoader & l,
	     string const & s, FileName const & f, double af);
	///
	~Impl();
	///
	Image const * image();
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
			   FileName const & f,
			   double af)
	: pimpl_(new Impl(*this, l, s, f, af))
{}


PreviewImage::~PreviewImage()
{
	delete pimpl_;
}


string const & PreviewImage::snippet() const
{
	return pimpl_->snippet_;
}


Dimension PreviewImage::dim() const
{
	Dimension dim;
	Image const * const image = pimpl_->iloader_.image();
	if (!image)
		return dim;

	dim.asc = int(pimpl_->ascent_frac_ * double(image->height()));
	dim.des = image->height() - dim.asc;
	dim.wid = image->width();
	return dim;
}


Image const * PreviewImage::image() const
{
	return pimpl_->image();
}


PreviewImage::Impl::Impl(PreviewImage & p, PreviewLoader & l,
			 string const & s,
			 FileName const & bf,
			 double af)
	: parent_(p), ploader_(l), iloader_(bf),
	  snippet_(s), ascent_frac_(af)
{
	iloader_.connect(boost::bind(&Impl::statusChanged, this));
}


PreviewImage::Impl::~Impl()
{
	support::unlink(iloader_.filename());
}


Image const * PreviewImage::Impl::image()
{
	if (iloader_.status() == WaitingToLoad)
		iloader_.startLoading();

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
		support::unlink(iloader_.filename());
		break;
	}
	ploader_.emitSignal(parent_);
}

} // namespace graphics
} // namespace lyx
