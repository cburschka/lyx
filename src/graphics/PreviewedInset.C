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
#include "PreviewImage.h"
#include "PreviewLoader.h"
#include "Previews.h"

#include "support/lstrings.h"

#include <boost/bind.hpp>

namespace graphics = lyx::graphics;
namespace support  = lyx::support;


bool PreviewedInset::activated()
{
	return graphics::Previews::activated();
}


PreviewedInset::PreviewedInset()
	: pimage_(0)
{}


boost::signals::connection PreviewedInset::connect(slot_type const & slot)
{
	return preview_ready_signal_.connect(slot);
}


void PreviewedInset::generatePreview(Buffer const & buffer)
{
	if (!activated() || !previewWanted(buffer))
		return;

	graphics::Previews & previews = graphics::Previews::get();
	graphics::PreviewLoader & loader = previews.loader(buffer);
	addPreview(loader);
	if (!snippet_.empty())
		loader.startLoading();
}


void PreviewedInset::addPreview(graphics::PreviewLoader & ploader)
{
	if (!activated() || !previewWanted(ploader.buffer()))
		return;

	snippet_ = support::trim(latexString(ploader.buffer()));
	if (snippet_.empty())
		return;

	pimage_ = ploader.preview(snippet_);
	if (pimage_)
		return;

	// If this is the first time of calling, connect to the
	// PreviewLoader signal that'll inform us when the preview image
	// is ready for loading.
	if (!ploader_connection_.connected()) {
		ploader_connection_ = ploader.connect(
			boost::bind(&PreviewedInset::imageReady, this, _1));
	}

	ploader.add(snippet_);
}


void PreviewedInset::removePreview(Buffer const & buffer)
{
	if (snippet_.empty())
		return;

	graphics::Previews & previews = graphics::Previews::get();
	graphics::PreviewLoader & loader = previews.loader(buffer);
	loader.remove(snippet_);
	snippet_.erase();
	pimage_ = 0;
}


bool PreviewedInset::previewReady() const
{
	return pimage_ ? pimage_->image() : false;
}


void PreviewedInset::imageReady(graphics::PreviewImage const & pimage) const
{
	// Check the current snippet is the same as that previewed.
	if (snippet_ != pimage.snippet())
		return;

	pimage_ = &pimage;
	preview_ready_signal_();
}
