/**
 * \file render_preview.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "render_preview.h"

#include "dimension.h"
#include "LColor.h"
#include "metricsinfo.h"

#include "frontends/Painter.h"

#include "graphics/PreviewImage.h"
#include "graphics/PreviewLoader.h"
#include "graphics/Previews.h"

#include "support/lstrings.h"

#include <boost/bind.hpp>

using std::string;

namespace graphics = lyx::graphics;
namespace support  = lyx::support;


bool RenderPreview::activated()
{
	return graphics::Previews::activated();
}


RenderPreview::RenderPreview()
	: pimage_(0)
{}


RenderPreview::RenderPreview(RenderPreview const & other)
	: RenderBase(other),
	  boost::signals::trackable(),
	  snippet_(other.snippet_),
	  pimage_(0)
{}


RenderBase * RenderPreview::clone() const
{
	return new RenderPreview(*this);
}


void RenderPreview::metrics(MetricsInfo &, Dimension & dim) const
{
	if (previewReady()) {
		dim.asc = pimage()->ascent();
		dim.des = pimage()->descent();
		dim.wid = pimage()->width();
	} else {
		dim.asc = 20;
		dim.des = 20;
		dim.wid = 20;
	}

	dim_ = dim;
}


void RenderPreview::draw(PainterInfo & pi, int x, int y) const
{
	BOOST_ASSERT(pi.base.bv);
	view_ = pi.base.bv->owner()->view();

	if (!previewReady())
		pi.pain.rectangle(x, y - dim_.asc, dim_.wid, dim_.height(),
				  LColor::foreground);
	else
		pi.pain.image(x, y - dim_.asc, dim_.wid, dim_.height(),
			      *(pimage()->image()));
}


boost::signals::connection RenderPreview::connect(slot_type const & slot)
{
	return preview_ready_signal_.connect(slot);
}


void RenderPreview::generatePreview(string const & latex_snippet,
				     Buffer const & buffer)
{
	if (!activated())
		return;

	graphics::Previews & previews = graphics::Previews::get();
	graphics::PreviewLoader & loader = previews.loader(buffer);
	addPreview(latex_snippet, loader);
	if (!snippet_.empty())
		loader.startLoading();
}


void RenderPreview::addPreview(string const & latex_snippet,
				graphics::PreviewLoader & ploader)
{
	if (!activated())
		return;

	snippet_ = support::trim(latex_snippet);
	pimage_ = 0;
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
			boost::bind(&RenderPreview::imageReady, this, _1));
	}

	ploader.add(snippet_);
}


void RenderPreview::removePreview(Buffer const & buffer)
{
	if (snippet_.empty())
		return;

	graphics::Previews & previews = graphics::Previews::get();
	graphics::PreviewLoader & loader = previews.loader(buffer);
	loader.remove(snippet_);
	snippet_.erase();
	pimage_ = 0;
}


bool RenderPreview::previewReady() const
{
	return pimage_ ? pimage_->image() : false;
}


void RenderPreview::imageReady(graphics::PreviewImage const & pimage)
{
	// Check the current snippet is the same as that previewed.
	if (snippet_ != pimage.snippet())
		return;

	pimage_ = &pimage;
	preview_ready_signal_();
}


void RenderMonitoredPreview::startMonitoring(string const & file)
{
	monitor_.reset(file);
	monitor_.start();
}


boost::signals::connection
RenderMonitoredPreview::fileChanged(slot_type const & slot)
{
	return monitor_.connect(slot);
}
