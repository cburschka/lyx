/**
 * \file PreviewedInset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "PreviewedInset.h"
#include "GraphicsImage.h"
#include "PreviewLoader.h"
#include "PreviewImage.h"
#include "Previews.h"

#include "buffer.h"
#include "BufferView.h"

#include "frontends/LyXView.h"

#include "support/lstrings.h"

#include <boost/bind.hpp>

namespace support = lyx::support;


namespace lyx {
namespace graphics {

bool PreviewedInset::activated()
{
	return Previews::activated();
}


BufferView * PreviewedInset::view() const
{
	return inset_.view();
}


void PreviewedInset::generatePreview()
{
	if (!Previews::activated() || !previewWanted() ||
	    !view() || !view()->buffer())
		return;

	Previews & previews = Previews::get();
	PreviewLoader & loader = previews.loader(*view()->buffer());
	addPreview(loader);
	if (!snippet_.empty())
		loader.startLoading();
}


void PreviewedInset::addPreview(PreviewLoader & ploader)
{
	if (!Previews::activated() || !previewWanted())
		return;

	snippet_ = support::trim(latexString());
	if (snippet_.empty())
		return;

	pimage_ = ploader.preview(snippet_);
	if (pimage_)
		return;

	// If this is the first time of calling, connect to the
	// PreviewLoader signal that'll inform us when the preview image
	// is ready for loading.
	if (!connection_.connected()) {
		connection_ = ploader.connect(
			boost::bind(&PreviewedInset::imageReady, this, _1));
	}

	ploader.add(snippet_);
}


void PreviewedInset::removePreview()
{
	if (!view() || !view()->buffer() || snippet_.empty())
		return;

	Previews & previews = Previews::get();
	PreviewLoader & loader = previews.loader(*view()->buffer());
	loader.remove(snippet_);
	snippet_.erase();
	pimage_ = 0;
}


bool PreviewedInset::previewReady() const
{
	if (!Previews::activated() || !previewWanted() ||
	    !view() || !view()->buffer())
		return false;

	if (!pimage_ || snippet_ != pimage_->snippet()) {
		PreviewLoader & ploader =
			Previews::get().loader(*view()->buffer());
		pimage_ = ploader.preview(snippet_);
	}

	if (!pimage_)
		return false;

	return pimage_->image();
}


void PreviewedInset::imageReady(PreviewImage const & pimage) const
{
	// Check snippet against the Inset's current contents
	if (snippet_ != pimage.snippet())
		return;

	pimage_ = &pimage;

	if (view())
		view()->updateInset(inset());
}

} // namespace graphics
} // namespace lyx
