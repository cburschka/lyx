// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#include <algorithm>
using std::for_each;

#include <cstdlib>
#include <csignal>

#include <unistd.h>
#include <sys/wait.h>

#include "support/lstrings.h"

#ifdef __GNUG__
#pragma implementation
#endif

#include "commandtags.h"
#include "BufferView.h"
#include "bufferlist.h"
#include "LyXView.h"
#include "lyxfunc.h"
#include "insets/lyxinset.h"
#include "minibuffer.h"
#include "lyxscreen.h"

#ifndef NEW_WA
#include "up.xpm"
#include "down.xpm"
#endif

#include "debug.h"
#include "lyxdraw.h"
#include "lyx_gui_misc.h"
#include "BackStack.h"
#include "lyxtext.h"
#include "lyx_cb.h"
#include "gettext.h"
#include "layout.h"
#include "TextCache.h"
#include "intl.h"
#include "lyxrc.h"
#include "lyxrow.h"
#include "WorkArea.h"

using std::find_if;

extern BufferList bufferlist;
extern LyXRC * lyxrc;

void sigchldhandler(pid_t pid, int * status);

extern void SetXtermCursor(Window win);
extern bool input_prohibited;
extern bool selection_possible;
extern char ascii_type;
extern void MenuPasteSelection(char at);
extern InsetUpdateStruct * InsetUpdateList;
extern void UpdateInsetUpdateList();
extern void FreeUpdateTimer();

#ifndef NEW_WA
// This is _very_ temporary
FL_OBJECT * figinset_canvas;
#endif

BufferView::BufferView(LyXView * o, int xpos, int ypos,
		       int width, int height)
	: owner_(o)
{
	buffer_ = 0;
	text = 0;
	screen = 0;
#ifdef NEW_WA
	workarea = new WorkArea(this, xpos, ypos, width, height);
#else
	figinset_canvas = 0;
	work_area = 0;
	scrollbar = 0;
	button_down = 0;
	button_up = 0;
#endif
	timer_cursor = 0;
	create_view(xpos, ypos, width, height);
	current_scrollbar_value = 0;
	// Activate the timer for the cursor 
	fl_set_timer(timer_cursor, 0.4);
#ifdef NEW_WA
	workarea->setFocus();
#else
	fl_set_focus_object(owner_->getForm(), work_area);
#endif
	work_area_focus = true;
	lyx_focus = false;
	the_locking_inset = 0;
	inset_slept = false;
}


BufferView::~BufferView()
{
	delete text;
}


#ifdef USE_PAINTER
Painter & BufferView::painter() 
{
	return workarea->getPainter();
}
#endif


void BufferView::buffer(Buffer * b)
{
	lyxerr[Debug::INFO] << "Setting buffer in BufferView ("
			    << b << ")" << endl;
	if (buffer_) {
		insetSleep();
		buffer_->delUser(this);

		// Put the old text into the TextCache, but
		// only if the buffer is still loaded.
		// Also set the owner of the test to 0
		text->owner(0);
		textcache.add(text);
		if (lyxerr.debugging())
			textcache.show(lyxerr, "BufferView::buffer");
		
		text = 0;
	}

	// Set current buffer
	buffer_ = b;

	if (bufferlist.getState() == BufferList::CLOSING) return;
	
	// Nuke old image
	// screen is always deleted when the buffer is changed.
	delete screen;
	screen = 0;

	// If we are closing the buffer, use the first buffer as current
	if (!buffer_) {
		buffer_ = bufferlist.first();
	}

	if (buffer_) {
		lyxerr[Debug::INFO] << "Buffer addr: " << buffer_ << endl;
		buffer_->addUser(this);
		owner_->getMenus()->showMenus();
		// If we don't have a text object for this, we make one
		if (text == 0)
			resizeCurrentBuffer();
		else {
			updateScreen();
			updateScrollbar();
		}
		screen->first = screen->TopCursorVisible();
		redraw();
		updateAllVisibleBufferRelatedPopups();
		insetWakeup();
	} else {
		lyxerr[Debug::INFO] << "  No Buffer!" << endl;
		owner_->getMenus()->hideMenus();
		updateScrollbar();
#ifdef NEW_WA
		workarea->redraw();
#else
		fl_redraw_object(work_area);
#endif

		// Also remove all remaining text's from the testcache.
		// (there should not be any!) (if there is any it is a
		// bug!)
		if (lyxerr.debugging())
			textcache.show(lyxerr, "buffer delete all");
		textcache.clear();
	}
	// should update layoutchoice even if we don't have a buffer.
	owner_->updateLayoutChoice();
	owner_->getMiniBuffer()->Init();
	owner_->updateWindowTitle();
}


void BufferView::updateScreen()
{
	// Regenerate the screen.
	delete screen;
#ifdef NEW_WA
	screen = new LyXScreen(this,
			       workarea->getWin(),
			       workarea->getPixmap(),
			       workarea->workWidth(),
			       workarea->height(),
			       workarea->xpos(),
			       workarea->ypos(),
			       text);
#else
	screen = new LyXScreen(FL_ObjWin(work_area),
			       work_area->w,
			       work_area->h,
			       work_area->x,
			       work_area->y,
			       text);
#endif
}


#ifdef NEW_WA
void BufferView::resize(int xpos, int ypos, int width, int height)
{
	workarea->resize(xpos, ypos, width, height);
	update(3);
	redraw();
}
#endif


void BufferView::resize()
{
	// This will resize the buffer. (Asger)
	if (buffer_)
		resizeCurrentBuffer();
}


#ifndef NEW_WA
static bool lgb_hack = false;
#endif

void BufferView::redraw()
{
	lyxerr[Debug::INFO] << "BufferView::redraw()" << endl;
#ifdef NEW_WA
	workarea->redraw();
#else
	lgb_hack = true;
	fl_redraw_object(work_area);
	fl_redraw_object(scrollbar);
	fl_redraw_object(button_down);
	fl_redraw_object(button_up);
	lgb_hack = false;
#endif
}


void BufferView::fitCursor()
{
	if (screen) screen->FitCursor();
	updateScrollbar();
}


void BufferView::update()
{
	if (screen) screen->Update();
}


void BufferView::updateScrollbar()
{
	/* If the text is smaller than the working area, the scrollbar
	 * maximum must be the working area height. No scrolling will 
	 * be possible */

	if (!buffer_) {
#ifdef NEW_WA
		workarea->setScrollbar(0, 1.0);
#else
		fl_set_slider_value(scrollbar, 0);
		fl_set_slider_size(scrollbar, scrollbar->h);
#endif
		return;
	}
	
	static long max2 = 0;
	static long height2 = 0;

	long cbth = 0;
	long cbsf = 0;

	if (text)
		cbth = text->height;
	if (screen)
		cbsf = screen->first;

	// check if anything has changed.
#ifdef NEW_WA
	if (max2 == cbth &&
	    height2 == workarea->height() &&
	    current_scrollbar_value == cbsf)
		return; // no
	max2 = cbth;
	height2 = workarea->height();
	current_scrollbar_value = cbsf;
#else
	if (max2 == cbth &&
	    height2 == work_area->h &&
	    current_scrollbar_value == cbsf)
		return;	      // no
	max2 = cbth;
	height2 = work_area->h;
	current_scrollbar_value = cbsf;
#endif

	if (cbth <= height2) { // text is smaller than screen
#ifdef NEW_WA
		workarea->setScrollbar(0, 1.0); // right?
#else
		fl_set_slider_size(scrollbar, scrollbar->h);
#endif
		return;
	}

#ifdef NEW_WA
	long maximum_height = workarea->height() * 3 / 4 + cbth;
#else
	long maximum_height = work_area->h * 3 / 4 + cbth;
#endif
	long value = cbsf;

	// set the scrollbar
#ifdef NEW_WA
	double hfloat = workarea->height();
#else
	double hfloat = work_area->h;
#endif
	double maxfloat = maximum_height;

#ifdef NEW_WA
	float slider_size = 0.0;
	int slider_value = value;

	workarea->setScrollbarBounds(0, text->height - workarea->height());
	double lineh = text->DefaultHeight();
	workarea->setScrollbarIncrements(lineh);
	if (maxfloat > 0.0) {
		if ((hfloat / maxfloat) * float(height2) < 3)
			slider_size = 3.0/float(height2);
		else
			slider_size = hfloat / maxfloat;
	} else
		slider_size = hfloat;

	workarea->setScrollbar(slider_value, slider_size / workarea->height());
#else
	fl_set_slider_value(scrollbar, value);
	fl_set_slider_bounds(scrollbar, 0,
			     maximum_height - work_area->h);

	double lineh = text->DefaultHeight();
	fl_set_slider_increment(scrollbar, work_area->h-lineh, lineh);

	if (maxfloat > 0){
		if ((hfloat / maxfloat) * float(height2) < 3)
			fl_set_slider_size(scrollbar,
					   3 / float(height2));
		else
			fl_set_slider_size(scrollbar,
					   hfloat / maxfloat);
	} else
		fl_set_slider_size(scrollbar, hfloat);
	fl_set_slider_precision(scrollbar, 0);
#endif
}


