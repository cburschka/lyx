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

#include "insets/render_preview.h"
#include "insets/inset.h"

#include "BufferView.h"
#include "dimension.h"
#include "gettext.h"
#include "LColor.h"
#include "lyx_main.h"
#include "metricsinfo.h"

#include "frontends/font_metrics.h"
#include "frontends/Painter.h"

#include "graphics/PreviewImage.h"
#include "graphics/PreviewLoader.h"
#include "graphics/Previews.h"

#include "support/lstrings.h"

#include <boost/bind.hpp>

using std::string;
using std::auto_ptr;

namespace graphics = lyx::graphics;
namespace support  = lyx::support;


bool RenderPreview::activated()
{
	return graphics::Previews::activated();
}


RenderPreview::RenderPreview(InsetBase const * inset)
	: pimage_(0),
	  parent_(inset)
{}


RenderPreview::RenderPreview(RenderPreview const & other,
			     InsetBase const * inset)
	: RenderBase(other),
	  boost::signals::trackable(),
	  snippet_(other.snippet_),
	  pimage_(0),
	  parent_(inset)
{}


auto_ptr<RenderBase> RenderPreview::clone(InsetBase const * inset) const
{
	return auto_ptr<RenderBase>(new RenderPreview(*this, inset));
}


namespace {

string const statusMessage(BufferView const * bv, string const & snippet)
{
	BOOST_ASSERT(bv && bv->buffer());

	Buffer const & buffer = *bv->buffer();
	graphics::Previews const & previews = graphics::Previews::get();
	graphics::PreviewLoader const & loader = previews.loader(buffer);
	graphics::PreviewLoader::Status const status = loader.status(snippet);

	string message;
	switch (status) {
	case graphics::PreviewLoader::InQueue:
	case graphics::PreviewLoader::Processing:
		message = _("Preview loading");
		break;
	case graphics::PreviewLoader::Ready:
		message = _("Preview ready");
		break;
	case graphics::PreviewLoader::NotFound:
		message = _("Preview failed");
		break;
	}

	return message;
}

} // namespace anon


void RenderPreview::metrics(MetricsInfo & mi, Dimension & dim) const
{
	if (previewReady()) {
		dim.asc = pimage_->ascent();
		dim.des = pimage_->descent();
		dim.wid = pimage_->width();
	} else {
		dim.asc = 50;
		dim.des = 0;

		LyXFont font(mi.base.font);
		font.setFamily(LyXFont::SANS_FAMILY);
		font.setSize(LyXFont::SIZE_FOOTNOTE);
		dim.wid = 15 +
			font_metrics::width(statusMessage(mi.base.bv, snippet_),
					    font);
	}

	dim_ = dim;
}


void RenderPreview::draw(PainterInfo & pi, int x, int y) const
{
	BOOST_ASSERT(pi.base.bv && pi.base.bv->buffer());
	Buffer const & buffer = *pi.base.bv->buffer();
	startLoading(buffer);

	if (previewReady()) {
		pi.pain.image(x, y - dim_.asc, dim_.wid, dim_.height(),
			      *(pimage_->image()));
		return;
	}

	pi.pain.rectangle(x + InsetOld::TEXT_TO_INSET_OFFSET,
			  y - dim_.asc,
			  dim_.wid - 2 * InsetOld::TEXT_TO_INSET_OFFSET,
			  dim_.asc + dim_.des,
			  LColor::foreground);

	LyXFont font(pi.base.font);
	font.setFamily(LyXFont::SANS_FAMILY);
	font.setSize(LyXFont::SIZE_FOOTNOTE);
	pi.pain.text(x + InsetOld::TEXT_TO_INSET_OFFSET + 6,
		     y - font_metrics::maxAscent(font) - 4,
		     statusMessage(pi.base.bv, snippet_), font);
}


void RenderPreview::startLoading(Buffer const & buffer) const
{
	if (!activated() && !snippet_.empty())
		return;

	graphics::Previews & previews = graphics::Previews::get();
	graphics::PreviewLoader & loader = previews.loader(buffer);
	loader.startLoading();
}


void RenderPreview::addPreview(string const & latex_snippet,
			       Buffer const & buffer)
{
	if (!activated())
		return;

	graphics::Previews & previews = graphics::Previews::get();
	graphics::PreviewLoader & loader = previews.loader(buffer);
	addPreview(latex_snippet, loader);
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
	LyX::cref().updateInset(parent_);
}


RenderMonitoredPreview::RenderMonitoredPreview(InsetBase const * inset)
	: RenderPreview(inset),
	  monitor_(std::string(), 2000)
{}


void RenderMonitoredPreview::setAbsFile(string const & file)
{
	monitor_.reset(file);
}


void RenderMonitoredPreview::draw(PainterInfo & pi, int x, int y) const
{
	RenderPreview::draw(pi, x, y);
	if (!monitoring())
		startMonitoring();
}


boost::signals::connection
RenderMonitoredPreview::fileChanged(slot_type const & slot)
{
	return monitor_.connect(slot);
}
