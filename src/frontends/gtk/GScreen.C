/**
 * \file GScreen.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

// Too hard to make concept checks work with this file
#ifdef _GLIBCPP_CONCEPT_CHECKS
#undef _GLIBCPP_CONCEPT_CHECKS
#endif

#include "GScreen.h"

#include "GWorkArea.h"

#include "buffer.h"
#include "BufferView.h"
#include "debug.h"
#include "language.h"
#include "LColor.h"
#include "lyxtext.h"
#include "lyxrow.h"

#include "frontends/screen.h"
#include "frontends/font_metrics.h"
#include "frontends/Painter.h"
#include "frontends/WorkArea.h"

#include "insets/insettext.h"

#include <algorithm>

namespace lyx {
namespace frontend {

GScreen::GScreen(GWorkArea & o)
	: LyXScreen(), owner_(o)
{
	// the cursor isnt yet visible
	cursorX_ = 0;
	cursorY_ = 0;
	cursorW_ = 0;
	cursorH_ = 0;
}


GScreen::~GScreen()
{
}


WorkArea & GScreen::workarea() const
{
	return owner_;
}


void GScreen::setCursorColor(Glib::RefPtr<Gdk::GC> gc)
{
	Gdk::Color * clr = owner_.getColorHandler().
		getGdkColor(LColor::cursor);
	gc->set_foreground(*clr);
}


void GScreen::showCursor(int x, int y, int h, Cursor_Shape shape)
{
	// Update the cursor color.
	setCursorColor(owner_.getGC());

	cursorX_ = x;
	cursorY_ = y;
	cursorH_ = h;

	switch (shape) {
	case BAR_SHAPE:
		cursorW_ = 1;
		break;
	case L_SHAPE:
		cursorW_ = cursorH_ / 3;
		break;
	case REVERSED_L_SHAPE:
		cursorW_ = cursorH_ / 3;
		cursorX_ = x - cursorW_ + 1;
		break;
	}

	int fx, fy, fwidth, fheight, fdepth;
	owner_.getWindow()->get_geometry(fx, fy, fwidth, fheight, fdepth);
	cursorPixmap_ = Gdk::Pixmap::create(owner_.getWindow(),
					    cursorW_,
					    cursorH_,
					    fdepth);
	cursorPixmap_->draw_drawable(owner_.getGC(),
				     owner_.getWindow(),
				     owner_.xpos() + cursorX_,
				     owner_.ypos() + cursorY_,
				     0, 0,
				     cursorW_,
				     cursorH_);
	owner_.getWindow()->draw_line(owner_.getGC(),
				      x + owner_.xpos(),
				      y + owner_.ypos(),
				      x + owner_.xpos(),
				      y + h - 1 + owner_.ypos());
	switch (shape) {
	case BAR_SHAPE:
		break;
	case L_SHAPE:
	case REVERSED_L_SHAPE:
		owner_.getWindow()->draw_line(owner_.getGC(),
					      owner_.xpos() + cursorX_,
					      owner_.ypos() + y + h - 1,
					      owner_.xpos() + cursorX_ + cursorW_ - 1,
					      owner_.xpos() + y + h - 1);
		break;
	}
}


void GScreen::removeCursor()
{
	if (cursorPixmap_) {
		owner_.getWindow()->draw_drawable(owner_.getGC(),
						  cursorPixmap_,
						  0, 0,
						  cursorX_ + owner_.xpos(),
						  cursorY_ + owner_.ypos(),
						  cursorW_, cursorH_);
	}
}


void GScreen::expose(int x, int y, int w, int h)
{
	lyxerr[Debug::GUI] << "expose " << w << 'x' << h
		<< '+' << x << '+' << y << std::endl;
	owner_.getWindow()->draw_drawable(owner_.getGC(),
					  owner_.getPixmap(),
					  x, y,
					  x + owner_.xpos(),
					  y + owner_.ypos(),
					  w, h);
}

} // namespace frontend
} // namespace lyx