void BufferView::redoCurrentBuffer()
{
	lyxerr[Debug::INFO] << "BufferView::redoCurrentBuffer" << endl;
	if (buffer_ && text) {
		resize();
		owner_->updateLayoutChoice();
	}
}


int BufferView::resizeCurrentBuffer()
{
	lyxerr[Debug::INFO] << "resizeCurrentBuffer" << endl;
	
	LyXParagraph * par = 0;
	LyXParagraph * selstartpar = 0;
	LyXParagraph * selendpar = 0;
	int pos = 0;
	int selstartpos = 0;
	int selendpos = 0;
	int selection = 0;
	int mark_set = 0;

	ProhibitInput();

	owner_->getMiniBuffer()->Set(_("Formatting document..."));   

	if (text) {
		par = text->cursor.par;
		pos = text->cursor.pos;
		selstartpar = text->sel_start_cursor.par;
		selstartpos = text->sel_start_cursor.pos;
		selendpar = text->sel_end_cursor.par;
		selendpos = text->sel_end_cursor.pos;
		selection = text->selection;
		mark_set = text->mark_set;
		delete text;
#ifdef NEW_WA
		text = new LyXText(this, workarea->workWidth(), buffer_);
#else
		text = new LyXText(work_area->w, buffer_);
#endif
	} else {
		// See if we have a text in TextCache that fits
		// the new buffer_ with the correct width.
#ifdef NEW_WA
		text = textcache.findFit(buffer_, workarea->workWidth());
#else
		text = textcache.findFit(buffer_, work_area->w);
#endif
		if (text) {
			if (lyxerr.debugging()) {
				lyxerr << "Found a LyXText that fits:\n";
				textcache.show(lyxerr, text);
			}
			// Set the owner of the newly found text
			text->owner(this);
			if (lyxerr.debugging())
				textcache.show(lyxerr, "resizeCurrentBuffer");
		} else {
#ifdef NEW_WA
			text = new LyXText(this, workarea->workWidth(), buffer_);
#else
			text = new LyXText(work_area->w, buffer_);
#endif
		}
	}
	updateScreen();

	if (par) {
		text->selection = true;
		/* at this point just to avoid the Delete-Empty-Paragraph
		 * Mechanism when setting the cursor */
		text->mark_set = mark_set;
		if (selection) {
			text->SetCursor(selstartpar, selstartpos);
			text->sel_cursor = text->cursor;
			text->SetCursor(selendpar, selendpos);
			text->SetSelection();
			text->SetCursor(par, pos);
		} else {
			text->SetCursor(par, pos);
			text->sel_cursor = text->cursor;
			text->selection = false;
		}
	}
	screen->first = screen->TopCursorVisible(); /* this will scroll the
						     * screen such that the
						     * cursor becomes
						     * visible */ 
	updateScrollbar();
	redraw();
	owner_->getMiniBuffer()->Init();
	SetState();
	AllowInput();

	// Now if the title form still exist kill it
	TimerCB(0, 0);

	return 0;
}


void BufferView::gotoError()
{
	if (!screen)
		return;
   
	screen->HideCursor();
	beforeChange();
	update(-2);
	LyXCursor tmp;

	if (!text->GotoNextError()) {
		if (text->cursor.pos 
		    || text->cursor.par != text->FirstParagraph()) {
			tmp = text->cursor;
			text->cursor.par = text->FirstParagraph();
			text->cursor.pos = 0;
			if (!text->GotoNextError()) {
				text->cursor = tmp;
				owner_->getMiniBuffer()
					->Set(_("No more errors"));
				LyXBell();
			}
		} else {
			owner_->getMiniBuffer()->Set(_("No more errors"));
			LyXBell();
		}
	}
	update(0);
	text->sel_cursor = text->cursor;
}


#ifdef NEW_WA
extern "C" {
	void C_BufferView_CursorToggleCB(FL_OBJECT * ob, long buf)
	{
		BufferView::CursorToggleCB(ob, buf);
	}
}
#else
extern "C" {
// Just a bunch of C wrappers around static members of BufferView
	void C_BufferView_UpCB(FL_OBJECT * ob, long buf)
	{
		BufferView::UpCB(ob, buf);
	}


	void C_BufferView_DownCB(FL_OBJECT * ob, long buf)
	{
		BufferView::DownCB(ob, buf);
	}


	void C_BufferView_ScrollCB(FL_OBJECT * ob, long buf)
	{
		BufferView::ScrollCB(ob, buf);
	}


	void C_BufferView_CursorToggleCB(FL_OBJECT * ob, long buf)
	{
		BufferView::CursorToggleCB(ob, buf);
	}


	int C_BufferView_work_area_handler(FL_OBJECT * ob, int event,
					   FL_Coord, FL_Coord, 
					   int key, void * xev)
	{
		return BufferView::work_area_handler(ob, event,
						     0, 0, key, xev);
	}
}
#endif


void BufferView::create_view(int xpos, int ypos, int width, int height)
{
	FL_OBJECT * obj;
#ifndef NEW_WA
	int const bw = abs(fl_get_border_width());

	// a hack for the figinsets (Matthias)
	// This one first, then it will probably be invisible. (Lgb)
	::figinset_canvas = figinset_canvas = obj = 
		  fl_add_canvas(FL_NORMAL_CANVAS,
				xpos + 1,
				ypos + 1, 1, 1, "");
	fl_set_object_boxtype(obj, FL_NO_BOX);
	fl_set_object_resize(obj, FL_RESIZE_ALL);
	fl_set_object_gravity(obj, NorthWestGravity, NorthWestGravity);

	// a box
	obj = fl_add_box(FL_BORDER_BOX, xpos, ypos,
			 width - 15,
			 height, "");
	fl_set_object_resize(obj, FL_RESIZE_ALL);
	fl_set_object_gravity(obj, NorthWestGravity, SouthEastGravity);

	// the free object
	work_area = obj = fl_add_free(FL_INPUT_FREE,
				      xpos + bw, ypos + bw,
				      width - 15 - 2 * bw /* scrollbarwidth */,
				      height - 2 * bw, "",
				      C_BufferView_work_area_handler);
	obj->wantkey = FL_KEY_TAB;
	obj->u_vdata = this; /* This is how we pass the BufferView
				to the work_area_handler. */
	fl_set_object_boxtype(obj, FL_DOWN_BOX);
	fl_set_object_resize(obj, FL_RESIZE_ALL);
	fl_set_object_gravity(obj, NorthWestGravity, SouthEastGravity);

	//
	// THE SCROLLBAR
	//

	// up - scrollbar button
	fl_set_border_width(-1); // to get visual feedback

	button_up = obj = fl_add_pixmapbutton(FL_TOUCH_BUTTON,
					      width - 15 + 4 * bw,
					      ypos,
					      15, 15, "");
	fl_set_object_boxtype(obj, FL_UP_BOX);
	fl_set_object_color(obj, FL_MCOL, FL_BLUE);
	fl_set_object_resize(obj, FL_RESIZE_ALL);
	fl_set_object_gravity(obj, NorthEastGravity, NorthEastGravity);
	fl_set_object_callback(obj, C_BufferView_UpCB, 0);
	obj->u_vdata = this;
	fl_set_pixmapbutton_data(obj, const_cast<char**>(up_xpm));

	// Remove the blue feedback rectangle
	fl_set_pixmapbutton_focus_outline(obj, 0);

	// the scrollbar slider
	fl_set_border_width(-bw);
	scrollbar = obj = fl_add_slider(FL_VERT_SLIDER,
					width - 15 + 4 * bw,
					ypos + 15,
					15, height - 30, "");
	fl_set_object_color(obj, FL_COL1, FL_MCOL);
	fl_set_object_boxtype(obj, FL_UP_BOX);
	fl_set_object_resize(obj, FL_RESIZE_ALL);
	fl_set_object_gravity(obj, NorthEastGravity, SouthEastGravity);
	fl_set_object_callback(obj, C_BufferView_ScrollCB, 0);
	obj->u_vdata = this;
	
	// down - scrollbar button
	fl_set_border_width(-1); // to get visible feedback

	button_down = obj = fl_add_pixmapbutton(FL_TOUCH_BUTTON,
						width - 15 + 4 * bw,
						ypos + height - 15,
						15, 15, "");
	fl_set_object_boxtype(obj, FL_UP_BOX);
	fl_set_object_color(obj, FL_MCOL, FL_BLUE);
	fl_set_object_resize(obj, FL_RESIZE_ALL);
	fl_set_object_gravity(obj, SouthEastGravity, SouthEastGravity);
	fl_set_object_callback(obj, C_BufferView_DownCB, 0);
	obj->u_vdata = this;
	fl_set_pixmapbutton_data(obj, const_cast<char**>(down_xpm));
	fl_set_border_width(-bw);

	// Remove the blue feedback rectangle
	fl_set_pixmapbutton_focus_outline(obj, 0);
#endif
	//
	// TIMERS
	//
	
	// timer_cursor
	timer_cursor = obj = fl_add_timer(FL_HIDDEN_TIMER,
					  0, 0, 0, 0, "Timer");
	fl_set_object_callback(obj, C_BufferView_CursorToggleCB, 0);
	obj->u_vdata = this;
}


