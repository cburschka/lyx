/**
 * \file screen.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 *
 * Splash screen code added by Angus Leeming
 */

#include <config.h>

#include "screen.h"
#include "font_metrics.h"
#include "lyx_gui.h"
#include "Painter.h"
#include "WorkArea.h"

#include "BufferView.h"
#include "buffer.h"
#include "bufferparams.h"
#include "debug.h"
#include "language.h"
#include "lyxfont.h"
#include "lyxrc.h"
#include "lyxrow.h"
#include "lyxtext.h"
#include "metricsinfo.h"
#include "rowpainter.h"
#include "version.h"

#include "insets/updatableinset.h"

#include "graphics/GraphicsImage.h"
#include "graphics/GraphicsLoader.h"

#include "support/filetools.h" // LibFileSearch

#include <boost/utility.hpp>
#include <boost/bind.hpp>
#include <boost/signals/trackable.hpp>

using lyx::support::LibFileSearch;

using std::min;
using std::max;


namespace {

class SplashScreen : boost::noncopyable, boost::signals::trackable {
public:
	/// This is a singleton class. Get the instance.
	static SplashScreen const & get();
	///
	lyx::graphics::Image const * image() const { return loader_.image(); }
	///
	string const & text() const { return text_; }
	///
	LyXFont const & font() const { return font_; }
	///
	void connect(lyx::graphics::Loader::slot_type const & slot) const {
		loader_.connect(slot);
	}
	///
	void startLoading() const {
		if (loader_.status() == lyx::graphics::WaitingToLoad)
			loader_.startLoading();
	}

private:
	/** Make the c-tor private so we can control how many objects
	 *  are instantiated.
	 */
	SplashScreen();

	///
	lyx::graphics::Loader loader_;
	/// The text to be written on top of the pixmap
	string const text_;
	/// in this font...
	LyXFont font_;
};


SplashScreen const & SplashScreen::get()
{
	static SplashScreen singleton;
	return singleton;
}


SplashScreen::SplashScreen()
	: text_(lyx_version ? lyx_version : "unknown")
{
	if (!lyxrc.show_banner)
		return;

	string const file = LibFileSearch("images", "banner", "ppm");
	if (file.empty())
		return;

	// The font used to display the version info
	font_.setFamily(LyXFont::SANS_FAMILY);
	font_.setSeries(LyXFont::BOLD_SERIES);
	font_.setSize(LyXFont::SIZE_NORMAL);
	font_.setColor(LColor::yellow);

	// Load up the graphics file
	loader_.reset(file);
}

} // namespace anon


LyXScreen::LyXScreen()
	: cursor_visible_(false), greyed_out_(true)
{
	// Start loading the pixmap as soon as possible
	if (lyxrc.show_banner) {
		SplashScreen const & splash = SplashScreen::get();
		splash.connect(boost::bind(&LyXScreen::greyOut, this));
		splash.startLoading();
	}
}


LyXScreen::~LyXScreen()
{
}


void LyXScreen::showCursor(BufferView & bv)
{
	// this is needed to make sure we copy back the right
	// pixmap on the hide for the Qt frontend
	lyx_gui::sync_events();

	if (cursor_visible_)
		return;

	if (!bv.available())
		return;

	Cursor_Shape shape = BAR_SHAPE;

	LyXText const & text = *bv.getLyXText();
	LyXFont const & realfont = text.real_current_font;
	BufferParams const & bp = bv.buffer()->params();
	bool const samelang = realfont.language() == bp.language;
	bool const isrtl = realfont.isVisibleRightToLeft();

	if (!samelang || isrtl != bp.language->RightToLeft()) {
		shape = L_SHAPE;
		if (isrtl)
			shape = REVERSED_L_SHAPE;
	}

	// The ERT language hack needs fixing up
	if (realfont.language() == latex_language)
		shape = BAR_SHAPE;

	int ascent = font_metrics::maxAscent(realfont);
	int descent = font_metrics::maxDescent(realfont);
	int h = ascent + descent;
	int x = 0;
	int y = 0;
	int const top_y = bv.top_y();

	if (bv.theLockingInset()) {
		// Would be nice to clean this up to make some understandable sense...
		UpdatableInset * inset = bv.theLockingInset();
		inset->getCursor(bv, x, y);

		// Non-obvious. The reason we have to have these
		// extra checks is that the ->getCursor() calls rely
		// on the inset's own knowledge of its screen position.
		// If we scroll up or down in a big enough increment, the
		// inset->draw() is not called: this doesn't update
		// inset.top_baseline, so getCursor() returns an old value.
		// Ugly as you like.
		int bx, by;
		inset->getCursorPos(&bv, bx, by);
		by += inset->insetInInsetY() + bv.text->cursor.y();
		if (by < top_y)
			return;
		if (by > top_y + workarea().workHeight())
			return;
	} else {
		x = bv.text->cursor.x();
		y = bv.text->cursor.y();
		y -= top_y;
	}

	y -= ascent;

	// if it doesn't fit entirely on the screen, don't try to show it
	if (y < 0 || y + h > workarea().workHeight())
		return;

	cursor_visible_ = true;
	showCursor(x, y, h, shape);
}


