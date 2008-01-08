/**
 * \file RenderGraphic.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "RenderGraphic.h"

#include "insets/Inset.h"

#include "support/FileName.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "LyX.h"
#include "LyXRC.h"
#include "MetricsInfo.h"

#include "frontends/FontMetrics.h"
#include "frontends/Painter.h"

#include "graphics/GraphicsImage.h"

#include "support/filetools.h"

#include <boost/bind.hpp>

using namespace std;

namespace lyx {


RenderGraphic::RenderGraphic(Inset const * inset)
{
	loader_.connect(boost::bind(&Inset::updateFrontend, inset));
	icon_.connect(boost::bind(&Inset::updateFrontend, inset));
}


RenderGraphic::RenderGraphic(RenderGraphic const & other, Inset const * inset)
	: RenderBase(other), loader_(other.loader_), icon_(other.icon_),
		params_(other.params_)
{
	loader_.connect(boost::bind(&Inset::updateFrontend, inset));
	icon_.connect(boost::bind(&Inset::updateFrontend, inset));
}


RenderBase * RenderGraphic::clone(Inset const * inset) const
{
	return new RenderGraphic(*this, inset);
}


void RenderGraphic::update(graphics::Params const & params)
{
	params_ = params;

	if (!params_.filename.empty())
		loader_.reset(params_.filename, params_);
	// If icon is set to empty, icon_ will not be reset to empty
	// but will not be displayed. This is to avoid repeated loading
	// of the same icon when figure status changes.
	if (!params_.icon.empty()) {
		support::FileName const icon = support::libFileSearch("images/",
			params_.icon, "png");
		if (!icon.empty()) // using an empty bounding box
			icon_.reset(icon, graphics::Params());
	}
}


namespace {

bool displayGraphic(graphics::Params const & params)
{
	return params.display != graphics::NoDisplay &&
		lyxrc.display_graphics != graphics::NoDisplay;
}


docstring const statusMessage(graphics::Params const & params,
			   graphics::ImageStatus status)
{
	docstring ret;

	if (!displayGraphic(params))
		ret = _("Not shown.");
	else {
		switch (status) {
		case graphics::WaitingToLoad:
			ret = _("Not shown.");
			break;
		case graphics::Loading:
			ret = _("Loading...");
			break;
		case graphics::Converting:
			ret = _("Converting to loadable format...");
			break;
		case graphics::Loaded:
			ret = _("Loaded into memory. Generating pixmap...");
			break;
		case graphics::ScalingEtc:
			ret = _("Scaling etc...");
			break;
		case graphics::Ready:
			ret = _("Ready to display");
			break;
		case graphics::ErrorNoFile:
			ret = _("No file found!");
			break;
		case graphics::ErrorConverting:
			ret = _("Error converting to loadable format");
			break;
		case graphics::ErrorLoading:
			ret = _("Error loading file into memory");
			break;
		case graphics::ErrorGeneratingPixmap:
			ret = _("Error generating the pixmap");
			break;
		case graphics::ErrorUnknown:
			ret = _("No image");
			break;
		}
	}

	return ret;
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

	dim.asc = image_ready ? loader_.image()->height() : 50;
	dim.des = 0;

	if (image_ready) {
		dim.wid = loader_.image()->width() + 2 * Inset::TEXT_TO_INSET_OFFSET;
	} else {
		int font_width = 0;

		FontInfo msgFont(mi.base.font);
		msgFont.setFamily(SANS_FAMILY);

		// FIXME UNICODE
		docstring const justname = from_utf8(params_.filename.onlyFileName());
		if (!justname.empty()) {
			msgFont.setSize(FONT_SIZE_FOOTNOTE);
			font_width = theFontMetrics(msgFont).width(justname);
		}

		docstring const msg = statusMessage(params_, loader_.status());
		if (!msg.empty()) {
			msgFont.setSize(FONT_SIZE_TINY);
			font_width = max(font_width,
				theFontMetrics(msgFont).width(msg));
		}

		dim.wid = max(50, font_width + 15);
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
		if (icon_.status() == graphics::WaitingToLoad)
			icon_.startLoading();
		if (!icon_.monitoring())
			icon_.startMonitoring();
	}

	// This will draw the graphics. If the graphics has not been
	// loaded yet, we draw just a rectangle.

	if (displayGraphic(params_) && readyToDisplay(loader_)) {
		pi.pain.image(x + Inset::TEXT_TO_INSET_OFFSET,
			      y - dim_.asc,
			      dim_.wid - 2 * Inset::TEXT_TO_INSET_OFFSET,
			      dim_.asc + dim_.des,
			      *loader_.image());

	} else {
		pi.pain.rectangle(x + Inset::TEXT_TO_INSET_OFFSET,
				  y - dim_.asc,
				  dim_.wid - 2 * Inset::TEXT_TO_INSET_OFFSET,
				  dim_.asc + dim_.des,
				  Color_foreground);

		// Print the file name.
		FontInfo msgFont = pi.base.font;
		msgFont.setFamily(SANS_FAMILY);
		string const justname = params_.filename.onlyFileName();

		if (!justname.empty()) {
			msgFont.setSize(FONT_SIZE_FOOTNOTE);
			pi.pain.text(x + Inset::TEXT_TO_INSET_OFFSET + 6,
				   y - theFontMetrics(msgFont).maxAscent() - 4,
				   from_utf8(justname), msgFont);
		}

		// Print the message.
		docstring const msg = statusMessage(params_, loader_.status());
		if (!msg.empty()) {
			msgFont.setSize(FONT_SIZE_TINY);
			pi.pain.text(x + Inset::TEXT_TO_INSET_OFFSET + 6,
				     y - 4, msg, msgFont);
		}
	}
	if (!params_.icon.empty() && readyToDisplay(icon_))
		pi.pain.image(x + Inset::TEXT_TO_INSET_OFFSET, y - dim_.asc, 
			10, 10, *icon_.image());
}


} // namespace lyx
