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

#include "lyxscreen.h"
#include "lyxdraw.h"
#include "lyxtext.h"
#include "lyxrow.h"
#include "BufferView.h"
#include "Painter.h"

extern int mono_video;
extern int fast_selection;

#ifdef USE_PAINTER
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
#endif


// Constructor
LyXScreen::LyXScreen(BufferView * o, Window window,
#ifdef NEW_WA
		     Pixmap p,
#endif
		     Dimension width, 
		     Dimension height,
		     Dimension offset_x,
		     Dimension offset_y,
		     LyXText *text_ptr)
	: owner(o), text(text_ptr), _window(window), 
	_width(width), _height(height),
	_offset_x(offset_x), _offset_y(offset_y)
{
	first = 0;
   
	/* the cursor isnt yet visible */ 
	cursor_visible = false;
	screen_refresh_y = -1;

	/* create the foreground pixmap */
#ifdef NEW_WA
	foreground = p;
#else
	foreground = XCreatePixmap (fl_display,
				    fl_root,
				    _width, _height, 
				    fl_get_visual_depth());
#endif
	cursor_pixmap = 0;
	cursor_pixmap_x = 0;
	cursor_pixmap_y = 0;
	cursor_pixmap_w = 0;
	cursor_pixmap_h = 0;

#ifdef USE_PAINTER
	// We need this GC
	gc_copy = createGC();
#endif
}


// Destructor
LyXScreen::~LyXScreen()
{
#ifndef NEW_WA
	XFreePixmap(fl_display, foreground);
#endif
}


void LyXScreen::Redraw()
{
	DrawFromTo(0, _height);
	screen_refresh_y = -1;
	expose(0, 0, _width, _height);
	if (cursor_visible) {
		cursor_visible = false;
		ShowCursor();
	}
}


#ifdef USE_PAINTER
void LyXScreen::expose(int x, int y, int exp_width, int exp_height)
{
	XCopyArea(fl_display,
		  foreground,
		  _window,
		  gc_copy,
		  x, y,
		  exp_width, exp_height,
		  x+_offset_x, y+_offset_y);
}
#else
void LyXScreen::expose(int x, int y, int exp_width, int exp_height)
{
	XCopyArea(fl_display,
		  foreground,
		  _window,
		  getGC(gc_copy),
		  x, y,
		  exp_width, exp_height,
		  x+_offset_x, y+_offset_y);
}
#endif


#ifdef USE_PAINTER
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
		row = row -> next;

	}
   
	/* maybe we have to clear the screen at the bottom */ 
	if (y < y2) {
		owner->painter().fillRectangle(0, y, _width, y2 - y,
					       LColor::bottomarea);
	}
}
#else
void LyXScreen::DrawFromTo(int y1, int y2)
{
	long y_text = first + y1;
   
	/* get the first needed row */ 
	Row * row = text->GetRowNearY(y_text);
	/* y_text is now the real beginning of the row */
   
	long y = y_text - first;
	/* y1 is now the real beginning of row on the screen */
	
	while (row != 0 && y < y2) {

		text->GetVisibleRow(*this, y, row, y + first);
		y += row->height;
		row = row -> next;

	}
   
	/* maybe we have to clear the screen at the bottom */ 
	if (y < y2) {
		fillRectangle(gc_lighted,
			      0,
			      y,
			      _width,
			      y2 - y);
	}
}
#endif


void LyXScreen::DrawOneRow(Row * row, long & y_text)
{
	long y = y_text - first;
      
	if (y + row->height > 0 && y - row->height <= _height) {
		/* ok there is something visible */
#ifdef USE_PAINTER
		text->GetVisibleRow(y, row, y + first);
#else
		text->GetVisibleRow(*this, y, row, y + first);
#endif
	}
	y_text+= row->height;
}