void LyXScreen::hideCursor()
{
	if (!cursor_visible_)
		return;

	cursor_visible_ = false;
	removeCursor();
}


void LyXScreen::toggleCursor(BufferView & bv)
{
	if (cursor_visible_)
		hideCursor();
	else
		showCursor(bv);
}


bool LyXScreen::fitManualCursor(BufferView * bv, LyXText *,
	int /*x*/, int y, int asc, int desc)
{
	int const vheight = workarea().workHeight();
	int const topy = bv->top_y();
	int newtop = topy;


	if (y + desc - topy >= vheight)
		newtop = y - 3 * vheight / 4;  // the scroll region must be so big!!
	else if (y - asc < topy && topy > 0)
		newtop = y - vheight / 4;

	newtop = max(newtop, 0); // can newtop ever be < 0? (Lgb)

	if (newtop == topy)
		return false;

	bv->top_y(newtop);
	return true;
}


unsigned int LyXScreen::topCursorVisible(LyXText * text)
{
	LyXCursor const & cursor = text->cursor;
	int top_y = text->bv()->top_y();
	int newtop = top_y;
	unsigned int const vheight = workarea().workHeight();

	RowList::iterator row = text->cursorRow();

	if (int(cursor.y() - row->baseline() + row->height() - top_y) >= vheight) {
		if (row->height() < vheight
		    && row->height() > vheight / 4) {
			newtop = cursor.y()
				+ row->height()
				- row->baseline() - vheight;
		} else {
			// scroll down, the scroll region must be so big!!
			newtop = cursor.y() - vheight / 2;
		}

	} else if (int(cursor.y() - row->baseline()) < top_y && top_y > 0) {
		if (row->height() < vheight && row->height() > vheight / 4) {
			newtop = cursor.y() - row->baseline();
		} else {
			// scroll up
			newtop = cursor.y() - vheight / 2;
			newtop = min(newtop, top_y);
		}
	}

	return max(newtop, 0);
}


bool LyXScreen::fitCursor(LyXText * text, BufferView * bv)
{
	// Is a change necessary?
	int const newtop = topCursorVisible(text);
	bool const result = (newtop != bv->top_y());
	bv->top_y(newtop);
	return result;
}


void LyXScreen::redraw(BufferView & bv)
{
	greyed_out_ = !bv.text;

	if (greyed_out_) {
		greyOut();
		return;
	}

	workarea().getPainter().start();

	hideCursor();

	int const y = paintText(bv);

	// maybe we have to clear the screen at the bottom
	int const y2 = workarea().workHeight();
	if (y < y2 && !bv.text->isInInset()) {
		workarea().getPainter().fillRectangle(0, y,
			workarea().workWidth(), y2 - y,
			LColor::bottomarea);
	}

	expose(0, 0, workarea().workWidth(), workarea().workHeight());

	workarea().getPainter().end();
}


void LyXScreen::greyOut()
{
	if (!greyed_out_)
		return;

	workarea().getPainter().start();

	workarea().getPainter().fillRectangle(0, 0,
		workarea().workWidth(),
		workarea().workHeight(),
		LColor::bottomarea);

	// Add a splash screen to the centre of the work area
	SplashScreen const & splash = SplashScreen::get();
	lyx::graphics::Image const * const splash_image = splash.image();
	if (splash_image) {
		int const w = splash_image->getWidth();
		int const h = splash_image->getHeight();

		int x = (workarea().workWidth() - w) / 2;
		int y = (workarea().workHeight() - h) / 2;

		workarea().getPainter().image(x, y, w, h, *splash_image);

		x += 260;
		y += 265;

		workarea().getPainter().text(x, y, splash.text(), splash.font());
	}
	expose(0, 0, workarea().workWidth(), workarea().workHeight());
	workarea().getPainter().end();
}