// Callback for scrollbar up button
#ifdef NEW_WA
void BufferView::UpCB(long time, int button)
{
	if (buffer_ == 0) return;

	switch (button) {
	case 3:
		ScrollUpOnePage();
		break;
	case 2:
		ScrollDownOnePage();
		break;
	default:
		ScrollUp(time);
		break;
	}
}
#else
void BufferView::UpCB(FL_OBJECT * ob, long)
{
	BufferView * view = static_cast<BufferView*>(ob->u_vdata);
	
	if (view->buffer_ == 0) return;

	static long time = 0;
	XEvent const * ev2 = fl_last_event();
	if (ev2->type == ButtonPress || ev2->type == ButtonRelease) 
		time = 0;
	int button = fl_get_button_numb(ob);
	switch (button) {
	case 3:
		view->ScrollUpOnePage(time++); break;
	case 2:
		view->ScrollDownOnePage(time++); break;
	default:
		view->ScrollUp(time++); break;
	}
}
#endif


static inline
void waitForX()
{
	XSync(fl_get_display(), 0);
}


// Callback for scrollbar slider
#ifdef NEW_WA
void BufferView::ScrollCB(double value)
{
	extern bool cursor_follows_scrollbar;
	
	if (buffer_ == 0) return;

	current_scrollbar_value = long(value);
	if (current_scrollbar_value < 0)
		current_scrollbar_value = 0;
   
	if (!screen)
		return;

	screen->Draw(current_scrollbar_value);

	if (cursor_follows_scrollbar) {
		LyXText * vbt = text;
		int height = vbt->DefaultHeight();
		
		if (vbt->cursor.y < screen->first + height) {
			vbt->SetCursorFromCoordinates(0,
						      screen->first +
						      height);
		} else if (vbt->cursor.y >
			   screen->first + workarea->height() - height) {
			vbt->SetCursorFromCoordinates(0,
						      screen->first +
						      workarea->height()  -
						      height);
		}
	}
	waitForX();
}
#else
void BufferView::ScrollCB(FL_OBJECT * ob, long)
{
	BufferView * view = static_cast<BufferView*>(ob->u_vdata);
	extern bool cursor_follows_scrollbar;
	
	if (view->buffer_ == 0) return;

	view->current_scrollbar_value = long(fl_get_slider_value(ob));
	if (view->current_scrollbar_value < 0)
		view->current_scrollbar_value = 0;
   
	if (!view->screen)
		return;

	view->screen->Draw(view->current_scrollbar_value);

	if (cursor_follows_scrollbar) {
		LyXText * vbt = view->text;
		int height = vbt->DefaultHeight();
		
		if (vbt->cursor.y < view->screen->first + height) {
			vbt->SetCursorFromCoordinates(0,
						      view->screen->first +
						      height);
		}
		else if (vbt->cursor.y >
			 view->screen->first + view->work_area->h - height) {
			vbt->SetCursorFromCoordinates(0,
						      view->screen->first +
						      view->work_area->h  -
						      height);
		}
	}
	waitForX();
}
#endif


// Callback for scrollbar down button
#ifdef NEW_WA
void BufferView::DownCB(long time, int button)
{
	if (buffer_ == 0) return;
	
	switch (button) {
	case 2:
		ScrollUpOnePage();
		break;
	case 3:
		ScrollDownOnePage();
		break;
	default:
		ScrollDown(time);
		break;
	}
}
#else
void BufferView::DownCB(FL_OBJECT * ob, long)
{
	BufferView * view = static_cast<BufferView*>(ob->u_vdata);

	if (view->buffer_ == 0) return;
	
	XEvent const * ev2;
	static long time = 0;
	ev2 = fl_last_event();
	if (ev2->type == ButtonPress || ev2->type == ButtonRelease) 
		time = 0;
	int button = fl_get_button_numb(ob);
	switch (button) {
	case 2:
		view->ScrollUpOnePage(time++); break;
	case 3:
		view->ScrollDownOnePage(time++); break;
	default:
		view->ScrollDown(time++); break;
	}
}
#endif


#ifdef NEW_WA
int BufferView::ScrollUp(long time)
{
	if (buffer_ == 0) return 0;
	if (!screen) return 0;
   
	double value = workarea->getScrollbarValue();
   
	if (value == 0) return 0;

	float add_value =  (text->DefaultHeight()
			    + float(time) * float(time) * 0.125);
   
	if (add_value > workarea->height())
		add_value = float(workarea->height() -
				  text->DefaultHeight());
   
	value -= add_value;

	if (value < 0)
		value = 0;
   
	workarea->setScrollbarValue(value);
   
	ScrollCB(value); 
	return 0;
}
#else
int BufferView::ScrollUp(long time)
{
	if (buffer_ == 0) return 0;
	if (!screen) return 0;
   
	double value = fl_get_slider_value(scrollbar);
   
	if (value == 0) return 0;

	float add_value =  (text->DefaultHeight()
			    + float(time) * float(time) * 0.125);
   
	if (add_value > work_area->h)
		add_value = float(work_area->h -
				  text->DefaultHeight());
   
	value -= add_value;

	if (value < 0)
		value = 0;
   
	fl_set_slider_value(scrollbar, value);
   
	ScrollCB(scrollbar, 0); 
	return 0;
}
#endif


#ifdef NEW_WA
int BufferView::ScrollDown(long time)
{
	if (buffer_ == 0) return 0;
	if (!screen) return 0;
   
	double value= workarea->getScrollbarValue();
	pair<double, double> p = workarea->getScrollbarBounds();
	double max = p.second;
	
	if (value == max) return 0;

	float add_value =  (text->DefaultHeight()
			    + float(time) * float(time) * 0.125);
   
	if (add_value > workarea->height())
		add_value = float(workarea->height() -
				  text->DefaultHeight());
   
	value += add_value;
   
	if (value > max)
		value = max;

	workarea->setScrollbarValue(value);
	
	ScrollCB(value); 
	return 0;
}
#else
int BufferView::ScrollDown(long time)
{
	if (buffer_ == 0) return 0;
	if (!screen) return 0;
   
	double value= fl_get_slider_value(scrollbar);
	double min, max;
	fl_get_slider_bounds(scrollbar, &min, &max);

	if (value == max) return 0;

	float add_value =  (text->DefaultHeight()
			    + float(time) * float(time) * 0.125);
   
	if (add_value > work_area->h)
		add_value = float(work_area->h -
				  text->DefaultHeight());
   
	value += add_value;
   
	if (value > max)
		value = max;
   
	fl_set_slider_value(scrollbar, value);
   
	ScrollCB(scrollbar, 0); 
	return 0;
}
#endif


#ifdef NEW_WA
void BufferView::ScrollUpOnePage()
{
	if (buffer_ == 0) return;
	if (!screen) return;
   
	long y = screen->first;

	if (!y) return;

	Row * row = text->GetRowNearY(y);

	y = y - workarea->height() + row->height;

	workarea->setScrollbarValue(y);
	
	ScrollCB(y); 
}
#else
void BufferView::ScrollUpOnePage(long /*time*/)
{
	if (buffer_ == 0) return;
	if (!screen) return;
   
	long y = screen->first;

	if (!y) return;

	Row * row = text->GetRowNearY(y);

	y = y - work_area->h + row->height;
	
	fl_set_slider_value(scrollbar, y);
   
	ScrollCB(scrollbar, 0); 
}
#endif


#ifdef NEW_WA
void BufferView::ScrollDownOnePage()
{
	if (buffer_ == 0) return;
	if (!screen) return;
   
	long y = screen->first;

	if (y > text->height - workarea->height())
		return;
   
	y += workarea->height();
	text->GetRowNearY(y);

	workarea->setScrollbarValue(y);
	
	ScrollCB(y); 
}
#else
void BufferView::ScrollDownOnePage(long /*time*/)
{
	if (buffer_ == 0) return;
	if (!screen) return;
   
	double min, max;
	fl_get_slider_bounds(scrollbar, &min, &max);
	long y = screen->first;

	if (y > text->height - work_area->h)
		return;
   
	y += work_area->h;
	text->GetRowNearY(y);

	fl_set_slider_value(scrollbar, y);
   
	ScrollCB(scrollbar, 0); 
}
#endif


