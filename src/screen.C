/* This file is part of
* ====================================================== 
* 
*           LyX, The Document Processor
* 	 
*	    Copyright 1995 Matthias Ettrich
*           Copyright 1995-1998 The LyX Team
*
* ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation "lyxscreen.h"
#endif

#include <algorithm>

#include "lyxscreen.h"
#include "lyxtext.h"
#include "lyxrow.h"
#include "Painter.h"
#include "WorkArea.h"
#include "bufferparams.h"
#include "font.h"

using std::max;
using std::min;

static
GC createGC()
{
	XGCValues val;
	val.foreground = BlackPixel(fl_display, 
				    DefaultScreen(fl_display));
	
	val.function=GXcopy;
	val.graphics_exposures = false;
	val.line_style = LineSolid;
	val.line_width = 0;
	return XCreateGC(fl_display, RootWindow(fl_display, 0), 
			 GCForeground | GCFunction | GCGraphicsExposures
			 | GCLineWidth | GCLineStyle , &val);
}


// Constructor
LyXScreen::LyXScreen(WorkArea & o, LyXText * text_ptr)
	: owner(o), text(text_ptr)
{
	first = 0;
   
	/* the cursor isnt yet visible */ 
	cursor_visible = false;
	screen_refresh_y = -1;

	cursor_pixmap = 0;
	cursor_pixmap_x = 0;
	cursor_pixmap_y = 0;
	cursor_pixmap_w = 0;
	cursor_pixmap_h = 0;

	// We need this GC
	gc_copy = createGC();
}


void LyXScreen::Redraw()
{
	DrawFromTo(0, owner.height());
	screen_refresh_y = -1;
	expose(0, 0, owner.workWidth(), owner.height());
	if (cursor_visible) {
		cursor_visible = false;
		ShowCursor();
	}
}


void LyXScreen::expose(int x, int y, int exp_width, int exp_height)
{
	XCopyArea(fl_display,
		  owner.getPixmap(),
		  owner.getWin(),
		  gc_copy,
		  x, y,
		  exp_width, exp_height,
		  x + owner.xpos(),
		  y + owner.ypos());
}


void LyXScreen::DrawFromTo(int y1, int y2)
{
	long y_text = first + y1;
   
	/* get the first needed row */ 
	Row * row = text->GetRowNearY(y_text);
	/* y_text is now the real beginning of the row */
   
	long y = y_text - first;
	/* y1 is now the real beginning of row on the screen */
	
	while (row != 0 && y < y2) {
		text->GetVisibleRow(y, row, y + first);
		y += row->height;
		row = row->next;
	}
   
	/* maybe we have to clear the screen at the bottom */ 
	if (y < y2) {
		owner.getPainter().fillRectangle(0, y,
						 owner.workWidth(),
						 y2 - y,
					       LColor::bottomarea);
	}
}


void LyXScreen::DrawOneRow(Row * row, long & y_text)
{
	long y = y_text - first;
      
	if (y + row->height > 0 && y - row->height <= owner.height()) {
		/* ok there is something visible */
		text->GetVisibleRow(y, row, y + first);
	}
	y_text += row->height;
}


/* draws the screen, starting with textposition y. uses as much already
* printed pixels as possible */
void LyXScreen::Draw(long  y)
{
	if (cursor_visible) HideCursor();

	if (y < 0) y = 0;
	long old_first = first;
	first = y;

	/* is any optimiziation possible? */ 
	if ((y - old_first) < owner.height()
	    && (old_first - y) < owner.height()) {
		if (first < old_first) {
			DrawFromTo(0, old_first - first);
			XCopyArea (fl_display,
				   owner.getWin(),
				   owner.getWin(),
				   gc_copy,
				   owner.xpos(),
				   owner.ypos(),
				   owner.workWidth(),
				   owner.height() - old_first + first,
				   owner.xpos(),
				   owner.ypos() + old_first - first
				);
			// expose the area drawn
			expose(0, 0,
			       owner.workWidth(),
			       old_first - first);
		} else  {
			DrawFromTo(
				owner.height() + old_first - first,
				   owner.height());
			XCopyArea (fl_display,
				   owner.getWin(),
				   owner.getWin(),
				   gc_copy,
				   owner.xpos(),
				   owner.ypos() + first - old_first,
				   owner.workWidth(),
				   owner.height() + old_first - first,
				   owner.xpos(),
				   owner.ypos());
			// expose the area drawn
			expose(0, owner.height() + old_first - first,
			       owner.workWidth(), first - old_first);
		}
	} else {
		/* make a dumb new-draw */ 
		DrawFromTo(0, owner.height());
		expose(0, 0, owner.workWidth(), owner.height());
	}
}


