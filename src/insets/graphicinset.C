/**
 * \file graphicinset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "insets/graphicinset.h"

#include "buffer.h"
#include "BufferView.h"
#include "gettext.h"
#include "metricsinfo.h"

#include "frontends/font_metrics.h"
#include "frontends/LyXView.h"
#include "frontends/Painter.h"

#include "graphics/GraphicsImage.h"

#include "support/filetools.h"


GraphicInset::GraphicInset()
	: checksum_(0)
{}


GraphicInset::GraphicInset(GraphicInset const & other)
	: loader_(other.loader_),
	  params_(other.params_),
	  nodisplay_message_(other.nodisplay_message_),
	  checksum_(0)
{}


void GraphicInset::update(grfx::Params const & params)
{
	params_ = params;

	if (!params_.filename.empty()) {
		lyx::Assert(AbsolutePath(params_.filename));
		loader_.reset(params_.filename, params_);
	}
}


bool GraphicInset::hasFileChanged() const
{
	unsigned long const new_checksum = loader_.checksum();
	bool const file_has_changed = checksum_ != new_checksum;
	if (file_has_changed)
		checksum_ = new_checksum;
	return file_has_changed;
}


BufferView * GraphicInset::view() const
{
	return view_.lock().get();
}


boost::signals::connection GraphicInset::connect(slot_type const & slot) const
{
	return loader_.connect(slot);
}


void GraphicInset::setNoDisplayMessage(string const & str)
{
	nodisplay_message_ = str;
}


string const GraphicInset::statusMessage() const
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


GraphicInset::DisplayType GraphicInset::displayType() const
{
	if (params_.display == grfx::NoDisplay && !nodisplay_message_.empty())
		return NODISPLAY_MESSAGE;

	if (!loader_.image() || loader_.status() != grfx::Ready)
		return STATUS_MESSAGE;

	return loader_.image()->isDrawable() ? IMAGE : STATUS_MESSAGE;
}


void GraphicInset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	DisplayType type = displayType();

	dim.asc = (type == IMAGE) ? loader_.image()->getHeight() : 50;
	dim.des = 0;

	switch (type) {
	case IMAGE:
		dim.wid = loader_.image()->getWidth() +
			2 * Inset::TEXT_TO_INSET_OFFSET;
		break;

	case STATUS_MESSAGE:
	{
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
		break;
	}

	case NODISPLAY_MESSAGE:
	{
		int font_width = 0;

		LyXFont msgFont(mi.base.font);
		msgFont.setFamily(LyXFont::SANS_FAMILY);
		msgFont.setSize(LyXFont::SIZE_FOOTNOTE);
		font_width = font_metrics::width(nodisplay_message_, msgFont);

		dim.wid = std::max(50, font_width + 15);
		break;
	}
	}

	dim_ = dim;
}


void GraphicInset::draw(PainterInfo & pi, int x, int y) const
{
	if (pi.base.bv)
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

	switch (displayType()) {
	case IMAGE: 
	{
		pi.pain.image(x + Inset::TEXT_TO_INSET_OFFSET,
			      y - dim_.asc,
			      dim_.wid - 2 * Inset::TEXT_TO_INSET_OFFSET,
			      dim_.asc + dim_.des,
			      *loader_.image());
		break;
	}

	case STATUS_MESSAGE:
	{
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
		break;
	}

	case NODISPLAY_MESSAGE:
	{
		pi.pain.rectangle(x + Inset::TEXT_TO_INSET_OFFSET,
				  y - dim_.asc,
				  dim_.wid - 2 * Inset::TEXT_TO_INSET_OFFSET,
				  dim_.asc + dim_.des);

		LyXFont msgFont = pi.base.font;
		msgFont.setFamily(LyXFont::SANS_FAMILY);
		msgFont.setSize(LyXFont::SIZE_FOOTNOTE);
		pi.pain.text(x + Inset::TEXT_TO_INSET_OFFSET + 6,
			     y - font_metrics::maxAscent(msgFont) - 4,
			     nodisplay_message_, msgFont);
		break;
	}
	}
}
