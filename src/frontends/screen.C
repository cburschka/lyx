/**
 * \file screen.C
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>

#include "screen.h"
#include "lyxtext.h"
#include "lyxrow.h"
#include "BufferView.h"
#include "buffer.h"
#include "WorkArea.h"
#include "Painter.h"
#include "font_metrics.h"
#include "language.h"
#include "debug.h"

using std::min;
using std::max;
using std::endl;
 
LyXScreen::LyXScreen()
	: force_clear_(true), cursor_visible_(false)
{
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
	int newtop = text->first_y;
 
	if (y + desc - text->first_y >= vheight)
		newtop = y - 3 * vheight / 4;  // the scroll region must be so big!!
	else if (y - asc < text->first_y
		&& text->first_y > 0) {
		newtop = y - vheight / 4;
	}

	newtop = max(newtop, 0); // can newtop ever be < 0? (Lgb)
 
	if (newtop != text->first_y) {
		draw(text, bv, newtop);
		text->first_y = newtop;
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
	
	if (cursor.y() - row->baseline() + row->height()
	    - top_y >= vheight) {
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
	int const newtop = topCursorVisible(text->cursor, text->first_y);
	bool const result = (newtop != text->first_y);
	if (result)
		draw(text, bv, newtop);
	return result;
}

  
void LyXScreen::update(LyXText * text, BufferView * bv,
	int yo, int xo)
{
	int const vwidth = workarea().workWidth();
	int const vheight = workarea().workHeight();

	workarea().getPainter().start();
 
	switch (text->status()) {
	case LyXText::NEED_MORE_REFRESH:
	{
		int const y = max(int(text->refresh_y - text->first_y), 0);
		drawFromTo(text, bv, y, vheight, yo, xo);
		text->refresh_y = 0;
		// otherwise this is called ONLY from BufferView_pimpl(update)
		// or we should see to set this flag accordingly
		if (text != bv->text)
			text->status(bv, LyXText::UNCHANGED);
		expose(0, y, vwidth, vheight - y);
	}
	break;
	case LyXText::NEED_VERY_LITTLE_REFRESH:
	{
		// ok I will update the current cursor row
		drawOneRow(text, bv, text->refresh_row, text->refresh_y,
			   yo, xo);
		// this because if we had a major update the refresh_row could
		// have been set to 0!
		if (text->refresh_row) {
			// otherwise this is called ONLY from BufferView_pimpl(update)
			// or we should see to set this flag accordingly
			if (text != bv->text)
				text->status(bv, LyXText::UNCHANGED);
			expose(0, text->refresh_y - text->first_y + yo,
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
		    text->first_y),
		static_cast<int>(text->first_y + workarea().workHeight()));
	int const top = min(
		max(static_cast<int>(text->selection.start.y() -
				     text->selection.start.row()->baseline()),
		    text->first_y),
		static_cast<int>(text->first_y + workarea().workHeight()));

	if (kill_selection)
		text->selection.set(false);
 
	workarea().getPainter().start();
 
	drawFromTo(text, bv, top - text->first_y, bottom - text->first_y,
		   yo, xo);
	expose(0, top - text->first_y,
	       workarea().workWidth(),
 	       bottom - text->first_y - (top - text->first_y));
 
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
	int const bottom = min(max(bottom_tmp, text->first_y),
		static_cast<int>(text->first_y + workarea().workHeight())) - offset;
	int const top = min(max(top_tmp, text->first_y),
		static_cast<int>(text->first_y + workarea().workHeight())) - offset;

	workarea().getPainter().start();

	drawFromTo(text, bv, top - text->first_y,
		   bottom - text->first_y, yo,
		   xo);
	expose(0, top - text->first_y, workarea().workWidth(),
	       bottom - text->first_y - (top - text->first_y));
 
	workarea().getPainter().end();
}


void LyXScreen::redraw(LyXText * text, BufferView * bv)
{
	workarea().getPainter().start();

	if (!text) {
		greyOut();
		expose(0, 0, workarea().workWidth(), workarea().workHeight());
		workarea().getPainter().end();
		return;
	}

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
	workarea().getPainter().fillRectangle(0, 0,
		workarea().workWidth(),
		workarea().workHeight(),
		LColor::bottomarea);
}


void LyXScreen::drawFromTo(LyXText * text, BufferView * bv,
	int y1, int y2, int yo, int xo,
	bool internal)
{
	int y_text = text->first_y + y1;
  
	// get the first needed row
	Row * row = text->getRowNearY(y_text);
	// y_text is now the real beginning of the row
  
	int y = y_text - text->first_y;
	// y1 is now the real beginning of row on the screen
	
	while (row != 0 && y < y2) {
		LyXText::text_status st = text->status();
		text->getVisibleRow(bv, y + yo,
		                    xo, row, y + text->first_y);
		internal = internal && (st != LyXText::CHANGED_IN_DRAW);
		while (internal && text->status() == LyXText::CHANGED_IN_DRAW) {
			text->fullRebreak(bv);
			st = LyXText::NEED_MORE_REFRESH;
			text->setCursor(bv, text->cursor.par(), text->cursor.pos());
			text->status(bv, st);
			text->getVisibleRow(bv, y + yo,
			                    xo, row, y + text->first_y);
		}
		y += row->height();
		row = row->next();
	}
	force_clear_ = false;

	// maybe we have to clear the screen at the bottom
	if ((y < y2) && text->bv_owner) {
		workarea().getPainter().fillRectangle(0, y,
			workarea().workWidth(), y2 - y,
			LColor::bottomarea);
	}
}


void LyXScreen::drawOneRow(LyXText * text, BufferView * bv, Row * row,
	int y_text, int yo, int xo)
{
	int const y = y_text - text->first_y + yo;

	if (((y + row->height()) > 0) &&
	    ((y - row->height()) <= static_cast<int>(workarea().workHeight()))) {
		text->getVisibleRow(bv, y, xo, row, y + text->first_y);
	}
	force_clear_ = false;
}