/* draws the screen, starting with textposition y. uses as much already
* printed pixels as possible */
#ifdef USE_PAINTER
void LyXScreen::Draw(long  y)
{
	if (cursor_visible) HideCursor();

	if (y < 0) y = 0;
	long old_first = first;
	first = y;

	/* is any optimiziation possible? */ 
	if ((y - old_first) < _height 
	    && (old_first - y) < _height) {
		if (first < old_first) {
			DrawFromTo(0, old_first - first);
			XCopyArea (fl_display,
				   _window,
				   _window,
				   gc_copy,
				   _offset_x, _offset_y, 
				   _width , _height - old_first + first,
				   _offset_x, _offset_y + old_first - first);
			// expose the area drawn
			expose(0, 0, _width, old_first - first);
		} else  {
			DrawFromTo(_height + old_first - first, _height);
			XCopyArea (fl_display,
				   _window,
				   _window,
				   gc_copy,
				   _offset_x, _offset_y + first - old_first, 
				   _width , _height + old_first - first, 
				   _offset_x, _offset_y);
			// expose the area drawn
			expose(0, _height + old_first - first, 
			       _width, first - old_first);
		}
	} else {
		/* make a dumb new-draw */ 
		DrawFromTo(0, _height);
		expose(0, 0, _width, _height);
	}
}
#else
void LyXScreen::Draw(long  y)
{
	if (cursor_visible) HideCursor();

	if (y < 0) y = 0;
	long old_first = first;
	first = y;

	/* is any optimiziation possible? */ 
	if ((y - old_first) < _height 
	    && (old_first - y) < _height) {
		if (first < old_first) {
			DrawFromTo(0, old_first - first);
			XCopyArea (fl_display,
				   _window,
				   _window,
				   getGC(gc_copy),
				   _offset_x, _offset_y, 
				   _width , _height - old_first + first,
				   _offset_x, _offset_y + old_first - first);
			// expose the area drawn
			expose(0, 0, _width, old_first - first);
		} else  {
			DrawFromTo(_height + old_first - first, _height);
			XCopyArea (fl_display,
				   _window,
				   _window,
				   getGC(gc_copy),
				   _offset_x, _offset_y + first - old_first, 
				   _width , _height + old_first - first, 
				   _offset_x, _offset_y);
			// expose the area drawn
			expose(0, _height + old_first - first, 
			       _width, first - old_first);
		}
	} else {
		/* make a dumb new-draw */ 
		DrawFromTo(0, _height);
		expose(0, 0, _width, _height);
	}
}
#endif


void LyXScreen::ShowCursor()
{
	long x = 0;
	long y1 = 0;
	long y2 = 0;
   
	if (cursor_visible) return;
   
	x = text->cursor.x;
	
	y1 = text->cursor.y - text->real_current_font.maxAscent() - first;
	if (y1 < 0) y1 = 0;
	
	y2 = text->cursor.y + text->real_current_font.maxDescent() - first;
	if (y2 > _height) y2 = _height;

	// Secure against very strange situations
	if (y2 < y1) y2 = y1;

#ifndef USE_PAINTER
	if (fast_selection || mono_video) {
 		if (y2 > 0 && y1 < _height) {
			XDrawLine(fl_display,
				  _window, getGC(gc_select),
				  x + _offset_x,
				  y1 + _offset_y,
				  x + _offset_x,
				  y2 + _offset_y);
			cursor_visible = true;
		}
	} else {
#endif
		if (cursor_pixmap){
			XFreePixmap(fl_display, cursor_pixmap);
			cursor_pixmap = 0;
		}
   
		if (y2 > 0 && y1 < _height) {
			cursor_pixmap_w = 1;
			cursor_pixmap_h = y2 - y1 + 1;
			cursor_pixmap_x = x;
			cursor_pixmap_y = y1;
			cursor_pixmap = 
				XCreatePixmap(fl_display,
					      fl_root,
					      cursor_pixmap_w,
					      cursor_pixmap_h,
					      fl_get_visual_depth());
#ifdef USE_PAINTER
			XCopyArea(fl_display,
				  _window,
				  cursor_pixmap,
				  gc_copy,
				  _offset_x + cursor_pixmap_x, 
				  _offset_y + cursor_pixmap_y,
				  cursor_pixmap_w, cursor_pixmap_h,
				  0, 0);
			XDrawLine(fl_display,
				  _window, gc_copy,
				  x + _offset_x,
				  y1 + _offset_y,
				  x + _offset_x,
				  y2 + _offset_y);
#else
			XCopyArea(fl_display,
				  _window,
				  cursor_pixmap,
				  getGC(gc_copy),
				  _offset_x + cursor_pixmap_x, 
				  _offset_y + cursor_pixmap_y,
				  cursor_pixmap_w, cursor_pixmap_h,
				  0, 0);
			XDrawLine(fl_display,
				  _window,
				  getGC(gc_copy),
				  x + _offset_x,
				  y1 + _offset_y,
				  x + _offset_x,
				  y2 + _offset_y);
#endif
			cursor_visible = true;
		}
#ifndef USE_PAINTER
	}
#endif
}