#ifndef NEW_WA
int BufferView::work_area_handler(FL_OBJECT * ob, int event,
				  FL_Coord, FL_Coord ,
				  int /*key*/, void * xev)
{
	static int x_old = -1;
	static int y_old = -1;
	static long scrollbar_value_old = -1;
	
	XEvent * ev = static_cast<XEvent*>(xev);
	BufferView * view = static_cast<BufferView*>(ob->u_vdata);

	// If we don't have a view yet; return
	if (!view || quitting) return 0;

	switch (event){   
	case FL_DRAW:
		view->workAreaExpose(); 
		break;
	case FL_PUSH:
		view->WorkAreaButtonPress(ob, 0, 0, 0, ev, 0);
		break; 
	case FL_RELEASE:
		view->WorkAreaButtonRelease(ob, 0, 0, 0, ev, 0);
		break;
	case FL_MOUSE:
		if (ev->xmotion.x != x_old || 
		    ev->xmotion.y != y_old ||
		    view->current_scrollbar_value != scrollbar_value_old) {
			x_old = ev->xmotion.x;
			y_old = ev->xmotion.y;
			scrollbar_value_old = view->current_scrollbar_value;
			view->WorkAreaMotionNotify(ob, 0, 0, 0, ev, 0);
		}
		break;
		// Done by the raw callback:
		//  case FL_KEYBOARD:
		//  WorkAreaKeyPress(ob, 0, 0, 0, ev, 0); break;
	case FL_FOCUS:
		if (!view->owner_->getMiniBuffer()->shows_no_match)
			view->owner_->getMiniBuffer()->Init();
		view->owner_->getMiniBuffer()->shows_no_match = false;
		view->work_area_focus = true;
		fl_set_timer(view->timer_cursor, 0.4);
		break;
	case FL_UNFOCUS:
		view->owner_->getMiniBuffer()->ExecCommand();
		view->work_area_focus = false;
		break;
	case FL_ENTER:
		SetXtermCursor(view->owner_->getForm()->window);
		// reset the timer
		view->lyx_focus = true;
		fl_set_timer(view->timer_cursor, 0.4);
		break;
	case FL_LEAVE: 
		if (!input_prohibited)
			XUndefineCursor(fl_display,
					view->owner_->getForm()->window);
		view->lyx_focus = false; // This is not an absolute truth
		// but if it is not true, it will be changed within a blink
		// of an eye. ... Not good enough... use regulare timeperiod
		//fl_set_timer(view->timer_cursor, 0.01); // 0.1 sec blink
		fl_set_timer(view->timer_cursor, 0.4); // 0.4 sec blink
		break;
	case FL_DBLCLICK: 
		// select a word
		if (!view->the_locking_inset) {
			if (view->screen && ev->xbutton.button == 1) {
				view->screen->HideCursor();
				view->screen->ToggleSelection();
				view->text->SelectWord();
				view->screen->ToggleSelection(false);
				/* This will fit the cursor on the screen
				 * if necessary */
				view->update(0);
			}
		}
		break;
	case FL_TRPLCLICK:
		// select a line
		if (view->buffer_ && view->screen && ev->xbutton.button == 1) {
			view->screen->HideCursor(); 
			view->screen->ToggleSelection();
			view->text->CursorHome();
			view->text->sel_cursor = view->text->cursor;
			view->text->CursorEnd();
			view->text->SetSelection();
			view->screen->ToggleSelection(false); 
			/* This will fit the cursor on the screen
			 * if necessary */
			view->update(0);
		}
		break;
	case FL_OTHER:
		view->WorkAreaSelectionNotify(ob,
					      view->owner_->getForm()->window,
					      0, 0, ev, 0); 
		break;
	}
	return 1;
}
#endif


#ifdef NEW_WA
void BufferView::WorkAreaMotionNotify(int x, int y, unsigned int state)
{
	if (buffer_ == 0 || !screen) return;

	// Check for inset locking
	if (the_locking_inset) {
		LyXCursor cursor = text->cursor;
		the_locking_inset->
			InsetMotionNotify(x - cursor.x,
					  y - cursor.y,
					  state);
		return;
	}

	// Only use motion with button 1
	if (!state & Button1MotionMask)
		return; 
   
	/* The selection possible is needed, that only motion events are 
	 * used, where the bottom press event was on the drawing area too */
	if (selection_possible) {
		screen->HideCursor();

		text->SetCursorFromCoordinates(x, y + screen->first);
      
		if (!text->selection)
			update(-3); // Maybe an empty line was deleted
      
		text->SetSelection();
		screen->ToggleToggle();
		if (screen->FitCursor())
			updateScrollbar(); 
		screen->ShowCursor();
	}
	return;
}
#else
int BufferView::WorkAreaMotionNotify(FL_OBJECT * ob, Window,
				     int /*w*/, int /*h*/,
				     XEvent * ev, void * /*d*/)
{
	if (buffer_ == 0) return 0;
	if (!screen) return 0;

	// Check for inset locking
	if (the_locking_inset) {
		LyXCursor cursor = text->cursor;
		the_locking_inset->
			InsetMotionNotify(ev->xbutton.x - ob->x - cursor.x,
					  ev->xbutton.y - ob->y -
					  (cursor.y),
					  ev->xbutton.state);
		return 0;
	}

	// Only use motion with button 1
	if (!ev->xmotion.state & Button1MotionMask)
		return 0; 
   
	/* The selection possible is needed, that only motion events are 
	 * used, where the bottom press event was on the drawing area too */
	if (selection_possible) {
		screen->HideCursor();

		text->SetCursorFromCoordinates(ev->xbutton.x - ob->x,
					       ev->xbutton.y - ob->y +
					       screen->first);
      
		if (!text->selection)
			update(-3); // Maybe an empty line was deleted
      
		text->SetSelection();
		screen->ToggleToggle();
		if (screen->FitCursor())
			updateScrollbar(); 
		screen->ShowCursor();
	}
	return 0;
}
#endif

#ifdef USE_PAINTER
extern int bibitemMaxWidth(Painter &, LyXFont const &);
#else
extern int bibitemMaxWidth(LyXFont const &);
#endif

