/**
 * \file screen.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 *
 * Splash screen code added by Angus Leeming
 */


#include <config.h>

#include "screen.h"
#include "lyxtext.h"
#include "lyxrc.h"
#include "lyxrow.h"
#include "BufferView.h"
#include "buffer.h"
#include "WorkArea.h"
#include "Painter.h"
#include "font_metrics.h"
#include "language.h"
#include "debug.h"
#include "rowpainter.h"

// Splash screen-specific stuff
#include "lyxfont.h"
#include "version.h"

#include "graphics/GraphicsLoader.h"
#include "graphics/GraphicsImage.h"

#include "support/filetools.h" // LibFileSearch

#include <boost/utility.hpp>
#include <boost/bind.hpp>
#include <boost/signals/trackable.hpp>

using std::min;
using std::max;
using std::endl;

namespace {

class SplashScreen : boost::noncopyable, boost::signals::trackable {
public:
	/// This is a singleton class. Get the instance.
	static SplashScreen const & get();
	///
	grfx::Image const * image() const { return loader_.image(); }
	///
	string const & text() const { return text_; }
	///
	LyXFont const & font() const { return font_; }
	///
	void connect(grfx::Loader::slot_type const & slot) const {
		loader_.connect(slot);
	}
	///
	void startLoading() const {
		if (loader_.status() == grfx::WaitingToLoad)
			loader_.startLoading();
	}

private:
	/** Make the c-tor private so we can control how many objects
	 *  are instantiated.
	 */
	SplashScreen();

