/**
 * \file renderers.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "insets/renderers.h"

#include "buffer.h"
#include "BufferView.h"
#include "gettext.h"
#include "metricsinfo.h"

#include "frontends/font_metrics.h"
#include "frontends/LyXView.h"
#include "frontends/Painter.h"

#include "graphics/GraphicsImage.h"

#include "support/filetools.h"


using namespace lyx::support;

namespace grfx = lyx::graphics;

RenderInset::RenderInset()
{}


RenderInset::RenderInset(RenderInset const &)
{
	// Cached variables are not copied
}


RenderInset::~RenderInset()
{}


RenderInset & RenderInset::operator=(RenderInset const &)
{
	// Cached variables are not copied
	return *this;
}


BufferView * RenderInset::view() const
{
	return view_.lock().get();
}


ButtonRenderer::ButtonRenderer()
	: editable_(false)
{}


RenderInset * ButtonRenderer::clone() const
{
	return new ButtonRenderer(*this);
}


void ButtonRenderer::update(string const & text, bool editable)
{
	text_ = text;
	editable_ = editable;
}


void ButtonRenderer::metrics(MetricsInfo & mi, Dimension & dim) const
{
	Assert(mi.base.bv);

	LyXFont font(LyXFont::ALL_SANE);
	font.decSize();

	if (editable_)
		font_metrics::buttonText(text_, font, dim.wid, dim.asc, dim.des);
	else
		font_metrics::rectText(text_, font, dim.wid, dim.asc, dim.des);

	dim.wid += 4;
}


void ButtonRenderer::draw(PainterInfo & pi, int x, int y) const
{
	Assert(pi.base.bv);
	view_ = pi.base.bv->owner()->view();

	// Draw it as a box with the LaTeX text
	LyXFont font(LyXFont::ALL_SANE);
	font.setColor(LColor::command).decSize();

	if (editable_) {
		pi.pain.buttonText(x + 2, y, text_, font);
	} else {
		pi.pain.rectText(x + 2, y, text_, font,
			      LColor::commandbg, LColor::commandframe);
	}
}


GraphicRenderer::GraphicRenderer()
	: checksum_(0)
{}


GraphicRenderer::GraphicRenderer(GraphicRenderer const & other)
	: RenderInset(other),
	  loader_(other.loader_),
	  params_(other.params_),
	  checksum_(0)
{}


RenderInset * GraphicRenderer::clone() const
{
	return new GraphicRenderer(*this);
}


void GraphicRenderer::update(grfx::Params const & params)
{
	params_ = params;

	if (!params_.filename.empty()) {
		Assert(AbsolutePath(params_.filename));
		loader_.reset(params_.filename, params_);
	}
}


bool GraphicRenderer::hasFileChanged() const
{
	unsigned long const new_checksum = loader_.checksum();
	bool const file_has_changed = checksum_ != new_checksum;
	if (file_has_changed)
		checksum_ = new_checksum;
	return file_has_changed;
}


boost::signals::connection GraphicRenderer::connect(slot_type const & slot) const
{
	return loader_.connect(slot);
}


string const GraphicRenderer::statusMessage() const
{
	switch (loader_.status()) {
		case grfx::WaitingToLoad:
			return _("Not shown.");
		case grfx::Loading:
			return _("Loading...");
		case grfx::Converting:
			return _("Converting to loadable format...");
		case grfx::Loaded:
			return _("Loaded into memory. Must now generate pixmap.");
		case grfx::ScalingEtc:
			return _("Scaling etc...");
		case grfx::Ready:
			return _("Ready to display");
		case grfx::ErrorNoFile:
			return _("No file found!");
		case grfx::ErrorConverting:
			return _("Error converting to loadable format");
		case grfx::ErrorLoading:
			return _("Error loading file into memory");
		case grfx::ErrorGeneratingPixmap:
			return _("Error generating the pixmap");
		case grfx::ErrorUnknown:
			return _("No image");
	}
	return string();
}


bool GraphicRenderer::readyToDisplay() const
{
	if (!loader_.image() || loader_.status() != grfx::Ready)
		return false;
	return loader_.image()->isDrawable();
}


void GraphicRenderer::metrics(MetricsInfo & mi, Dimension & dim) const
{
	bool image_ready = readyToDisplay();

	dim.asc = image_ready ? loader_.image()->getHeight() : 50;
	dim.des = 0;

	if (image_ready) {
		dim.wid = loader_.image()->getWidth() +
			2 * Inset::TEXT_TO_INSET_OFFSET;
	} else {
		int font_width = 0;

		LyXFont msgFont(mi.base.font);
		msgFont.setFamily(LyXFont::SANS_FAMILY);

		string const justname = OnlyFilename(params_.filename);
		if (!justname.empty()) {
			msgFont.setSize(LyXFont::SIZE_FOOTNOTE);
			font_width = font_metrics::width(justname, msgFont);
		}

		string const msg = statusMessage();
		if (!msg.empty()) {
			msgFont.setSize(LyXFont::SIZE_TINY);
			font_width = std::max(font_width,
					      font_metrics::width(msg, msgFont));
		}

		dim.wid = std::max(50, font_width + 15);
	}

	dim_ = dim;
}


void GraphicRenderer::draw(PainterInfo & pi, int x, int y) const
{
	Assert(pi.base.bv);
	view_ = pi.base.bv->owner()->view();

#if 0
	// Comment this out and see if anything goes wrong.
	// The explanation for why it _was_ needed once upon a time is below.

	// MakeAbsPath returns filename_ unchanged if it is absolute
	// already.
	string const file_with_path =
		MakeAbsPath(params_.filename, view_->buffer()->filePath());

	// A 'paste' operation creates a new inset with the correct filepath,
	// but then the 'old' inset stored in the 'copy' operation is actually
	// added to the buffer.

	// Thus, pasting a graphic into a new buffer with different
	// buffer->filePath() will result in the image being displayed in LyX even
	// though the relative path now points at nothing at all. Subsequent
	// loading of the file into LyX will therefore fail.

	// We should ensure that the filepath is correct.
	if (file_with_path != loader_.filename()) {
		params_.filename = file_with_path;
		update(params_);
	}
#endif

	if (params_.display != grfx::NoDisplay &&
	    loader_.status() == grfx::WaitingToLoad)
		loader_.startLoading();

	if (params_.display != grfx::NoDisplay && !loader_.monitoring())
		loader_.startMonitoring();

	// This will draw the graphics. If the graphics has not been loaded yet,
	// we draw just a rectangle.

	if (readyToDisplay()) {
		pi.pain.image(x + Inset::TEXT_TO_INSET_OFFSET,
			      y - dim_.asc,
			      dim_.wid - 2 * Inset::TEXT_TO_INSET_OFFSET,
			      dim_.asc + dim_.des,
			      *loader_.image());

	} else {
		pi.pain.rectangle(x + Inset::TEXT_TO_INSET_OFFSET,
				  y - dim_.asc,
				  dim_.wid - 2 * Inset::TEXT_TO_INSET_OFFSET,
				  dim_.asc + dim_.des);

		// Print the file name.
		LyXFont msgFont = pi.base.font;
		msgFont.setFamily(LyXFont::SANS_FAMILY);
		string const justname = OnlyFilename(params_.filename);

		if (!justname.empty()) {
			msgFont.setSize(LyXFont::SIZE_FOOTNOTE);
			pi.pain.text(x + Inset::TEXT_TO_INSET_OFFSET + 6,
				   y - font_metrics::maxAscent(msgFont) - 4,
				   justname, msgFont);
		}

		// Print the message.
		string const msg = statusMessage();
		if (!msg.empty()) {
			msgFont.setSize(LyXFont::SIZE_TINY);
			pi.pain.text(x + Inset::TEXT_TO_INSET_OFFSET + 6,
				     y - 4, msg, msgFont);
		}
	}
}
