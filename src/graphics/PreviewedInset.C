// -*- C++ -*-
/**
 *  \file PreviewedInset.C
 *  Read the file COPYING
 *
 * \author Angus Leeming 
 *
 * Full author contact details available in file CREDITS
 */

#ifdef __GNUG__
#pragma implementation
#endif

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

#include "debug.h"    // temporary

namespace grfx {

bool PreviewedInset::activated()
{
	return Previews::activated();
}


void PreviewedInset::generatePreview()
{
	if (!Previews::activated() || !previewWanted() ||
	    !view() || !view()->buffer())
		return;

	grfx::Previews & previews = grfx::Previews::get();
	grfx::PreviewLoader & loader = previews.loader(view()->buffer());
	addPreview(loader);
	if (!snippet_.empty())
		loader.startLoading();
}


void PreviewedInset::addPreview(grfx::PreviewLoader & ploader)
{
	if (!Previews::activated() || !previewWanted())
		return;

	setView(ploader.buffer().getUser());

	snippet_ = trim(latexString());
	if (snippet_.empty())
		return;

	pimage_ = ploader.preview(snippet_);
	if (pimage_)
		return;

	// If this is the first time of calling, connect to the
	// grfx::PreviewLoader signal that'll inform us when the preview image
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

	grfx::Previews & previews = grfx::Previews::get();
	grfx::PreviewLoader & loader = previews.loader(view()->buffer());
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
		grfx::PreviewLoader & ploader =
			grfx::Previews::get().loader(view()->buffer());
		pimage_ = ploader.preview(snippet_);
	}

	if (!pimage_)
		return false;

	return pimage_->image(inset_, *view());
}


void PreviewedInset::setView(BufferView * bv)
{
	if (!bv)
		return;
	
	view_ = bv->owner()->view();
}

void PreviewedInset::imageReady(grfx::PreviewImage const & pimage) const
{
	// Check snippet against the Inset's current contents
	if (snippet_ != pimage.snippet())
		return;

	pimage_ = &pimage;

	if (view())
		view()->updateInset(&inset_, false);
}

} // namespace grfx