void LyXScreen::ShowCursor()
{
	if (!cursor_visible) {
		Cursor_Shape shape = BAR_SHAPE;
		if (text->real_current_font.language() !=
		    text->parameters->language_info
		    || text->real_current_font.isVisibleRightToLeft()
		    != text->parameters->language_info->RightToLeft)
			shape = (text->real_current_font.isVisibleRightToLeft())
				? REVERSED_L_SHAPE : L_SHAPE;
		ShowManualCursor(text->cursor.x, text->cursor.y,
				 lyxfont::maxAscent(text->real_current_font),
				 lyxfont::maxDescent(text->real_current_font),
				 shape);
	}
}


/* returns 1 if first has changed, otherwise 0 */ 
int LyXScreen::FitManualCursor(long /*x*/, long y, int asc, int desc)
{
	long  newtop = first;
  
	if (y + desc  - first >= owner.height())
		newtop = y - 3 * owner.height() / 4;   /* the scroll region must be so big!! */
	else if (y - asc < first 
		 && first > 0) {
		newtop = y - owner.height() / 4;
	}
	if (newtop < 0)
		newtop = 0;
  
	if (newtop != first){
		Draw(newtop);
		first = newtop;
		return 1;
	}
	return 0;
}


void  LyXScreen::ShowManualCursor(long x, long y, int asc, int desc,
				  Cursor_Shape shape)
{
	long y1 = max(y - first - asc, 0L);
	long y2 = min(y - first + desc, long(owner.height()));

	// Secure against very strange situations
	if (y2 < y1) y2 = y1;

	if (cursor_pixmap){
		XFreePixmap(fl_display, cursor_pixmap);
		cursor_pixmap = 0;
	}

	if (y2 > 0 && y1 <
	    owner.height()) {
		cursor_pixmap_h = y2 - y1 + 1;
		cursor_pixmap_y = y1;

		switch(shape) {
		case BAR_SHAPE:
			cursor_pixmap_w = 1;
			cursor_pixmap_x = x;
			break;
		case L_SHAPE:
			cursor_pixmap_w = cursor_pixmap_h/3;
			cursor_pixmap_x = x;
			break;
		case REVERSED_L_SHAPE:
			cursor_pixmap_w = cursor_pixmap_h/3;
			cursor_pixmap_x = x - cursor_pixmap_w + 1;
			break;
		}

		cursor_pixmap = 
			XCreatePixmap (fl_display,
				       fl_root,
				       cursor_pixmap_w,
				       cursor_pixmap_h,
				       fl_get_visual_depth());
		XCopyArea (fl_display,
			   owner.getWin(),
			   cursor_pixmap,
			   gc_copy,
			   owner.xpos() + cursor_pixmap_x,
			   owner.ypos() + cursor_pixmap_y,
			   cursor_pixmap_w,
			   cursor_pixmap_h,
			   0, 0);
		XDrawLine(fl_display,
			  owner.getWin(),
			  gc_copy,
			  x + owner.xpos(),
			  y1 + owner.ypos(),
			  x + owner.xpos(),
			  y2 + owner.ypos());
		switch(shape) {
		case BAR_SHAPE:
			break;
		case L_SHAPE:
		case REVERSED_L_SHAPE:
			int rectangle_h = (cursor_pixmap_h+10)/20;
			XFillRectangle(fl_display,
				       owner.getWin(),
				       gc_copy,
				       cursor_pixmap_x + owner.xpos(),
				       y2 - rectangle_h + 1 + owner.ypos(),
				       cursor_pixmap_w - 1, rectangle_h);
			break;
		}

	}
	cursor_visible = true;
}


void LyXScreen::HideCursor()
{
	if (!cursor_visible) return;

	if (cursor_pixmap){
		XCopyArea (fl_display, 
			   cursor_pixmap,
			   owner.getWin(),
			   gc_copy,
			   0, 0, 
			   cursor_pixmap_w, cursor_pixmap_h,
			   cursor_pixmap_x + owner.xpos(),
			   cursor_pixmap_y + owner.ypos());
	}
	cursor_visible = false;
}


void LyXScreen::CursorToggle()
{
	if (cursor_visible)
		HideCursor();
	else
		ShowCursor();
}


