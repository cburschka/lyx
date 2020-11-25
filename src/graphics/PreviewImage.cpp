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

#include "Buffer.h"
#include "Dimension.h"
#include "GraphicsImage.h"
#include "GraphicsLoader.h"
#include "PreviewLoader.h"

#include "support/FileName.h"


using namespace std;
using namespace lyx::support;

namespace lyx {
namespace graphics {

class PreviewImage::Impl {
public:
	///
	Impl(PreviewImage & p, PreviewLoader & l,
	     string const & s, FileName const & f, double af);
	///
	~Impl();
	///
	Image const * image() const;
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


support::FileName const & PreviewImage::filename() const
{
	return pimpl_->iloader_.filename();
}


Dimension PreviewImage::dim() const
{
	Dimension dim;
	Image const * const image = pimpl_->iloader_.image();
	if (!image)
		return dim;

	dim.asc = int(pimpl_->ascent_frac_ * double(image->height()) + 0.5);
	dim.des = image->height() - dim.asc;
	dim.wid = image->width();
	return dim;
}


Image const * PreviewImage::image() const
{
	return pimpl_->image();
}


PreviewLoader & PreviewImage::previewLoader() const
{
	return pimpl_->ploader_;
}


PreviewImage::Impl::Impl(PreviewImage & p, PreviewLoader & l, string const & s,
                         FileName const & bf, double af)
	: parent_(p), ploader_(l), iloader_(l.buffer().fileName(), bf),
	  snippet_(s), ascent_frac_(af)
{
	iloader_.setDisplayPixelRatio(l.displayPixelRatio());
	// This connection is destroyed at the same time as this.
	iloader_.connect([this](){ statusChanged(); });
}


PreviewImage::Impl::~Impl()
{
	// If these images are generated for a clone, then that may be
	// because we are previewing. We therefore do not want to delete
	// them when this Buffer is destroyed.
	if (!ploader_.buffer().isClone())
		iloader_.filename().removeFile();
}


Image const * PreviewImage::Impl::image() const
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
		//iloader_.filename().removeFile();
		ploader_.remove(snippet_);
		// FIXME: We need to return here, because PreviewLoader::remove
		// removes the preview image from the cache, which deletes this
		// object, so we should not try to do anything here.
		return;

	case Ready:
		iloader_.filename().removeFile();
		break;
	}
	ploader_.emitSignal(parent_);
}

} // namespace graphics
} // namespace lyx
