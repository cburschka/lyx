// -*- C++ -*-
/**
 *  \file PreviewedInset.C
 *  Copyright 2002 the LyX Team
 *  Read the file COPYING
 *
 * \author Angus Leeming <leeming@lyx.org>
 */

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>

#include "PreviewedInset.h"

#include "BufferView.h"

#include "GraphicsImage.h"
#include "PreviewLoader.h"
#include "PreviewImage.h"
#include "Previews.h"

#include <boost/bind.hpp>


namespace grfx {

bool PreviewedInset::activated()
{
	return Previews::activated();
}


void PreviewedInset::generatePreview() const
{
	if (!Previews::activated() || !previewWanted() ||
	    !view() || !view()->buffer())
		return;

	grfx::Previews & previews = grfx::Previews::get();
	grfx::PreviewLoader & loader = previews.loader(view()->buffer());
	addPreview(loader);
	loader.startLoading();
}


void PreviewedInset::addPreview(grfx::PreviewLoader & ploader) const
{
	if (!Previews::activated() || !previewWanted())
		return;

	// Generate the LaTeX snippet.
	string const snippet = latexString();

	pimage_ = ploader.preview(snippet);
	if (pimage_)
		return;

	// If this is the first time of calling, connect to the
	// grfx::PreviewLoader signal that'll inform us when the preview image
	// is ready for loading.
	if (!connection_.connected()) {
		connection_ = ploader.connect(
			boost::bind(&PreviewedInset::imageReady, this, _1));
	}

	ploader.add(snippet);
}


bool PreviewedInset::previewReady() const
{
	if (!grfx::Previews::activated() || !previewWanted() ||
	    !view() || !view()->buffer())
		return false;

	// If the cached grfx::PreviewImage is invalid, update it.
	string const snippet = latexString();

	if (!pimage_ || snippet != pimage_->snippet()) {
		grfx::PreviewLoader & ploader =
			grfx::Previews::get().loader(view()->buffer());
		pimage_ = ploader.preview(snippet);
	}

	if (!pimage_)
		return false;

	return pimage_->image(inset_, *view());
}


void PreviewedInset::imageReady(grfx::PreviewImage const & pimage) const
{
	// Check snippet against the Inset's current contents
	if (latexString() != pimage.snippet())
		return;

	pimage_ = &pimage;
	if (view())
		view()->updateInset(&inset_, false);
}

} // namespace grfx