/* returns a new top so that the cursor is visible */ 
long LyXScreen::TopCursorVisible()
{
	long newtop = first;

	if (text->cursor.y
	    - text->cursor.row->baseline
	    + text->cursor.row->height
	    - first >= owner.height()) {
		if (text->cursor.row->height < owner.height()
		    && text->cursor.row->height > owner.height() / 4)
			newtop = text->cursor.y
				+ text->cursor.row->height
				- text->cursor.row->baseline - owner.height();
		else
			newtop = text->cursor.y
				- 3 * owner.height() / 4;   /* the scroll region must be so big!! */
	} else if (text->cursor.y - text->cursor.row->baseline < first 
		   && first > 0) {
		if (text->cursor.row->height < owner.height()
		    && text->cursor.row->height > owner.height() / 4)
			newtop = text->cursor.y - text->cursor.row->baseline;
		else {
			newtop = text->cursor.y - owner.height() / 4;
			if (newtop > first)
				newtop = first;
		}
	}
	if (newtop < 0)
		newtop = 0;
	
	return newtop;
}


/* scrolls the screen so that the cursor is visible, if necessary.
* returns 1 if a change was made, otherwise 0 */ 
int LyXScreen::FitCursor()
{
	/* is a change necessary */ 
	long  newtop = TopCursorVisible();
	int result = (newtop != first);
	if (result)
		Draw(newtop);
	return result;
}

   
void LyXScreen::Update()
{
	if (text->status == LyXText::NEED_MORE_REFRESH
	    || screen_refresh_y > -1 ) {
		long y = 0;
		if (screen_refresh_y > -1
		    && screen_refresh_y < text->refresh_y)
			y = screen_refresh_y;
		else
			y = text->refresh_y;
		
		if (y < first) y = first;
		
		DrawFromTo(y - first, owner.height());
		text->refresh_y = 0;
		text->status = LyXText::UNCHANGED;
		screen_refresh_y = -1;
		expose(0, y - first,
		       owner.workWidth(), owner.height() - (y - first));
	} else if (text->status == LyXText::NEED_VERY_LITTLE_REFRESH) {
		/* ok I will update the current cursor row */
		long y = text->refresh_y;
		DrawOneRow(text->refresh_row, y);
		text->status = LyXText::UNCHANGED;
		expose(0, text->refresh_y - first,
		       owner.workWidth(), text->refresh_row->height);
	}
}


void LyXScreen::SmallUpdate()
{
	if (text->status == LyXText::NEED_MORE_REFRESH) {
		/* ok I will update till the current cursor row */
		Row * row = text->refresh_row;
		long y = text->refresh_y;
		long y2 = y;
      
		if (y > text->cursor.y) {
			Update();
			return;
		}
	 
		while (row
		       && row != text->cursor.row
		       && y < first + owner.height()) {
			DrawOneRow(row, y);
			row = row->next;
		}
      
		DrawOneRow(row, y);
		screen_refresh_y = y;
		screen_refresh_row = row->next;
		text->status = LyXText::UNCHANGED;
		// Is the right regin exposed?
		expose(0, y2 - first,
		       owner.workWidth(), y - y2);
	} else if (text->status == LyXText::NEED_VERY_LITTLE_REFRESH) {
		/* ok I will update the current cursor row */
		long y = text->refresh_y;
		DrawOneRow(text->refresh_row, y);
		text->status = LyXText::UNCHANGED;
		expose(0, text->refresh_y - first,
		       owner.workWidth(), text->refresh_row->height);
	}
}


void LyXScreen::ToggleSelection(bool kill_selection)
{
	/* only if there is a selection */ 
	if (!text->selection) return;

	long top = text->sel_start_cursor.y
		- text->sel_start_cursor.row->baseline;
	long bottom = text->sel_end_cursor.y
		- text->sel_end_cursor.row->baseline 
		+ text->sel_end_cursor.row->height;

	if (top < first)
		top = max(top, first);
        if (bottom < first)
		bottom = max(bottom, first);
	
	if (bottom > first + owner.height())
		bottom = first + owner.height();
	if (top > first + owner.height())
		top = first + owner.height();

	if (kill_selection)
		text->selection = 0;
	DrawFromTo(top - first, bottom - first);
	expose(0, top - first,
	       owner.workWidth(),
	       bottom - first - (top - first));
}
  
   
void LyXScreen::ToggleToggle()
{
	if (text->toggle_cursor.par == text->toggle_end_cursor.par
	    && text->toggle_cursor.pos == text->toggle_end_cursor.pos)
		return;
	
	long top = text->toggle_cursor.y
		- text->toggle_cursor.row->baseline;
	long bottom = text->toggle_end_cursor.y
		- text->toggle_end_cursor.row->baseline 
		+ text->toggle_end_cursor.row->height;
	
	if (top - first < 0)
		top = first;
	if (bottom - first < 0)
		bottom = first;
	
	if (bottom - first > owner.height())
		bottom = first + owner.height();
	if (top - first > owner.height())
		top = first + owner.height();
	
	DrawFromTo(top - first, bottom - first);
	expose(0, top - first, owner.workWidth(),
	       bottom - first - (top - first));
}
