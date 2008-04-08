/**
 * \file RenderPreview.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "insets/RenderPreview.h"
#include "insets/Inset.h"

#include "BufferView.h"
#include "Dimension.h"
#include "support/gettext.h"
#include "LyX.h"
#include "LyXRC.h"
#include "MetricsInfo.h"

#include "frontends/FontMetrics.h"
#include "frontends/Painter.h"

#include "graphics/PreviewImage.h"
#include "graphics/PreviewLoader.h"
#include "graphics/Previews.h"

#include "support/FileName.h"
#include "support/lstrings.h"

#include <boost/bind.hpp>

using namespace std;
using namespace lyx::support;

namespace lyx {


LyXRC_PreviewStatus RenderPreview::status()
{
	return graphics::Previews::status();
}


RenderPreview::RenderPreview(Inset const * inset)
	: parent_(inset)
{}


RenderPreview::RenderPreview(RenderPreview const & other,
			     Inset const * inset)
	: RenderBase(other),
	  boost::signals::trackable(),
	  snippet_(other.snippet_),
	  parent_(inset)
{}


RenderPreview::~RenderPreview()
{
	if (ploader_connection_.connected())
		ploader_connection_.disconnect();
}


RenderBase * RenderPreview::clone(Inset const * inset) const
{
	return new RenderPreview(*this, inset);
}


namespace {

graphics::PreviewLoader & getPreviewLoader(Buffer const & buffer)
{
	graphics::Previews const & previews = graphics::Previews::get();
	return previews.loader(buffer);
}


docstring const statusMessage(BufferView const * bv, string const & snippet)
{
	BOOST_ASSERT(bv);

	Buffer const & buffer = bv->buffer();
	graphics::PreviewLoader const & loader = getPreviewLoader(buffer);
	graphics::PreviewLoader::Status const status = loader.status(snippet);

	docstring message;
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


graphics::PreviewImage const *
RenderPreview::getPreviewImage(Buffer const & buffer) const
{
	graphics::PreviewLoader const & loader = getPreviewLoader(buffer);
	return loader.preview(snippet_);
}


void RenderPreview::metrics(MetricsInfo & mi, Dimension & dim) const
{
	BOOST_ASSERT(mi.base.bv);

	graphics::PreviewImage const * const pimage =
		getPreviewImage(mi.base.bv->buffer());

	if (pimage) {
		dim = pimage->dim();
	} else {
		dim.asc = 50;
		dim.des = 0;

		FontInfo font(mi.base.font);
		font.setFamily(SANS_FAMILY);
		font.setSize(FONT_SIZE_FOOTNOTE);
		docstring const stat = statusMessage(mi.base.bv, snippet_);
		dim.wid = 15 + theFontMetrics(font).width(stat);
	}

	dim_ = dim;
}


void RenderPreview::draw(PainterInfo & pi, int x, int y) const
{
	BOOST_ASSERT(pi.base.bv);

	graphics::PreviewImage const * const pimage =
		getPreviewImage(pi.base.bv->buffer());
	graphics::Image const * const image = pimage ? pimage->image() : 0;

	if (image) {
		pi.pain.image(x, y - dim_.asc, dim_.wid, dim_.height(),
			      *image);

	} else {
		int const offset = Inset::TEXT_TO_INSET_OFFSET;

		pi.pain.rectangle(x + offset,
				  y - dim_.asc,
				  dim_.wid - 2 * offset,
				  dim_.asc + dim_.des,
				  Color_foreground);

		FontInfo font(pi.base.font);
		font.setFamily(SANS_FAMILY);
		font.setSize(FONT_SIZE_FOOTNOTE);

		docstring const stat = statusMessage(pi.base.bv, snippet_);
		pi.pain.text(x + offset + 6,
			     y - theFontMetrics(font).maxAscent() - 4,
			     stat, font);
	}
}


void RenderPreview::startLoading(Buffer const & buffer) const
{
	if (status() == LyXRC::PREVIEW_OFF || snippet_.empty())
		return;

	graphics::PreviewLoader const & loader = getPreviewLoader(buffer);
	loader.startLoading();
}


void RenderPreview::addPreview(docstring const & latex_snippet,
			       Buffer const & buffer)
{
	if (status() == LyXRC::PREVIEW_OFF)
		return;

	graphics::PreviewLoader & loader = getPreviewLoader(buffer);
	addPreview(latex_snippet, loader);
}


void RenderPreview::addPreview(docstring const & latex_snippet,
			       graphics::PreviewLoader & ploader)
{
	if (status() == LyXRC::PREVIEW_OFF)
		return;

	// FIXME UNICODE
	// We have to make sure that we call latex with the right encoding
	snippet_ = support::trim(to_utf8(latex_snippet));
	if (snippet_.empty())
		return;

	if (ploader.preview(snippet_))
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

	graphics::PreviewLoader & loader = getPreviewLoader(buffer);
	loader.remove(snippet_);
	snippet_.erase();
}


void RenderPreview::imageReady(graphics::PreviewImage const & pimage)
{
	// Check the current snippet is the same as that previewed.
	if (snippet_ == pimage.snippet())
		parent_->updateFrontend();
}


RenderMonitoredPreview::RenderMonitoredPreview(Inset const * inset)
	: RenderPreview(inset), monitor_(FileName(), 2000)
{}


void RenderMonitoredPreview::setAbsFile(FileName const & file)
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

} // namespace lyx