// Single-click on work area
#ifdef NEW_WA
void BufferView::WorkAreaButtonPress(int xpos, int ypos, unsigned int button)
{
	last_click_x = -1;
	last_click_y = -1;

	if (buffer_ == 0 || !screen) return;

	Inset * inset_hit = checkInsetHit(xpos, ypos);

	// ok ok, this is a hack.
	if (button == 4 || button == 5) {
		switch (button) {
		case 4:
			ScrollUp(100); // This number is only temporary
			break;
		case 5:
			ScrollDown(100);
			break;
		}
	}
	
	if (the_locking_inset) {
		// We are in inset locking mode
		
		/* Check whether the inset was hit. If not reset mode,
		   otherwise give the event to the inset */
		if (inset_hit) {
			the_locking_inset->
				InsetButtonPress(xpos, ypos,
						 button);
			return;
		} else {
			unlockInset(the_locking_inset);
		}
	}
	
	selection_possible = true;
	screen->HideCursor();
	
	// Right button mouse click on a table
	if (button == 3 &&
	    (text->cursor.par->table ||
	     text->MouseHitInTable(xpos, ypos + screen->first))) {
		// Set the cursor to the press-position
		text->SetCursorFromCoordinates(xpos, ypos + screen->first);
		bool doit = true;
		
		// Only show the table popup if the hit is in
		// the table, too
		if (!text->HitInTable(text->cursor.row, xpos))
			doit = false;
		
		// Hit above or below the table?
		if (doit) {
			if (!text->selection) {
				screen->ToggleSelection();
				text->ClearSelection();
				text->FullRebreak();
				screen->Update();
				updateScrollbar();
			}
			// Popup table popup when on a table.
			// This is obviously temporary, since we
			// should be able to popup various
			// context-sensitive-menus with the
			// the right mouse. So this should be done more
			// general in the future. Matthias.
			selection_possible = false;
			owner_->getLyXFunc()
				->Dispatch(LFUN_LAYOUT_TABLE,
					   "true");
			return;
		}
	}
	
	int screen_first = screen->first;
	
	// Middle button press pastes if we have a selection
	bool paste_internally = false;
	if (button == 2
	    && text->selection) {
		owner_->getLyXFunc()->Dispatch(LFUN_COPY);
		paste_internally = true;
	}
	
	// Clear the selection
	screen->ToggleSelection();
	text->ClearSelection();
	text->FullRebreak();
	screen->Update();
	updateScrollbar();
	
	// Single left click in math inset?
	if (inset_hit != 0 && inset_hit->Editable() == 2) {
		// Highly editable inset, like math
		selection_possible = false;
		owner_->updateLayoutChoice();
		owner_->getMiniBuffer()->Set(inset_hit->EditMessage());
		inset_hit->Edit(xpos, ypos);
		return;
	} 
	
	// Right click on a footnote flag opens float menu
	if (button == 3) { 
		selection_possible = false;
		return;
	}
	
	text->SetCursorFromCoordinates(xpos, ypos + screen_first);
	text->FinishUndo();
	text->sel_cursor = text->cursor;
	text->cursor.x_fix = text->cursor.x;
	
	owner_->updateLayoutChoice();
	if (screen->FitCursor()){
		updateScrollbar();
		selection_possible = false;
	}
	
	// Insert primary selection with middle mouse
	// if there is a local selection in the current buffer,
	// insert this
	if (button == 2) {
		if (paste_internally)
			owner_->getLyXFunc()->Dispatch(LFUN_PASTE);
		else
			owner_->getLyXFunc()->Dispatch(LFUN_PASTESELECTION,
						       "paragraph");
		selection_possible = false;
		return;
	}
}
#else
int BufferView::WorkAreaButtonPress(FL_OBJECT * ob, Window,
				    int /*w*/, int /*h*/,
				    XEvent * ev, void */*d*/)
{
	last_click_x = -1;
	last_click_y = -1;

	if (buffer_ == 0) return 0;
	if (!screen) return 0;

	int const x = ev->xbutton.x - ob->x;
	int const y = ev->xbutton.y - ob->y;
	// If we hit an inset, we have the inset coordinates in these
	// and inset_hit points to the inset.  If we do not hit an
	// inset, inset_hit is 0, and inset_x == x, inset_y == y.
	int inset_x = x;
	int inset_y = y;
	Inset * inset_hit = checkInsetHit(inset_x, inset_y);

	// ok ok, this is a hack.
	int button = ev->xbutton.button;
	if (button == 4 || button == 5) goto wheel;

	{
		if (the_locking_inset) {
			// We are in inset locking mode
		
			/* Check whether the inset was hit. If not reset mode,
			   otherwise give the event to the inset */
			if (inset_hit != 0) {
				the_locking_inset->
					InsetButtonPress(inset_x, inset_y,
							 button);
				return 0;
			} else {
				unlockInset(the_locking_inset);
			}
		}

		selection_possible = true;
		screen->HideCursor();
	
		// Right button mouse click on a table
		if (button == 3 &&
		    (text->cursor.par->table ||
		     text->MouseHitInTable(x, y + screen->first))) {
			// Set the cursor to the press-position
			text->SetCursorFromCoordinates(x, y + screen->first);
			bool doit = true;
		
			// Only show the table popup if the hit is in
			// the table, too
			if (!text->HitInTable(text->cursor.row, x))
				doit = false;
		
			// Hit above or below the table?
			if (doit) {
				if (!text->selection) {
					screen->ToggleSelection();
					text->ClearSelection();
					text->FullRebreak();
					screen->Update();
					updateScrollbar();
				}
				// Popup table popup when on a table.
				// This is obviously temporary, since we
				// should be able to popup various
				// context-sensitive-menus with the
				// the right mouse. So this should be done more
				// general in the future. Matthias.
				selection_possible = false;
				owner_->getLyXFunc()
					->Dispatch(LFUN_LAYOUT_TABLE,
						   "true");
				return 0;
			}
		}
	
		int screen_first = screen->first;
	
		// Middle button press pastes if we have a selection
		bool paste_internally = false;
		if (button == 2  // && !buffer_->the_locking_inset
		    && text->selection) {
			owner_->getLyXFunc()->Dispatch(LFUN_COPY);
			paste_internally = true;
		}
	
		// Clear the selection
		screen->ToggleSelection();
		text->ClearSelection();
		text->FullRebreak();
		screen->Update();
		updateScrollbar();
		
		// Single left click in math inset?
		if (inset_hit != 0 && inset_hit->Editable() == 2) {
			// Highly editable inset, like math
			selection_possible = false;
			owner_->updateLayoutChoice();
			owner_->getMiniBuffer()->Set(inset_hit->EditMessage());
			inset_hit->Edit(inset_x, inset_y);
			return 0;
		} 

		// Right click on a footnote flag opens float menu
		if (button == 3) { 
			selection_possible = false;
			return 0;
		}
	
		text->SetCursorFromCoordinates(x, y + screen_first);
		text->FinishUndo();
		text->sel_cursor = text->cursor;
		text->cursor.x_fix = text->cursor.x;
	
		owner_->updateLayoutChoice();
		if (screen->FitCursor()){
			updateScrollbar();
			selection_possible = false;
		}

		// Insert primary selection with middle mouse
		// if there is a local selection in the current buffer,
		// insert this
		if (button == 2) { //  && !buffer_->the_locking_inset){
			if (paste_internally)
				owner_->getLyXFunc()->Dispatch(LFUN_PASTE);
			else
				owner_->getLyXFunc()->Dispatch(LFUN_PASTESELECTION,
							       "paragraph");
			selection_possible = false;
			return 0;
		}
	}
	goto out;
  wheel:
	{
		// I am not quite sure if this is the correct place to put
		// this, but it will not cause any harm.
		// Patch from Mark Huang (markman@mit.edu) to make LyX
		// recognise button 4 and 5. This enables LyX use use
		// the scrollwhell on certain mice for something useful. (Lgb)
		// Added wheel acceleration detection code. (Rvdk)
		static Time lastTime = 0;
		int diff = ev->xbutton.time - lastTime;
		int scroll = int(1.0 + (4.0 / (abs(diff) + 1.0)) * 200.0);
		switch (button) {
		case 4:
			ScrollUp(scroll);
			break;
		case 5:
			ScrollDown(scroll);
			break;
		}
		lastTime = ev->xbutton.time;
		return 0;
	}
  out:
	last_click_x = x;
	last_click_y = y;
	
	return 0;
}
#endif


