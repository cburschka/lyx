/**
 * \file render_graphic.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "render_graphic.h"

#include "insets/inset.h"

#include "gettext.h"
#include "LColor.h"
#include "lyx_main.h"
#include "lyxrc.h"
#include "metricsinfo.h"

#include "frontends/font_metrics.h"
#include "frontends/Painter.h"

#include "graphics/GraphicsImage.h"

#include "support/filetools.h"

#include <boost/bind.hpp>

namespace graphics = lyx::graphics;

using lyx::support::AbsolutePath;
using lyx::support::OnlyFilename;

using std::string;
using std::auto_ptr;


RenderGraphic::RenderGraphic(InsetBase const * inset)
{
	loader_.connect(boost::bind(&LyX::updateInset,
				    boost::cref(LyX::cref()), inset));
}


RenderGraphic::RenderGraphic(RenderGraphic const & other,
			     InsetBase const * inset)
	: RenderBase(other),
	  loader_(other.loader_),
	  params_(other.params_)
{
	loader_.connect(boost::bind(&LyX::updateInset,
				    boost::cref(LyX::cref()), inset));
}


auto_ptr<RenderBase> RenderGraphic::clone(InsetBase const * inset) const
{
	return auto_ptr<RenderBase>(new RenderGraphic(*this, inset));
}


void RenderGraphic::update(graphics::Params const & params)
{
	params_ = params;

	if (!params_.filename.empty()) {
		BOOST_ASSERT(AbsolutePath(params_.filename));
		loader_.reset(params_.filename, params_);
	}
}


namespace {

bool displayGraphic(graphics::Params const & params)
{
	return params.display != graphics::NoDisplay &&
		lyxrc.display_graphics != graphics::NoDisplay;
}


string const statusMessage(graphics::Params const & params,
			   graphics::ImageStatus status)
{
	if (!displayGraphic(params))
		return _("Not shown.");

	switch (status) {
	case graphics::WaitingToLoad:
		return _("Not shown.");
	case graphics::Loading:
		return _("Loading...");
	case graphics::Converting:
		return _("Converting to loadable format...");
	case graphics::Loaded:
		return _("Loaded into memory. Generating pixmap...");
	case graphics::ScalingEtc:
		return _("Scaling etc...");
	case graphics::Ready:
		return _("Ready to display");
	case graphics::ErrorNoFile:
		return _("No file found!");
	case graphics::ErrorConverting:
		return _("Error converting to loadable format");
	case graphics::ErrorLoading:
		return _("Error loading file into memory");
	case graphics::ErrorGeneratingPixmap:
		return _("Error generating the pixmap");
	case graphics::ErrorUnknown:
		return _("No image");
	}
	return string();
}


bool readyToDisplay(graphics::Loader const & loader)
{
	if (!loader.image() || loader.status() != graphics::Ready)
		return false;
	return loader.image()->isDrawable();
}

} // namespace anon


void RenderGraphic::metrics(MetricsInfo & mi, Dimension & dim) const
{
	bool image_ready = displayGraphic(params_) && readyToDisplay(loader_);

	dim.asc = image_ready ? loader_.image()->getHeight() : 50;
	dim.des = 0;

	if (image_ready) {
		dim.wid = loader_.image()->getWidth() +
			2 * InsetOld::TEXT_TO_INSET_OFFSET;
	} else {
		int font_width = 0;

		LyXFont msgFont(mi.base.font);
		msgFont.setFamily(LyXFont::SANS_FAMILY);

		string const justname = OnlyFilename(params_.filename);
		if (!justname.empty()) {
			msgFont.setSize(LyXFont::SIZE_FOOTNOTE);
			font_width = font_metrics::width(justname, msgFont);
		}

		string const msg = statusMessage(params_, loader_.status());
		if (!msg.empty()) {
			msgFont.setSize(LyXFont::SIZE_TINY);
			font_width = std::max(font_width,
					      font_metrics::width(msg, msgFont));
		}

		dim.wid = std::max(50, font_width + 15);
	}

	dim_ = dim;
}


void RenderGraphic::draw(PainterInfo & pi, int x, int y) const
{
	if (displayGraphic(params_)) {
		if (loader_.status() == graphics::WaitingToLoad)
			loader_.startLoading();
		if (!loader_.monitoring())
			loader_.startMonitoring();
	}

	// This will draw the graphics. If the graphics has not been
	// loaded yet, we draw just a rectangle.

	if (displayGraphic(params_) && readyToDisplay(loader_)) {
		pi.pain.image(x + InsetOld::TEXT_TO_INSET_OFFSET,
			      y - dim_.asc,
			      dim_.wid - 2 * InsetOld::TEXT_TO_INSET_OFFSET,
			      dim_.asc + dim_.des,
			      *loader_.image());

	} else {
		pi.pain.rectangle(x + InsetOld::TEXT_TO_INSET_OFFSET,
				  y - dim_.asc,
				  dim_.wid - 2 * InsetOld::TEXT_TO_INSET_OFFSET,
				  dim_.asc + dim_.des,
				  LColor::foreground);

		// Print the file name.
		LyXFont msgFont = pi.base.font;
		msgFont.setFamily(LyXFont::SANS_FAMILY);
		string const justname = OnlyFilename(params_.filename);

		if (!justname.empty()) {
			msgFont.setSize(LyXFont::SIZE_FOOTNOTE);
			pi.pain.text(x + InsetOld::TEXT_TO_INSET_OFFSET + 6,
				   y - font_metrics::maxAscent(msgFont) - 4,
				   justname, msgFont);
		}

		// Print the message.
		string const msg = statusMessage(params_, loader_.status());
		if (!msg.empty()) {
			msgFont.setSize(LyXFont::SIZE_TINY);
			pi.pain.text(x + InsetOld::TEXT_TO_INSET_OFFSET + 6,
				     y - 4, msg, msgFont);
		}
	}
}