	///
	grfx::Loader loader_;
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

// FIXME: GUII these cursor methods need to decide
// whether the workarea is focused or not

void LyXScreen::showCursor(LyXText const * text, BufferView const * bv)
{
	if (cursor_visible_)
		return;

	workarea().getPainter().start();

	Cursor_Shape shape = BAR_SHAPE;
	BufferParams const & bp(bv->buffer()->params);
	LyXFont const & realfont(text->real_current_font);

	if (realfont.language() != bp.language
		|| realfont.isVisibleRightToLeft()
		!= bp.language->RightToLeft()) {
		shape = (realfont.isVisibleRightToLeft())
			? REVERSED_L_SHAPE : L_SHAPE;
	}

	showManualCursor(text, text->cursor.x(), text->cursor.y(),
		font_metrics::maxAscent(realfont),
		font_metrics::maxDescent(realfont),
		shape);

	workarea().getPainter().end();
}


bool LyXScreen::fitManualCursor(BufferView * bv, LyXText * text,
	int /*x*/, int y, int asc, int desc)
{
	int const vheight = workarea().workHeight();
	int newtop = text->top_y();

	if (y + desc - text->top_y() >= vheight)
		newtop = y - 3 * vheight / 4;  // the scroll region must be so big!!
	else if (y - asc < text->top_y()
		&& text->top_y() > 0) {
		newtop = y - vheight / 4;
	}

	newtop = max(newtop, 0); // can newtop ever be < 0? (Lgb)

	if (newtop != text->top_y()) {
		draw(text, bv, newtop);
		text->top_y(newtop);
		return true;
	}

	return false;
}


void LyXScreen::cursorToggle(BufferView * bv) const
{
	if (cursor_visible_)
		bv->hideCursor();
	else
		bv->showCursor();
}


unsigned int LyXScreen::topCursorVisible(LyXCursor const & cursor, int top_y)
{
	int const vheight = workarea().workHeight();
	int newtop = top_y;

	Row * row = cursor.row();

	// Is this a hack? Yes, probably... (Lgb)
	if (!row)
		return max(newtop, 0);

	if (cursor.y() - row->baseline() + row->height() - top_y >= vheight) {
		if (row->height() < vheight
		    && row->height() > vheight / 4) {
			newtop = cursor.y()
				+ row->height()
				- row->baseline() - vheight;
		} else {
			// scroll down
			newtop = cursor.y()
				- vheight / 2;   /* the scroll region must be so big!! */
		}

	} else if (static_cast<int>((cursor.y()) - row->baseline()) <
		   top_y && top_y > 0) {
		if (row->height() < vheight
		    && row->height() > vheight / 4) {
			newtop = cursor.y() - row->baseline();
		} else {
			// scroll up
			newtop = cursor.y() - vheight / 2;
			newtop = min(newtop, top_y);
		}
	}

	newtop = max(newtop, 0);

	return newtop;
}


bool LyXScreen::fitCursor(LyXText * text, BufferView * bv)
{
	// Is a change necessary?
	int const newtop = topCursorVisible(text->cursor, text->top_y());
	bool const result = (newtop != text->top_y());
	if (result) {
		draw(text, bv, newtop);
	}

	return result;
}


void LyXScreen::update(BufferView & bv, int yo, int xo)
{
	int const vwidth = workarea().workWidth();
	int const vheight = workarea().workHeight();
	LyXText * text = bv.text;

	workarea().getPainter().start();

	switch (text->status()) {
	case LyXText::NEED_MORE_REFRESH:
	{
		int const y = max(int(text->refresh_y - text->top_y()), 0);
		drawFromTo(text, &bv, y, vheight, yo, xo);
		expose(0, y, vwidth, vheight - y);
	}
	break;
	case LyXText::NEED_VERY_LITTLE_REFRESH:
	{
		// ok I will update the current cursor row
		drawOneRow(text, &bv, text->refresh_row, text->refresh_y,
			   yo, xo);
		// this because if we had a major update the refresh_row could
		// have been set to 0!
		if (text->refresh_row) {
			expose(0, text->refresh_y - text->top_y() + yo,
				   vwidth, text->refresh_row->height());
		}
	}
	break;
	case LyXText::CHANGED_IN_DRAW: // just to remove the warning
	case LyXText::UNCHANGED:
		// Nothing needs done
		break;
	}

	workarea().getPainter().end();
}


void LyXScreen::toggleSelection(LyXText * text, BufferView * bv,
				bool kill_selection,
				int yo, int xo)
{
	// only if there is a selection
	if (!text->selection.set()) return;

	int const bottom = min(
		max(static_cast<int>(text->selection.end.y()
				     - text->selection.end.row()->baseline()
				     + text->selection.end.row()->height()),
		    text->top_y()),
		static_cast<int>(text->top_y() + workarea().workHeight()));
	int const top = min(
		max(static_cast<int>(text->selection.start.y() -
				     text->selection.start.row()->baseline()),
		    text->top_y()),
		static_cast<int>(text->top_y() + workarea().workHeight()));

	if (kill_selection)
		text->selection.set(false);

	workarea().getPainter().start();

	drawFromTo(text, bv, top - text->top_y(), bottom - text->top_y(),
		   yo, xo);
	expose(0, top - text->top_y(),
	       workarea().workWidth(),
	       bottom - text->top_y() - (top - text->top_y()));

	workarea().getPainter().end();
}


void LyXScreen::toggleToggle(LyXText * text, BufferView * bv,
			     int yo, int xo)
{
	if (text->toggle_cursor.par() == text->toggle_end_cursor.par()
	    && text->toggle_cursor.pos() == text->toggle_end_cursor.pos())
		return;

	int const top_tmp = text->toggle_cursor.y()
		- text->toggle_cursor.row()->baseline();
	int const bottom_tmp = text->toggle_end_cursor.y()
		- text->toggle_end_cursor.row()->baseline()
		+ text->toggle_end_cursor.row()->height();

	int const offset = yo < 0 ? yo : 0;
	int const bottom = min(max(bottom_tmp, text->top_y()),
		static_cast<int>(text->top_y() + workarea().workHeight())) - offset;
	int const top = min(max(top_tmp, text->top_y()),
		static_cast<int>(text->top_y() + workarea().workHeight())) - offset;

	workarea().getPainter().start();

	drawFromTo(text, bv, top - text->top_y(),
		   bottom - text->top_y(), yo,
		   xo);
	expose(0, top - text->top_y(), workarea().workWidth(),
	       bottom - text->top_y() - (top - text->top_y()));

	workarea().getPainter().end();
}


void LyXScreen::redraw(LyXText * text, BufferView * bv)
{
	greyed_out_ = !text;

	if (greyed_out_) {
		greyOut();
		return;
	}



	workarea().getPainter().start();

	drawFromTo(text, bv, 0, workarea().workHeight(), 0, 0, text == bv->text);
	expose(0, 0, workarea().workWidth(), workarea().workHeight());

	workarea().getPainter().end();

	if (cursor_visible_) {
		cursor_visible_ = false;
		bv->showCursor();
	}
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
	grfx::Image const * const splash_image = splash.image();
	if (splash_image) {
		int const w = splash_image->getWidth();
		int const h = splash_image->getHeight();

		int x = (workarea().workWidth() - w) / 2;
		int y = (workarea().workHeight() - h) / 2;

		workarea().getPainter().image(x, y, w, h, *splash_image);

		string const & splash_text  = splash.text();
		LyXFont const & splash_font = splash.font();

		x += 260;
		y += 265;

		workarea().getPainter().text(x, y, splash_text, splash_font);
	}
	expose(0, 0, workarea().workWidth(), workarea().workHeight());
	workarea().getPainter().end();
}


void LyXScreen::drawFromTo(LyXText * text, BufferView * bv,
	int y1, int y2, int yo, int xo,
	bool internal)
{
	lyxerr[Debug::GUI] << "screen: drawFromTo " << y1 << '-' << y2 << endl;

	int y_text = text->top_y() + y1;

	// get the first needed row
	Row * row = text->getRowNearY(y_text);
	// y_text is now the real beginning of the row

	int y = y_text - text->top_y();
	// y1 is now the real beginning of row on the screen


	while (row != 0 && y < y2) {
		LyXText::text_status st = text->status();
		// we need this here as the row pointer may be illegal
		// at a later time (Jug20020502)
		Row * prev = row->previous();
		RowPainter rp(*bv, *text, *row);

		if (rp.paint(y + yo, xo, y + text->top_y()))
			text->markChangeInDraw(row, prev);

		internal = internal && (st != LyXText::CHANGED_IN_DRAW);
		while (internal && text->status() == LyXText::CHANGED_IN_DRAW) {
			text->fullRebreak();
			text->setCursor(text->cursor.par(),
					text->cursor.pos());
			text->postPaint(0);
			Row * prev = row->previous();
			RowPainter rp(*bv, *text, *row);
			if (rp.paint(y + yo, xo, y + text->top_y()))
				text->markChangeInDraw(row, prev);
		}
		y += row->height();
		row = row->next();
	}

	// maybe we have to clear the screen at the bottom
	if ((y < y2) && !text->isInInset()) {
		workarea().getPainter().fillRectangle(0, y,
			workarea().workWidth(), y2 - y,
			LColor::bottomarea);
	}
}


void LyXScreen::drawOneRow(LyXText * text, BufferView * bv, Row * row,
	int y_text, int yo, int xo)
{
	int const y = y_text - text->top_y() + yo;

	if (((y + row->height()) > 0) &&
	    ((y - row->height()) <= static_cast<int>(workarea().workHeight()))) {
		Row * prev = row->previous();
		RowPainter rp(*bv, *text, *row);
		if (rp.paint(y, xo, y + text->top_y()))
			text->markChangeInDraw(row, prev);
	}
}