#ifdef NEW_WA
void BufferView::WorkAreaButtonRelease(int x, int y, unsigned int button)
{
	if (buffer_ == 0 || screen == 0) return;

	// If we hit an inset, we have the inset coordinates in these
	// and inset_hit points to the inset.  If we do not hit an
	// inset, inset_hit is 0, and inset_x == x, inset_y == y.
	Inset * inset_hit = checkInsetHit(x, y);

	if (the_locking_inset) {
		// We are in inset locking mode.

		/* LyX does a kind of work-area grabbing for insets.
		   Only a ButtonPress Event outside the inset will 
		   force a InsetUnlock. */
		the_locking_inset->
			InsetButtonRelease(x, x, button);
		return;
	}
	
	selection_possible = false;
        if (text->cursor.par->table) {
                int cell = text->
                        NumberOfCell(text->cursor.par,
                                     text->cursor.pos);
                if (text->cursor.par->table->IsContRow(cell) &&
                    text->cursor.par->table->
                    CellHasContRow(text->cursor.par->table->
                                   GetCellAbove(cell))<0) {
                        text->CursorUp();
                }
        }
	
	if (button >= 2) return;

	SetState();
	owner_->getMiniBuffer()->Set(CurrentState());

	// Did we hit an editable inset?
	if (inset_hit != 0) {
		// Inset like error, notes and figures
		selection_possible = false;
#ifdef WITH_WARNINGS
#warning fix this proper in 0.13
#endif
		// Following a ref shouldn't issue
		// a push on the undo-stack
		// anylonger, now that we have
		// keybindings for following
		// references and returning from
		// references.  IMHO though, it
		// should be the inset's own business
		// to push or not push on the undo
		// stack. They don't *have* to
		// alter the document...
		// (Joacim)
		// ...or maybe the SetCursorParUndo()
		// below isn't necessary at all anylonger?
		if (inset_hit->LyxCode() == Inset::REF_CODE) {
			text->SetCursorParUndo();
		}

		owner_->getMiniBuffer()->Set(inset_hit->EditMessage());
		inset_hit->Edit(x, y);
		return;
	}

	// check whether we want to open a float
	if (text) {
		bool hit = false;
		char c = ' ';
		if (text->cursor.pos <
		    text->cursor.par->Last()) {
			c = text->cursor.par->
				GetChar(text->cursor.pos);
		}
		if (c == LyXParagraph::META_FOOTNOTE
		    || c == LyXParagraph::META_MARGIN
		    || c == LyXParagraph::META_FIG
		    || c == LyXParagraph::META_TAB
		    || c == LyXParagraph::META_WIDE_FIG
		    || c == LyXParagraph::META_WIDE_TAB
                    || c == LyXParagraph::META_ALGORITHM){
			hit = true;
		} else if (text->cursor.pos - 1 >= 0) {
			c = text->cursor.par->
				GetChar(text->cursor.pos - 1);
			if (c == LyXParagraph::META_FOOTNOTE
			    || c == LyXParagraph::META_MARGIN
			    || c == LyXParagraph::META_FIG
			    || c == LyXParagraph::META_TAB
			    || c == LyXParagraph::META_WIDE_FIG 
			    || c == LyXParagraph::META_WIDE_TAB
			    || c == LyXParagraph::META_ALGORITHM){
				// We are one step too far to the right
				text->CursorLeft();
				hit = true;
			}
		}
		if (hit == true) {
			toggleFloat();
			selection_possible = false;
			return;
		}
	}

	// Do we want to close a float? (click on the float-label)
	if (text->cursor.row->par->footnoteflag == 
	    LyXParagraph::OPEN_FOOTNOTE
	    //&& text->cursor.pos == 0
	    && text->cursor.row->previous &&
	    text->cursor.row->previous->par->
	    footnoteflag != LyXParagraph::OPEN_FOOTNOTE){
		LyXFont font (LyXFont::ALL_SANE);
		font.setSize(LyXFont::SIZE_FOOTNOTE);

		int box_x = 20; // LYX_PAPER_MARGIN;
		box_x += font.textWidth(" wide-tab ", 10);

		int screen_first = screen->first;

		if (x < box_x
		    && y + screen_first > text->cursor.y -
		    text->cursor.row->baseline
		    && y + screen_first < text->cursor.y -
		    text->cursor.row->baseline
		    + font.maxAscent() * 1.2 + font.maxDescent() * 1.2) {
			toggleFloat();
			selection_possible = false;
			return;
		}
	}

	// Maybe we want to edit a bibitem ale970302
#ifdef USE_PAINTER
	if (text->cursor.par->bibkey && x < 20 + 
	    bibitemMaxWidth(painter(),
			    textclasslist
			    .TextClass(buffer_->
				       params.textclass).defaultfont())) {
		text->cursor.par->bibkey->Edit(0, 0);
	}
#else
	if (text->cursor.par->bibkey && x < 20 + 
	    bibitemMaxWidth(textclasslist
			    .TextClass(buffer_->
				       params.textclass).defaultfont())) {
		text->cursor.par->bibkey->Edit(0, 0);
	}
#endif

	return;
}
#else
int BufferView::WorkAreaButtonRelease(FL_OBJECT * ob, Window ,
				      int /*w*/, int /*h*/,
				      XEvent * ev, void * /*d*/)
{
	if (buffer_ == 0 || screen == 0) return 0;

	int const x = ev->xbutton.x - ob->x;
	int const y = ev->xbutton.y - ob->y;

	// If we hit an inset, we have the inset coordinates in these
	// and inset_hit points to the inset.  If we do not hit an
	// inset, inset_hit is 0, and inset_x == x, inset_y == y.
	int inset_x = x;
	int inset_y = y;
	Inset * inset_hit = checkInsetHit(inset_x, inset_y);

	if (the_locking_inset) {
		// We are in inset locking mode.

		/* LyX does a kind of work-area grabbing for insets.
		   Only a ButtonPress Event outside the inset will 
		   force a InsetUnlock. */
		the_locking_inset->
			InsetButtonRelease(inset_x, inset_y, 
					   ev->xbutton.button);
		return 0;
	}
	
	selection_possible = false;
        if (text->cursor.par->table) {
                int cell = text->
                        NumberOfCell(text->cursor.par,
                                     text->cursor.pos);
                if (text->cursor.par->table->IsContRow(cell) &&
                    text->cursor.par->table->
                    CellHasContRow(text->cursor.par->table->
                                   GetCellAbove(cell))<0) {
                        text->CursorUp();
                }
        }
	
	if (ev->xbutton.button >= 2)
		return 0;

	// Make sure that the press was not far from the release
	if ((abs(last_click_x - x) >= 5) ||
	    (abs(last_click_y - y) >= 5)) {
		return 0;
	}
	SetState();
	owner_->getMiniBuffer()->Set(CurrentState());

	// Did we hit an editable inset?
	if (inset_hit != 0) {
		// Inset like error, notes and figures
		selection_possible = false;
#ifdef WITH_WARNINGS
#warning fix this proper in 0.13
#endif
		// Following a ref shouldn't issue
		// a push on the undo-stack
		// anylonger, now that we have
		// keybindings for following
		// references and returning from
		// references.  IMHO though, it
		// should be the inset's own business
		// to push or not push on the undo
		// stack. They don't *have* to
		// alter the document...
		// (Joacim)
		// ...or maybe the SetCursorParUndo()
		// below isn't necessary at all anylonger?
		if (inset_hit->LyxCode() == Inset::REF_CODE) {
			text->SetCursorParUndo();
		}

		owner_->getMiniBuffer()->Set(inset_hit->EditMessage());
		inset_hit->Edit(inset_x, inset_y);
		return 0;
	}

	// check whether we want to open a float
	if (text) {
		bool hit = false;
		char c = ' ';
		if (text->cursor.pos <
		    text->cursor.par->Last()) {
			c = text->cursor.par->
				GetChar(text->cursor.pos);
		}
		if (c == LyXParagraph::META_FOOTNOTE
		    || c == LyXParagraph::META_MARGIN
		    || c == LyXParagraph::META_FIG
		    || c == LyXParagraph::META_TAB
		    || c == LyXParagraph::META_WIDE_FIG
		    || c == LyXParagraph::META_WIDE_TAB
                    || c == LyXParagraph::META_ALGORITHM){
			hit = true;
		} else if (text->cursor.pos - 1 >= 0) {
			c = text->cursor.par->
				GetChar(text->cursor.pos - 1);
			if (c == LyXParagraph::META_FOOTNOTE
			    || c == LyXParagraph::META_MARGIN
			    || c == LyXParagraph::META_FIG
			    || c == LyXParagraph::META_TAB
			    || c == LyXParagraph::META_WIDE_FIG 
			    || c == LyXParagraph::META_WIDE_TAB
			    || c == LyXParagraph::META_ALGORITHM){
				// We are one step too far to the right
				text->CursorLeft();
				hit = true;
			}
		}
		if (hit == true) {
			toggleFloat();
			selection_possible = false;
			return 0;
		}
	}

	// Do we want to close a float? (click on the float-label)
	if (text->cursor.row->par->footnoteflag == 
	    LyXParagraph::OPEN_FOOTNOTE
	    //&& text->cursor.pos == 0
	    && text->cursor.row->previous &&
	    text->cursor.row->previous->par->
	    footnoteflag != LyXParagraph::OPEN_FOOTNOTE){
		LyXFont font (LyXFont::ALL_SANE);
		font.setSize(LyXFont::SIZE_FOOTNOTE);

		int box_x = 20; // LYX_PAPER_MARGIN;
		box_x += font.textWidth(" wide-tab ", 10);

		int screen_first = screen->first;

		if (x < box_x
		    && y + screen_first > text->cursor.y -
		    text->cursor.row->baseline
		    && y + screen_first < text->cursor.y -
		    text->cursor.row->baseline
		    + font.maxAscent() * 1.2 + font.maxDescent() * 1.2) {
			toggleFloat();
			selection_possible = false;
			return 0;
		}
	}

	// Maybe we want to edit a bibitem ale970302
	if (text->cursor.par->bibkey && x < 20 + 
	    bibitemMaxWidth(textclasslist
			    .TextClass(buffer_->
				       params.textclass).defaultfont())) {
		text->cursor.par->bibkey->Edit(0, 0);
	}

	return 0;
}
#endif

/* 
 * Returns an inset if inset was hit. 0 otherwise.
 * If hit, the coordinates are changed relative to the inset. 
 * Otherwise coordinates are not changed, and false is returned.
 */
#ifdef USE_PAINTER
Inset * BufferView::checkInsetHit(int & x, int & y)
{
	if (!getScreen())
		return 0;
  
	int y_tmp = y + getScreen()->first;
  
	LyXCursor & cursor = text->cursor;
	LyXDirection direction = text->real_current_font.getFontDirection();

	if (cursor.pos < cursor.par->Last()
	    && cursor.par->GetChar(cursor.pos) == LyXParagraph::META_INSET
	    && cursor.par->GetInset(cursor.pos)
	    && cursor.par->GetInset(cursor.pos)->Editable()) {

		// Check whether the inset really was hit
		Inset * tmpinset = cursor.par->GetInset(cursor.pos);
		LyXFont font = text->GetFont(cursor.par, cursor.pos);
		int start_x, end_x;
		if (direction == LYX_DIR_LEFT_TO_RIGHT) {
			start_x = cursor.x;
			end_x = cursor.x + tmpinset->width(painter(), font);
		} else {
			start_x = cursor.x - tmpinset->width(painter(), font);
			end_x = cursor.x;
		}

		if (x > start_x && x < end_x
		    && y_tmp > cursor.y - tmpinset->ascent(painter(), font)
		    && y_tmp < cursor.y + tmpinset->descent(painter(), font)) {
			x = x - start_x;
			// The origin of an inset is on the baseline
			y = y_tmp - (cursor.y); 
			return tmpinset;
		}
	}

	if (cursor.pos - 1 >= 0
		   && cursor.par->GetChar(cursor.pos - 1) == LyXParagraph::META_INSET
		   && cursor.par->GetInset(cursor.pos - 1)
		   && cursor.par->GetInset(cursor.pos - 1)->Editable()) {
		text->CursorLeft();
		Inset * tmpinset = cursor.par->GetInset(cursor.pos);
		LyXFont font = text->GetFont(cursor.par, cursor.pos);
		int start_x, end_x;
		if (direction == LYX_DIR_LEFT_TO_RIGHT) {
			start_x = cursor.x;
			end_x = cursor.x + tmpinset->width(painter(), font);
		} else {
			start_x = cursor.x - tmpinset->width(painter(), font);
			end_x = cursor.x;
		}
		if (x > start_x && x < end_x
		    && y_tmp > cursor.y - tmpinset->ascent(painter(), font)
		    && y_tmp < cursor.y + tmpinset->descent(painter(), font)) {
			x = x - start_x;
			// The origin of an inset is on the baseline
			y = y_tmp - (cursor.y); 
			return tmpinset;
		} else {
			text->CursorRight();
			return 0;
		}
	}
	return 0;
}
#else
Inset * BufferView::checkInsetHit(int & x, int & y)
{
	if (!getScreen())
		return 0;
  
	int y_tmp = y + getScreen()->first;
  
	LyXCursor & cursor = text->cursor;
	LyXDirection direction = text->real_current_font.getFontDirection();

	if (cursor.pos < cursor.par->Last()
	    && cursor.par->GetChar(cursor.pos) == LyXParagraph::META_INSET
	    && cursor.par->GetInset(cursor.pos)
	    && cursor.par->GetInset(cursor.pos)->Editable()) {

		// Check whether the inset really was hit
		Inset * tmpinset = cursor.par->GetInset(cursor.pos);
		LyXFont font = text->GetFont(cursor.par, cursor.pos);
		int start_x, end_x;
		if (direction == LYX_DIR_LEFT_TO_RIGHT) {
			start_x = cursor.x;
			end_x = cursor.x + tmpinset->Width(font);
		} else {
			start_x = cursor.x - tmpinset->Width(font);
			end_x = cursor.x;
		}

		if (x > start_x && x < end_x
		    && y_tmp > cursor.y - tmpinset->Ascent(font)
		    && y_tmp < cursor.y + tmpinset->Descent(font)) {
			x = x - start_x;
			// The origin of an inset is on the baseline
			y = y_tmp - (cursor.y); 
			return tmpinset;
		}
	}

	if (cursor.pos - 1 >= 0
		   && cursor.par->GetChar(cursor.pos - 1) == LyXParagraph::META_INSET
		   && cursor.par->GetInset(cursor.pos - 1)
		   && cursor.par->GetInset(cursor.pos - 1)->Editable()) {
		text->CursorLeft();
		Inset * tmpinset = cursor.par->GetInset(cursor.pos);
		LyXFont font = text->GetFont(cursor.par, cursor.pos);
		int start_x, end_x;
		if (direction == LYX_DIR_LEFT_TO_RIGHT) {
			start_x = cursor.x;
			end_x = cursor.x + tmpinset->Width(font);
		} else {
			start_x = cursor.x - tmpinset->Width(font);
			end_x = cursor.x;
		}
		if (x > start_x && x < end_x
		    && y_tmp > cursor.y - tmpinset->Ascent(font)
		    && y_tmp < cursor.y + tmpinset->Descent(font)) {
			x = x - start_x;
			// The origin of an inset is on the baseline
			y = y_tmp - (cursor.y); 
			return tmpinset;
		} else {
			text->CursorRight();
			return 0;
		}
	}
	return 0;
}
#endif