/* returns 1 if first has changed, otherwise 0 */ 
int LyXScreen::FitManualCursor(long /*x*/, long y, int asc, int desc)
{
	long  newtop = first;
  
	if (y + desc  - first >= _height)
		newtop = y - 3*_height / 4;   /* the scroll region must be so big!! */
	else if (y - asc < first 
		 && first > 0) {
		newtop = y - _height / 4;
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


void  LyXScreen::HideManualCursor(long x, long y, int asc, int desc)
{
	if (fast_selection || mono_video)
		ShowManualCursor(x, y, asc, desc);
	else
		HideCursor();
}


void  LyXScreen::ShowManualCursor(long x, long y, int asc, int desc)
{
	long y1 = 0;
	long y2 = 0;
	
	y1 = y - first - asc;
	if (y1 < 0)
		y1 = 0;
	y2 = y -first + desc;
	if (y2 > _height)
		y2 = _height;

#ifndef USE_PAINTER
	if (fast_selection || mono_video) {
		if (y2 > 0 && y1 < _height) {
			XDrawLine(fl_display,
				  _window, getGC(gc_select),
				  x+_offset_x,
				  y1+_offset_y,
				  x+_offset_x,
				  y2+_offset_y);
		}
	} else {
#endif
		if (cursor_pixmap){
			XFreePixmap(fl_display, cursor_pixmap);
			cursor_pixmap = 0;
		}
		
		if (y2 > 0 && y1 < _height) {
			cursor_pixmap_w = 1;
			cursor_pixmap_h = y2 - y1 + 1;
			cursor_pixmap_x = x,
				cursor_pixmap_y = y1;
			cursor_pixmap = 
				XCreatePixmap (fl_display,
					       fl_root,
					       cursor_pixmap_w,
					       cursor_pixmap_h,
					       fl_get_visual_depth());
#ifdef USE_PAINTER
			XCopyArea (fl_display,
				   _window,
				   cursor_pixmap,
				   gc_copy,
				   _offset_x + cursor_pixmap_x,
				   _offset_y + cursor_pixmap_y,
				   cursor_pixmap_w,
				   cursor_pixmap_h,
				   0, 0);
			XDrawLine(fl_display,
				  _window, gc_copy,
				  x+_offset_x,
				  y1+_offset_y,
				  x+_offset_x,
				  y2+_offset_y);
#else
			XCopyArea (fl_display,
				   _window,
				   cursor_pixmap,
				   getGC(gc_copy),
				   _offset_x + cursor_pixmap_x,
				   _offset_y + cursor_pixmap_y,
				   cursor_pixmap_w,
				   cursor_pixmap_h,
				   0, 0);
			XDrawLine(fl_display,
				  _window,
				  getGC(gc_copy),
				  x+_offset_x,
				  y1+_offset_y,
				  x+_offset_x,
				  y2+_offset_y);
#endif
		}
		cursor_visible = true;
#ifndef USE_PAINTER
	}
#endif
}


void LyXScreen::HideCursor()
{
	if (!cursor_visible) return;

#ifndef USE_PAINTER
	if (fast_selection || mono_video) {
		cursor_visible = false;
		ShowCursor();
		cursor_visible = false;
	} else {
#endif
		if (cursor_pixmap){
#ifdef USE_PAINTER
			XCopyArea (fl_display, 
				   cursor_pixmap,
				   _window,
				   gc_copy,
				   0, 0, 
				   cursor_pixmap_w, cursor_pixmap_h,
				   cursor_pixmap_x + _offset_x,
				   cursor_pixmap_y + _offset_y);
#else
			XCopyArea (fl_display, 
				   cursor_pixmap,
				   _window,
				   getGC(gc_copy),
				   0, 0, 
				   cursor_pixmap_w, cursor_pixmap_h,
				   cursor_pixmap_x + _offset_x,
				   cursor_pixmap_y + _offset_y);
#endif
		}
		cursor_visible = false;
#ifndef USE_PAINTER
	}
#endif
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
	    - first >= _height) {
		if (text->cursor.row->height < _height
		    && text->cursor.row->height > _height/4)
			newtop = text->cursor.y
				+ text->cursor.row->height
				- text->cursor.row->baseline - _height;
		else
			newtop = text->cursor.y
				- 3*_height / 4;   /* the scroll region must be so big!! */
	} else if (text->cursor.y - text->cursor.row->baseline < first 
		   && first > 0) {
		if (text->cursor.row->height < _height
		    && text->cursor.row->height > _height/4)
			newtop = text->cursor.y - text->cursor.row->baseline;
		else {
			newtop = text->cursor.y - _height / 4;
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
	long y = 0;

	if (text->status == LyXText::NEED_MORE_REFRESH
	    || screen_refresh_y > -1 ) {
		if (screen_refresh_y > -1
		    && screen_refresh_y < text->refresh_y)
			y = screen_refresh_y;
		else
			y = text->refresh_y;
		
		if (y < first) y = first;
		
		DrawFromTo(y - first, _height);
		text->refresh_y = 0;
		text->status = LyXText::UNCHANGED;
		screen_refresh_y = -1;
		expose(0, y-first, _width, _height - (y - first));
	} else if (text->status == LyXText::NEED_VERY_LITTLE_REFRESH) {
		/* ok I will update the current cursor row */
		y = text->refresh_y;
		DrawOneRow(text->refresh_row, y);
		text->status = LyXText::UNCHANGED;
		expose(0, text->refresh_y-first,
		       _width, text->refresh_row->height);
	}
}


void LyXScreen::SmallUpdate()
{
	Row * row = 0;
	long y = 0;
	long y2 = 0;
	
	if (text->status == LyXText::NEED_MORE_REFRESH){
		/* ok I will update till the current cursor row */
		row = text->refresh_row;
		y = text->refresh_y;
		y2 = y;
      
		if (y > text->cursor.y) {
			Update();
			return;
		}
	 
		while (row && row != text->cursor.row && y < first + _height) {
			DrawOneRow(row, y);
			row = row->next;
		}
      
		DrawOneRow(row, y);
		screen_refresh_y = y;
		screen_refresh_row = row->next;
		text->status = LyXText::UNCHANGED;
		// Is the right regin exposed?
		expose(0, y2-first, _width, y-y2);
	} else if (text->status == LyXText::NEED_VERY_LITTLE_REFRESH) {
		/* ok I will update the current cursor row */
		row = text->refresh_row;
		y = text->refresh_y;
		DrawOneRow(row, y);
		text->status = LyXText::UNCHANGED;
		expose(0, text->refresh_y - first,
		       _width, row->height);
	}
}


void LyXScreen::ToggleSelection(bool kill_selection)
{
	/* only if there is a selection */ 
	if (!text->selection)
		return;

	long top = 0;
	long bottom = 0;
   
#ifndef USE_PAINTER
	if (fast_selection || mono_video) {
		
		/* selection only in one row ?*/ 
		if (text->sel_start_cursor.y == text->sel_end_cursor.y) {
			
			/* only if something is visible */ 
			if (text->sel_start_cursor.y
			    - text->sel_start_cursor.row->baseline
			    - first < _height
			    && text->sel_start_cursor.y
			    - text->sel_start_cursor.row->baseline + 
			    text->sel_start_cursor.row->height - first > 0) {
				top = text->sel_start_cursor.y
					- text->sel_start_cursor.row->baseline
					- first;
				bottom = top
					+ text->sel_start_cursor.row->height;
				if (top<0)
					top = 0;
				if (bottom > _height)
					bottom = _height;
				XFillRectangle(fl_display, _window,
					       getGC(gc_select),
					       text->sel_start_cursor.x
					       +_offset_x, 
					       top+_offset_y,
					       text->sel_end_cursor.x
					       - text->sel_start_cursor.x,
					       bottom - top);
			}
		} else {
			/* the sel_start_cursor row first */ 
			/* only if anything is visible */ 
			if (text->sel_start_cursor.y
			    - text->sel_start_cursor.row->baseline
			    - first < _height
			    && text->sel_start_cursor.y
			    - text->sel_start_cursor.row->baseline + 
			    text->sel_start_cursor.row->height - first > 0) {
				top = text->sel_start_cursor.y
					- text->sel_start_cursor.row->baseline
					- first;
				bottom = top
					+ text->sel_start_cursor.row->height;
				if (top<0)
					top = 0;
				if (bottom > _height)
					bottom = _height;
				XFillRectangle(fl_display, _window,
					       getGC(gc_select),
					       text->sel_start_cursor.x
					       +_offset_x, 
					       top+_offset_y,
					       _width
					       - text->sel_start_cursor.x,
					       bottom - top);
			}
			
			/* the main body */ 
			
			if (text->sel_start_cursor.row->next != 
			    text->sel_end_cursor.row) {
				top = text->sel_start_cursor.y
					- text->sel_start_cursor.row->baseline
					+ text->sel_start_cursor.row->height;
				bottom = text->sel_end_cursor.y
					- text->sel_end_cursor.row->baseline;
				
				if (top - first < 0)
					top = first;
				if (bottom - first < 0)
					bottom = first;
				
				if (bottom - first > _height)
					bottom = first + _height;
				if (top - first > _height)
					top = first + _height;
				
				if (top != bottom) {
					XFillRectangle(fl_display,
						       _window,
						       getGC(gc_select),
						       0+_offset_x, 
						       top - first+_offset_y,
						       _width,
						       bottom - top);
				}
			}
			
			/* the sel_end_cursor row last */ 
			if (text->sel_end_cursor.y
			    - text->sel_end_cursor.row->baseline
			    - first < _height
			    && text->sel_end_cursor.y
			    - text->sel_end_cursor.row->baseline +
			    text->sel_end_cursor.row->height - first > 0) {
				top = text->sel_end_cursor.y
					- text->sel_end_cursor.row->baseline
					- first;
				bottom = top
					+ text->sel_end_cursor.row->height;
				if (top<0)
					top = 0;
				if (bottom > _height)
					bottom = _height;
				XFillRectangle(fl_display, _window,
					       getGC(gc_select),
					       0+_offset_x, 
					       top+_offset_y,
					       text->sel_end_cursor.x,
					       bottom - top);
			}
		}
	} else {
#endif
		top = text->sel_start_cursor.y
			- text->sel_start_cursor.row->baseline;
		bottom = text->sel_end_cursor.y
			- text->sel_end_cursor.row->baseline 
			+ text->sel_end_cursor.row->height;
		
		if (top - first < 0)
			top = first;
		if (bottom - first < 0)
			bottom = first;
		
		if (bottom - first > _height)
			bottom = first + _height;
		if (top - first > _height)
			top = first + _height;
		
		if (kill_selection)
			text->selection = 0;
		DrawFromTo(top - first, bottom - first);
		expose(0, top - first, _width, bottom - first - (top - first));
#ifndef USE_PAINTER
	}
#endif
}
  
   
void LyXScreen::ToggleToggle()
{
	long top = 0;
	long bottom = 0;
	
	if (text->toggle_cursor.par == text->toggle_end_cursor.par
	    && text->toggle_cursor.pos == text->toggle_end_cursor.pos)
		return;

#ifndef USE_PAINTER
	if (fast_selection || mono_video) {
		
		/* selection only in one row ?*/ 
		if (text->toggle_cursor.y == text->toggle_end_cursor.y) {
			
			/* only if anything is visible */ 
			if (text->toggle_cursor.y - text->toggle_cursor.row->baseline - first < _height
			    && text->toggle_cursor.y - text->toggle_cursor.row->baseline + 
			    text->toggle_cursor.row->height - first > 0) {
				top = text->toggle_cursor.y - text->toggle_cursor.row->baseline - first;
				bottom = top + text->toggle_cursor.row->height;
				if (top < 0) top = 0;
				if (bottom > _height) bottom = _height;
				XFillRectangle(fl_display, _window,
					       getGC(gc_select),
					       text->toggle_cursor.x+_offset_x, 
					       top+_offset_y,
					       text->toggle_end_cursor.x  -
					       text->toggle_cursor.x,
					       bottom - top);
			}
		} else {
			/* the toggle_cursor row first */ 
			/* only if anything is visible */ 
			if (text->toggle_cursor.y - text->toggle_cursor.row->baseline - first < _height
			    && text->toggle_cursor.y - text->toggle_cursor.row->baseline + 
			    text->toggle_cursor.row->height - first > 0) {
				top = text->toggle_cursor.y - text->toggle_cursor.row->baseline - first;
				bottom = top + text->toggle_cursor.row->height;
				if (top<0)
					top = 0;
				if (bottom > _height)
					bottom = _height;
				XFillRectangle(fl_display, _window,
					       getGC(gc_select),
					       text->toggle_cursor.x+_offset_x, 
					       top+_offset_y,
					       _width - text->toggle_cursor.x,
					       bottom - top);
			}
			
			/* the main body */ 
			
			if (text->toggle_cursor.row->next != 
			    text->toggle_end_cursor.row) {
				top = text->toggle_cursor.y
					- text->toggle_cursor.row->baseline
					+ text->toggle_cursor.row->height;
				bottom = text->toggle_end_cursor.y
					- text->toggle_end_cursor.row->baseline;
				
				if (top - first < 0)
					top = first;
				if (bottom - first < 0)
					bottom = first;
				
				if (bottom - first > _height)
					bottom = first + _height;
				if (top - first > _height)
					top = first + _height;
				
				if (top != bottom) {
					XFillRectangle(fl_display, _window,
						       getGC(gc_select),
						       0+_offset_x, 
						       top - first+_offset_y,
						       _width,
						       bottom - top);
				}
			}
			
			/* the toggle_end_cursor row last */ 
			if (text->toggle_end_cursor.y - text->toggle_end_cursor.row->baseline - first < _height
			    && text->toggle_end_cursor.y - text->toggle_end_cursor.row->baseline +
			    text->toggle_end_cursor.row->height - first > 0) {
				top = text->toggle_end_cursor.y
					- text->toggle_end_cursor.row->baseline
					- first;
				bottom = top
					+ text->toggle_end_cursor.row->height;
				if (top<0)
					top = 0;
				if (bottom > _height)
					bottom = _height;
				XFillRectangle(fl_display, _window,
					       getGC(gc_select),
					       0+_offset_x, 
					       top+_offset_y,
					       text->toggle_end_cursor.x,
					       bottom - top);
			}
		}
	} else {
#endif
		top = text->toggle_cursor.y
			- text->toggle_cursor.row->baseline;
		bottom = text->toggle_end_cursor.y
			- text->toggle_end_cursor.row->baseline 
			+ text->toggle_end_cursor.row->height;
		
		if (top - first < 0)
			top = first;
		if (bottom - first < 0)
			bottom = first;
		
		if (bottom - first > _height)
			bottom = first + _height;
		if (top - first > _height)
			top = first + _height;
		
		DrawFromTo(top - first, bottom - first);
		expose(0, top - first, _width, bottom - first - (top - first));
#ifndef USE_PAINTER
	}
#endif
}


#ifndef USE_PAINTER
void LyXScreen::drawTableLine(int baseline, int x, int length, bool on_off)
{
	GC gc;
	if (on_off)
		gc = getGC(gc_thin_on_off_line);
	else
		gc = getGC(gc_copy);
	drawLine(gc,
		 x,
		 baseline,
		 x + length,
		 baseline);
}
#endif


#ifndef USE_PAINTER
void LyXScreen::drawVerticalTableLine(int x, int y1, int y2, bool on_off)
{
	GC gc;
	if (on_off)
		gc = getGC(gc_thin_on_off_line);
	else
		gc = getGC(gc_copy);
	drawLine(gc,
		 x,
		 y1,
		 x,
		 y2);
}
#endif


#ifndef USE_PAINTER
void LyXScreen::drawFrame(int /*ft*/, int x, int y, int w, int h,
			  FL_COLOR /*col*/, int /*b*/)
{
// Implement this using X11 calls, and the repaint problems are gone!
// At least, I think that should do it since we only have them after
// one of these buttons are displayed now! Lars, it seems you've hit the
// nail :-) (Asger)
//	fl_winset(foreground);
//	fl_drw_frame(ft, x,  y,  w,  h, col, b);
	// This should be changed to draw a button like frame, in the
	// mean time we'll just use a regular rectangle
     
         // I want the buttons back before 0.12. OK, this is very simple,
         // like what is done in xforms sources.  (Ale)

	// This one is too dirty. Get rid of it.
	extern GC fl_gc;

	// Please comment this annonymous variable.
	int d = 2;

	// I think these calls to fl_color might make xforms sometimes
	// draw the wrong color on other objects.
	fl_color(FL_TOP_BCOL);
	XFillRectangle(fl_display, foreground, fl_gc,
		       x - d, y - d, w + 2 * d, d);
	fl_color(FL_BOTTOM_BCOL);
	XFillRectangle(fl_display, foreground, fl_gc,
		       x - d, y + h, w + 2 * d, d);
 
	// Now a couple of trapezoids
	XPoint pl[4], pr[4]; 
 
	pl[0].x = x - d;   pl[0].y = y - d;
	pl[1].x = x - d;   pl[1].y = y + h + d;
	pl[2].x = x;     pl[2].y = y + h;
	pl[3].x = x;     pl[3].y = y;
	
	pr[0].x = x + w + d; pr[0].y = y - d;
	pr[1].x = x + w + d; pr[1].y = y + h + d;
	pr[2].x = x + w;   pr[2].y = y + h;
	pr[3].x = x + w;   pr[3].y = y;
	
	fl_color(FL_LEFT_BCOL);
	XFillPolygon(fl_display,
		     foreground,
		     fl_gc, pl, 4,
		     Convex, CoordModeOrigin); 
	fl_color(FL_RIGHT_BCOL);
	XFillPolygon(fl_display,
		     foreground,
		     fl_gc, pr, 4,
		     Convex, CoordModeOrigin); 
}
#endif