#ifdef NEW_WA
void BufferView::workAreaExpose()
{
	// this is a hack to ensure that we only call this through
	// BufferView::redraw().
	//if (!lgb_hack) {
	//	redraw();
	//}
	
	static int work_area_width = -1;
	static int work_area_height = -1;

	bool widthChange = workarea->workWidth() != work_area_width;
	bool heightChange = workarea->height() != work_area_height;

	// update from work area
	work_area_width = workarea->workWidth();
	work_area_height = workarea->height();
	if (buffer_ != 0) {
		if (widthChange) {
			// All buffers need a resize
			bufferlist.resize();

			// Remove all texts from the textcache
			// This is not _really_ what we want to do. What
			// we really want to do is to delete in textcache
			// that does not have a BufferView with matching
			// width, but as long as we have only one BufferView
			// deleting all gives the same result.
			if (lyxerr.debugging())
				textcache.show(lyxerr, "Expose delete all");
			textcache.clear();
		} else if (heightChange) {
			// Rebuild image of current screen
			updateScreen();
			// fitCursor() ensures we don't jump back
			// to the start of the document on vertical
			// resize
			fitCursor();

			// The main window size has changed, repaint most stuff
			redraw();
			// ...including the minibuffer
			owner_->getMiniBuffer()->Init();

		} else if (screen) screen->Redraw();
	} else {
		// Grey box when we don't have a buffer
		workarea->greyOut();
	}

	// always make sure that the scrollbar is sane.
	updateScrollbar();
	owner_->updateLayoutChoice();
	return;
}
#else
int BufferView::workAreaExpose()
{
	if (!work_area || !work_area->form->visible) 
		return 1;

	// this is a hack to ensure that we only call this through
	// BufferView::redraw().
	if (!lgb_hack) {
		redraw();
	}
	
	static int work_area_width = work_area->w;
	static int work_area_height = work_area->h;

	bool widthChange = work_area->w != work_area_width;
	bool heightChange = work_area->h != work_area_height;

	// update from work area
	work_area_width = work_area->w;
	work_area_height = work_area->h;
	if (buffer_ != 0) {
		if (widthChange) {
			// All buffers need a resize
			bufferlist.resize();

			// Remove all texts from the textcache
			// This is not _really_ what we want to do. What
			// we really want to do is to delete in textcache
			// that does not have a BufferView with matching
			// width, but as long as we have only one BufferView
			// deleting all gives the same result.
			if (lyxerr.debugging())
				textcache.show(lyxerr, "Expose delete all");
			textcache.clear();
		} else if (heightChange) {
			// Rebuild image of current screen
			updateScreen();
			// fitCursor() ensures we don't jump back
			// to the start of the document on vertical
			// resize
			fitCursor();

			// The main window size has changed, repaint most stuff
			redraw();
			// ...including the minibuffer
			owner_->getMiniBuffer()->Init();

		} else if (screen) screen->Redraw();
	} else {
		// Grey box when we don't have a buffer
		fl_winset(FL_ObjWin(work_area));
		fl_rectangle(1, work_area->x, work_area->y,
			     work_area->w, work_area->h, FL_GRAY63);
	}

	// always make sure that the scrollbar is sane.
	updateScrollbar();
	owner_->updateLayoutChoice();
	return 1;
}
#endif


// Callback for cursor timer
void BufferView::CursorToggleCB(FL_OBJECT * ob, long)
{
	BufferView * view = static_cast<BufferView*>(ob->u_vdata);
	
	// Quite a nice place for asyncron Inset updating, isn't it?
	// Actually no! This is run even if no buffer exist... so (Lgb)
	if (view && !view->buffer_) {
		goto set_timer_and_return;
	}

	// NOTE:
	// On my quest to solve the gs render hangups I am now
	// disabling the SIGHUP completely, and will do a wait
	// now and then instead. If the guess that xforms somehow
	// destroys something is true, this is likely (hopefully)
	// to solve the problem...at least I hope so. Lgb

	// ...Ok this seems to work...at least it does not make things
	// worse so far. However I still see gs processes that hangs.
	// I would really like to know _why_ they are hanging. Anyway
	// the solution without the SIGCHLD handler seems to be easier
	// to debug.

	// When attaching gdb to a a running gs that hangs it shows
	// that it is waiting for input(?) Is it possible for us to
	// provide that input somehow? Or figure what it is expecing
	// to read?

	// One solution is to, after some time, look if there are some
	// old gs processes still running and if there are: kill them
	// and re render.

	// Another solution is to provide the user an option to rerender
	// a picture. This would, for the picture in question, check if
	// there is a gs running for it, if so kill it, and start a new
	// rendering process.

	// these comments posted to lyx@via
	{
		int status = 1;
		int pid = waitpid(static_cast<pid_t>(0), &status, WNOHANG);
		if (pid == -1) // error find out what is wrong
			; // ignore it for now.
		else if (pid > 0)
			sigchldhandler(pid, &status);
	}
	if (InsetUpdateList) 
		UpdateInsetUpdateList();

	if (view && !view->screen){
		goto set_timer_and_return;
	}

	if (view->lyx_focus && view->work_area_focus) {
		if (!view->the_locking_inset) {
			view->screen->CursorToggle();
		} else {
			view->the_locking_inset->
				ToggleInsetCursor();
		}
		goto set_timer_and_return;
	} else {
		// Make sure that the cursor is visible.
		if (!view->the_locking_inset) {
			view->screen->ShowCursor();
		} else {
			if (!view->the_locking_inset->isCursorVisible())
				view->the_locking_inset->
					ToggleInsetCursor();
		}
		// This is only run when work_area_focus or lyx_focus is false.
		Window tmpwin;
		int tmp;
		XGetInputFocus(fl_display, &tmpwin, &tmp);
		// Commenting this out, we have not had problems with this
		// for a long time. We will probably work on this code later
		// and we can reenable this debug code then. Now it only
		// anoying when debugging. (Lgb)
		//if (lyxerr.debugging(Debug::INFO)) {
		//	lyxerr << "tmpwin: " << tmpwin
		//	       << "\nwindow: " << view->owner_->getForm()->window
		//	       << "\nwork_area_focus: " << view->work_area_focus
		//	       << "\nlyx_focus      : " << view->lyx_focus
		//	       << endl;
		//}
		if (tmpwin != view->owner_->getForm()->window) {
			view->lyx_focus = false;
			goto skip_timer;
		} else {
			view->lyx_focus = true;
			if (!view->work_area_focus)
				goto skip_timer;
			else
				goto set_timer_and_return;
		}
	}

  set_timer_and_return:
	fl_set_timer(ob, 0.4);
  skip_timer:
	return;
}


#ifdef NEW_WA
static
string fromClipboard(Window win, XEvent * event)
{
	string strret;
	if (event->xselection.type == XA_STRING
	    && event->xselection.property) {
		Atom tmpatom;
		unsigned long ul1;
		unsigned long ul2;
		unsigned char * uc = 0;
		int tmpint;
		if (XGetWindowProperty(
			event->xselection.display,  // display
			win,                        // w
			event->xselection.property, // property
			0,                          // long_offset	
			0,                          // logn_length	
			False,                      // delete	
			XA_STRING,                  // req_type	
			&tmpatom,                   // actual_type_return
			&tmpint,                    // actual_format_return
			&ul1,
			&ul2,
			&uc                         // prop_return	
			) != Success) {
			return strret;
		}
		if (uc) {
			free(uc);
			uc = 0;
		}
		if (XGetWindowProperty(
			event->xselection.display,             // display
			win,                        // w
			event->xselection.property, // property
			0,                          // long_offset
			ul2/4+1,                    // long_length
			True,                       // delete
			XA_STRING,                  // req_type
			&tmpatom,                   // actual_type_return
			&tmpint,                    // actual_format_return
			&ul1,                       // nitems_return
			&ul2,                       // bytes_after_return
			&uc                         // prop_return */
			) != Success) {
			return strret;
		}
		if (uc) {
			strret = reinterpret_cast<char*>(uc);
			free(uc); // yes free!
			uc = 0;
		}
	}
	return strret;
}


void BufferView::WorkAreaSelectionNotify(Window win, XEvent * event)
{
	if (buffer_ == 0) return;

	screen->HideCursor();
	beforeChange();
	string clb = fromClipboard(win, event);
	if (!clb.empty()) {
		if (!ascii_type)
			text->InsertStringA(clb.c_str());
		else
			text->InsertStringB(clb.c_str());

		update(1);
		
	}
}

#else
int BufferView::WorkAreaSelectionNotify(FL_OBJECT *, Window win,
					int /*w*/, int /*h*/,
					XEvent * event, void */*d*/)
{
	if (buffer_ == 0) return 0;
	if (event->type != SelectionNotify)
		return 0;

	Atom tmpatom;
	unsigned long ul1;
	unsigned long ul2;
	unsigned char * uc = 0;
	int tmpint;
	screen->HideCursor();
	beforeChange();
	if (event->xselection.type == XA_STRING
	    && event->xselection.property) {
    
		if (XGetWindowProperty(
			fl_display            /* display */,
			win /* w */,
			event->xselection.property        /* property */,
			0                /* long_offset */,
			0                /* long_length */,
			false                /* delete */,
			XA_STRING                /* req_type */,
			&tmpatom               /* actual_type_return */,
			&tmpint                /* actual_format_return */,
			&ul1      /* nitems_return */,
			&ul2      /* bytes_after_return */,
			&uc     /* prop_return */
			) != Success) {
			return 0;
		}
		XFlush(fl_display);

		if (uc) {
			free(uc);
			uc = 0;
		}

		if (XGetWindowProperty(
			fl_display           /* display */,
			win              /* w */,
			event->xselection.property           /* property */,
			0                /* long_offset */,
			ul2/4+1                /* long_length */,
			True                /* delete */,
			XA_STRING                /* req_type */,
			&tmpatom               /* actual_type_return */,
			&tmpint                /* actual_format_return */,
			&ul1      /* nitems_return */,
			&ul2      /* bytes_after_return */,
			&uc     /* prop_return */
			) != Success) {
			return 0;
		}
		XFlush(fl_display);
        
		if (uc) {
			if (!ascii_type) {
				text->InsertStringA(reinterpret_cast<char*>(uc));
			} else {
				text->InsertStringB(reinterpret_cast<char*>(uc));
			}
			free(uc);
			uc = 0;
		}

		update(1);
	}
	return 0;
}
#endif


#ifdef NEW_WA
void BufferView::cursorPrevious()
{
	if (!text->cursor.row->previous) return;
	
	long y = getScreen()->first;
	Row * cursorrow = text->cursor.row;
	text->SetCursorFromCoordinates(text->cursor.x_fix, y);
	text->FinishUndo();
	// This is to allow jumping over large insets
	if ((cursorrow == text->cursor.row))
		text->CursorUp();
	
  	if (text->cursor.row->height < workarea->height())
		getScreen()->Draw(text->cursor.y
				  - text->cursor.row->baseline
				  + text->cursor.row->height
				  - workarea->height() + 1 );
}
#else
void BufferView::cursorPrevious()
{
	if (!text->cursor.row->previous) return;
	
	long y = getScreen()->first;
	Row * cursorrow = text->cursor.row;
	text->SetCursorFromCoordinates(text->cursor.x_fix, y);
	text->FinishUndo();
	// This is to allow jumping over large insets
	if ((cursorrow == text->cursor.row))
		text->CursorUp();
	
  	if (text->cursor.row->height < work_area->h)
		getScreen()->Draw(text->cursor.y
				  - text->cursor.row->baseline
				  + text->cursor.row->height
				  - work_area->h +1 );
}
#endif


#ifdef NEW_WA
void BufferView::cursorNext()
{
	if (!text->cursor.row->next) return;
	
	long y = getScreen()->first;
	text->GetRowNearY(y);
	Row * cursorrow = text->cursor.row;
	text->SetCursorFromCoordinates(text->cursor.x_fix, y
				       + workarea->height());
	text->FinishUndo();
	// This is to allow jumping over large insets
	if ((cursorrow == text->cursor.row))
		text->CursorDown();
	
 	if (text->cursor.row->height < workarea->height())
		getScreen()->Draw(text->cursor.y
				  - text->cursor.row->baseline);
}
#else
void BufferView::cursorNext()
{
	if (!text->cursor.row->next) return;
	
	long y = getScreen()->first;
	text->GetRowNearY(y);
	Row * cursorrow = text->cursor.row;
	text->SetCursorFromCoordinates(text->cursor.x_fix, y + work_area->h);
	text->FinishUndo();
	// This is to allow jumping over large insets
	if ((cursorrow == text->cursor.row))
		text->CursorDown();
	
 	if (text->cursor.row->height < work_area->h)
		getScreen()->Draw(text->cursor.y
				  - text->cursor.row->baseline);
}
#endif


bool BufferView::available() const
{
	if (buffer_ && text) return true;
	return false;
}


void BufferView::beforeChange()
{
	getScreen()->ToggleSelection();
	text->ClearSelection();
	FreeUpdateTimer();
}


void BufferView::savePosition()
{
	backstack.push(buffer()->fileName(),
		       text->cursor.x,
		       text->cursor.y);
}


void BufferView::restorePosition()
{
	if (backstack.empty()) return;
	
	int  x, y;
	string fname = backstack.pop(&x, &y);
	
	beforeChange();
	Buffer * b = bufferlist.exists(fname) ?
		bufferlist.getBuffer(fname) :
		bufferlist.loadLyXFile(fname); // don't ask, just load it
	buffer(b);
	text->SetCursorFromCoordinates(x, y);
	update(0);
} 


void BufferView::update(signed char f)
{
	owner()->updateLayoutChoice();

	if (!text->selection && f > -3)
		text->sel_cursor = text->cursor;
	
	FreeUpdateTimer();
	text->FullRebreak();

	update();

	if (f != 3 && f != -3) {
		fitCursor();
		updateScrollbar();
      	}

	if (f == 1 || f == -1) {
		if (buffer()->isLyxClean()) {
			buffer()->markDirty();
			owner()->getMiniBuffer()->setTimer(4);
		} else {
			buffer()->markDirty();
		}
	}
}


void BufferView::smallUpdate(signed char f)
{
	getScreen()->SmallUpdate();
	if (getScreen()->TopCursorVisible()
	    != getScreen()->first) {
		update(f);
		return;
	}

	fitCursor();
	updateScrollbar();

	if (!text->selection)
		text->sel_cursor = text->cursor;

	if (f == 1 || f == -1) {
		if (buffer()->isLyxClean()) {
			buffer()->markDirty();
			owner()->getMiniBuffer()->setTimer(4);
		} else {
			buffer()->markDirty();
		}
	}
}


void BufferView::SetState()
{
	if (!lyxrc->rtl_support)
		return;
	
	if (text->real_current_font.getFontDirection()
	    == LYX_DIR_LEFT_TO_RIGHT) {
		if (!owner_->getIntl()->primarykeymap)
			owner_->getIntl()->KeyMapPrim();
	} else {
		if (owner_->getIntl()->primarykeymap)
			owner_->getIntl()->KeyMapSec();
	}
}


void BufferView::insetSleep()
{
	if (the_locking_inset && !inset_slept) {
		the_locking_inset->GetCursorPos(slx, sly);
		the_locking_inset->InsetUnlock();
		inset_slept = true;
	}
}


void BufferView::insetWakeup()
{
	if (the_locking_inset && inset_slept) {
		the_locking_inset->Edit(slx, sly);
		inset_slept = false;
	}
}


void BufferView::insetUnlock()
{
	if (the_locking_inset) {
		if (!inset_slept) the_locking_inset->InsetUnlock();
		the_locking_inset = 0;
		text->FinishUndo();
		inset_slept = false;
	}
}


bool BufferView::focus() const
{
	return workarea->hasFocus();
}


void BufferView::focus(bool f)
{
	if (f) workarea->setFocus();
}


bool BufferView::active() const
{
	return workarea->active();
}
