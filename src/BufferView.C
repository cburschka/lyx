// -*- C++ -*-
/* This file is part of
* ======================================================
* 
*           LyX, The Document Processor
*        
*           Copyright (C) 1995 Matthias Ettrich
*           Copyright (C) 1995-1998 The LyX Team.
*
*======================================================*/

#include <config.h>

#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

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
#include "up.xpm"
#include "down.xpm"
#include "error.h"
#include "lyxdraw.h"
#include "lyx_gui_misc.h"
#include "BackStack.h"
#include "lyxtext.h"
#include "lyx_cb.h"
#include "gettext.h"

// 	$Id: BufferView.C,v 1.1 1999/09/27 18:44:36 larsbj Exp $	

#if !defined(lint) && !defined(WITH_WARNINGS)
static char vcid[] = "$Id: BufferView.C,v 1.1 1999/09/27 18:44:36 larsbj Exp $";
#endif /* lint */

extern BufferList bufferlist;
void sigchldhandler(pid_t pid, int *status);

extern void SetXtermCursor(Window win);
extern bool input_prohibited;
extern bool selection_possible;
extern void BeforeChange();
extern char ascii_type;
extern int UnlockInset(UpdatableInset* inset);
extern void ToggleFloat();
extern void MenuPasteSelection(char at);
extern InsetUpdateStruct *InsetUpdateList;
extern void UpdateInsetUpdateList();

// This is _very_ temporary
FL_OBJECT *figinset_canvas;

BufferView::BufferView(LyXView *o, int xpos, int ypos,
		       int width, int height)
	: _owner(o)
{
	_buffer = 0;
	
	screen = 0;
	work_area = 0;
	figinset_canvas = 0;
	scrollbar = 0;
	button_down = 0;
	button_up = 0;
	timer_cursor = 0;
	current_scrollbar_value = 0;
	create_view(xpos, ypos, width, height);
	// Activate the timer for the cursor 
	fl_set_timer(timer_cursor, 0.4);
	fl_set_focus_object(_owner->getForm(), work_area);
	work_area_focus = true;
	lyx_focus = false;
        backstack = new BackStack(16);
}


BufferView::~BufferView()
{
	delete backstack;
}   


void BufferView::setBuffer(Buffer *b)
{
	lyxerr.debug("Setting buffer in BufferView");
	if (_buffer) {
		_buffer->InsetSleep();
		_buffer->delUser(this);
	}

	// Set current buffer
	_buffer = b;

	if (bufferlist.getState() == BufferList::CLOSING) return;
	
	// Nuke old image
	if (screen)
		delete screen;
	screen = 0;

	// If we are closing the buffer, use the first buffer as current
	if (!_buffer) {
		_buffer = bufferlist.first();
	}

	if (_buffer) {
		lyxerr.debug(LString("  Buffer addr: ") + PTR_AS_INT(_buffer));
		_buffer->addUser(this);
		_owner->getMenus()->showMenus();
		// If we don't have a text object for this, we make one
		if (_buffer->text == 0)
			resizeCurrentBuffer();
		else {
			updateScreen();
			updateScrollbar();
		}
		screen->first = screen->TopCursorVisible();
		redraw();
		updateAllVisibleBufferRelatedPopups();
	        _buffer->InsetWakeup();
	} else {
		lyxerr.debug("  No Buffer!");
		_owner->getMenus()->hideMenus();
		//workAreaExpose();
		updateScrollbar();
		fl_redraw_object(work_area);
	}
	// should update layoutchoice even if we don't have a buffer.
	_owner->updateLayoutChoice();
	_owner->getMiniBuffer()->Init();
	_owner->updateWindowTitle();
}


void BufferView::updateScreen()
{
	// Regenerate the screen.
	if (screen)
		delete screen;
	screen = new LyXScreen(FL_ObjWin(work_area),
			       work_area->w,
			       work_area->h,
			       work_area->x,
			       work_area->y,
			       _buffer->text);
}


void BufferView::resize()
{
	// This will resize the buffer. (Asger)
	if (_buffer)
		resizeCurrentBuffer();
}


static bool lgb_hack = false;

void BufferView::redraw()
{
	lyxerr.debug("BufferView::redraw()");
	lgb_hack = true;
	fl_redraw_object(work_area);
	fl_redraw_object(scrollbar);
	fl_redraw_object(button_down);
	fl_redraw_object(button_up);
	lgb_hack = false;
}


void BufferView::fitCursor()
{
	if (screen) screen->FitCursor();
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

	if (!_buffer) {
		fl_set_slider_value(scrollbar, 0);
		fl_set_slider_size(scrollbar, scrollbar->h);
		return;
	}
	
	static long max2 = 0;
	static long height2 = 0;

	long cbth = 0;
	long cbsf = 0;

	if (_buffer->text)
		cbth = _buffer->text->height;
	if (screen)
		cbsf = screen->first;

	// check if anything has changed.
	if (max2 == cbth &&
	    height2 == work_area->h &&
	    current_scrollbar_value == cbsf)
		return;	      // no
	
	max2 = cbth;
	height2 = work_area->h;
	current_scrollbar_value = cbsf;

	if (cbth <= height2) { // text is smaller than screen
		fl_set_slider_size(scrollbar, scrollbar->h);
		return;
	}
	
	long maximum_height = work_area->h * 3/4 + cbth;
	long value = cbsf;

	/* set the scrollbar */
	double hfloat = work_area->h;
	double maxfloat = maximum_height;
   
	fl_set_slider_value(scrollbar, value);
	fl_set_slider_bounds(scrollbar, 0,
			     maximum_height - work_area->h);
#if FL_REVISION > 85
	double lineh = _buffer->text->DefaultHeight();
	fl_set_slider_increment(scrollbar,work_area->h-lineh,lineh);
#endif
	if (maxfloat>0){
		if ((hfloat/maxfloat) * (float) height2 < 3)
			fl_set_slider_size(scrollbar,
					   3/(float)height2);
		else
			fl_set_slider_size(scrollbar,
					   hfloat/maxfloat);
	} else
		fl_set_slider_size(scrollbar, hfloat);
	fl_set_slider_precision(scrollbar, 0);
}


void BufferView::redoCurrentBuffer()
{
	lyxerr.debug("BufferView::redoCurrentBuffer");
	if (_buffer && _buffer->text) {
		resize();
		_owner->updateLayoutChoice();
	}
}


int BufferView::resizeCurrentBuffer()
{
	lyxerr.debug("resizeCurrentBuffer");
	
	LyXParagraph *par = 0;
	LyXParagraph *selstartpar = 0;
	LyXParagraph *selendpar = 0;
	int pos = 0;
	int selstartpos = 0;
	int selendpos = 0;
	int selection = 0;
	int mark_set = 0;

	ProhibitInput();

	_owner->getMiniBuffer()->Set(_("Formatting document..."));   

	if (_buffer->text) {
		par = _buffer->text->cursor.par;
		pos = _buffer->text->cursor.pos;
		selstartpar = _buffer->text->sel_start_cursor.par;
		selstartpos = _buffer->text->sel_start_cursor.pos;
		selendpar = _buffer->text->sel_end_cursor.par;
		selendpos = _buffer->text->sel_end_cursor.pos;
		selection = _buffer->text->selection;
		mark_set = _buffer->text->mark_set;
		delete _buffer->text;
	}
	_buffer->text = new LyXText(work_area->w, _buffer);

	updateScreen();
   
	if (par) {
		_buffer->text->selection = true;
		/* at this point just
		 * to avoid the Delete-
		 * Empty-Paragraph
		 * Mechanism when
		 * setting the cursor */
		_buffer->text->mark_set = mark_set;
		if (selection) {
			_buffer->text->SetCursor(selstartpar, selstartpos);
			_buffer->text->sel_cursor = _buffer->text->cursor;
			_buffer->text->SetCursor(selendpar, selendpos);
			_buffer->text->SetSelection();
			_buffer->text->SetCursor(par, pos);
		} else {
			_buffer->text->SetCursor(par, pos);
			_buffer->text->sel_cursor = _buffer->text->cursor;
			_buffer->text->selection = false;
		}
	}
	screen->first = screen->TopCursorVisible(); /* this will scroll the
						     * screen such that the
						     * cursor becomes
						     * visible */ 
	updateScrollbar();
	redraw();
	_owner->getMiniBuffer()->Init();
	AllowInput();

	// Now if the title form still exist kill it
	TimerCB(0,0);

	return 0;
}


void BufferView::gotoError()
{
	if (!screen)
		return;
   
	screen->HideCursor();
	BeforeChange();
	_buffer->update(-2);
	LyXCursor tmp;
   
	if (!_buffer->text->GotoNextError()) {
		if (_buffer->text->cursor.pos 
		    || _buffer->text->cursor.par !=
		    _buffer->text->FirstParagraph()) {
			tmp = _buffer->text->cursor;
			_buffer->text->cursor.par =
				_buffer->text->FirstParagraph();
			_buffer->text->cursor.pos = 0;
			if (!_buffer->text->GotoNextError()) {
				_buffer->text->cursor = tmp;
				_owner->getMiniBuffer()->Set(_("No more errors"));
				LyXBell();
			}
		} else {
			_owner->getMiniBuffer()->Set(_("No more errors"));
			LyXBell();
		}
	}
	_buffer->update(0);
	_buffer->text->sel_cursor =
		_buffer->text->cursor;
}


void BufferView::create_view(int xpos, int ypos, int width, int height)
{
	FL_OBJECT *obj;
	const int bw = abs(fl_get_border_width());

	// a hack for the figinsets (Matthias)
	// This one first, then it will probably be invisible. (Lgb)
	::figinset_canvas = figinset_canvas = obj =
		fl_add_canvas(FL_NORMAL_CANVAS,
			      xpos + 1,
			      ypos + 1,1,1,"");
	fl_set_object_boxtype(obj,FL_NO_BOX);
	fl_set_object_resize(obj, FL_RESIZE_ALL);
	fl_set_object_gravity(obj, NorthWestGravity, NorthWestGravity);

	// a box
	obj = fl_add_box(FL_BORDER_BOX, xpos, ypos,
			 width - 15,
			 height,"");
	fl_set_object_resize(obj, FL_RESIZE_ALL);
	fl_set_object_gravity(obj, NorthWestGravity, SouthEastGravity);

	// the free object
	work_area = obj = fl_add_free(FL_INPUT_FREE,
				      xpos +bw, ypos+bw,
				      width-15-2*bw /* scrollbarwidth */,
				      height-2*bw,"",
				      work_area_handler);
	obj->wantkey = FL_KEY_TAB;
	obj->u_vdata = (void*) this; /* This is how we pass the BufferView
				       to the work_area_handler. */
	fl_set_object_boxtype(obj,FL_DOWN_BOX);
	fl_set_object_resize(obj, FL_RESIZE_ALL);
	fl_set_object_gravity(obj, NorthWestGravity, SouthEastGravity);

	//
	// THE SCROLLBAR
	//

	// up - scrollbar button
#if FL_REVISION > 85
	fl_set_border_width(-1);
#else
	fl_set_border_width(-2); // to get visible feedback
#endif
	button_up = obj = fl_add_pixmapbutton(FL_TOUCH_BUTTON,
					      width-15+4*bw,
					      ypos,
					      15,15,"");
	fl_set_object_boxtype(obj,FL_UP_BOX);
	fl_set_object_color(obj,FL_MCOL,FL_BLUE);
	fl_set_object_resize(obj, FL_RESIZE_ALL);
	fl_set_object_gravity(obj,NorthEastGravity, NorthEastGravity);
	fl_set_object_callback(obj,UpCB,(long)this);
	fl_set_pixmapbutton_data(obj, up_xpm);

#if FL_REVISION >85
	// Remove the blue feedback rectangle
	fl_set_pixmapbutton_focus_outline(obj,0);
#endif	

	// the scrollbar slider
	fl_set_border_width(-bw);
	scrollbar = obj = fl_add_slider(FL_VERT_SLIDER,
					width-15+4*bw,
					ypos + 15,
					15,height-30,"");
	fl_set_object_color(obj,FL_COL1,FL_MCOL);
	fl_set_object_boxtype(obj, FL_UP_BOX);
	fl_set_object_resize(obj, FL_RESIZE_ALL);
	fl_set_object_gravity(obj, NorthEastGravity, SouthEastGravity);
	fl_set_object_callback(obj,ScrollCB,(long)this);
	
	// down - scrollbar button
#if FL_REVISION > 85
	fl_set_border_width(-1);
#else
	fl_set_border_width(-2); // to get visible feedback
#endif
	button_down = obj = fl_add_pixmapbutton(FL_TOUCH_BUTTON,
						      width-15+4*bw,
						      ypos + height-15,
						      15,15,"");
	fl_set_object_boxtype(obj,FL_UP_BOX);
	fl_set_object_color(obj,FL_MCOL,FL_BLUE);
	fl_set_object_resize(obj, FL_RESIZE_ALL);
	fl_set_object_gravity(obj, SouthEastGravity, SouthEastGravity);
	fl_set_object_callback(obj,DownCB,(long)this);
	fl_set_pixmapbutton_data(obj, down_xpm);
	fl_set_border_width(-bw);

#if FL_REVISION >85
	// Remove the blue feedback rectangle
	fl_set_pixmapbutton_focus_outline(obj,0);
#endif	

	//
	// TIMERS
	//
	
	// timer_cursor
	timer_cursor = obj = fl_add_timer(FL_HIDDEN_TIMER,
					  0,0,0,0,"Timer");
	fl_set_object_callback(obj,CursorToggleCB,0);
	obj->u_vdata = (void*) this;
}


// Callback for scrollbar up button
void BufferView::UpCB(FL_OBJECT *ob, long buf)
{
	BufferView *view = (BufferView*) buf;
	
	if (view->_buffer == 0) return;

	const XEvent*ev2;
	static long time = 0;
	ev2 = fl_last_event();
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


static
void waitForX()
{
	static Window w = 0;
	static Atom a = 0;
	if (!a)
		a = XInternAtom(fl_display, "WAIT_FOR_X", False);
	if (w == 0) {
		int mask;
		XSetWindowAttributes attr;
		mask = CWOverrideRedirect;
		attr.override_redirect = 1;
		w = XCreateWindow(fl_display, fl_root,
				  0, 0, 1, 1, 0, CopyFromParent,
				  InputOnly, CopyFromParent, mask, &attr);
		XSelectInput(fl_display, w, PropertyChangeMask);
		XMapWindow(fl_display, w);
	}
	static XEvent ev;
	XChangeProperty(fl_display, w, a, a, 8,
			PropModeAppend, (unsigned char *)"", 0);
	XWindowEvent(fl_display, w, PropertyChangeMask, &ev);
}


// Callback for scrollbar slider
void BufferView::ScrollCB(FL_OBJECT *ob, long buf)
{
	BufferView *view = (BufferView*) buf;
	extern bool cursor_follows_scrollbar;
	
	if (view->_buffer == 0) return;

	view->current_scrollbar_value = (long)fl_get_slider_value(ob);
	if (view->current_scrollbar_value < 0)
		view->current_scrollbar_value = 0;
   
	if (!view->screen)
		return;

	view->screen->Draw(view->current_scrollbar_value);

	if (cursor_follows_scrollbar) {
		LyXText * vbt = view->_buffer->text;
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


// Callback for scrollbar down button
void BufferView::DownCB(FL_OBJECT *ob, long buf)
{
	BufferView *view = (BufferView*) buf;

	if (view->_buffer == 0) return;
	
	const XEvent*ev2;
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


int BufferView::ScrollUp(long time)
{
	if (_buffer == 0) return 0;
	if (!screen)
		return 0;
   
	double value= fl_get_slider_value(scrollbar);
   
	if (value == 0)
		return 0;
   
	float add_value =  (_buffer->text->DefaultHeight()
			    + (float)(time) * (float)(time) * 0.125);
   
	if (add_value > work_area->h)
		add_value = (float) (work_area->h -
				     _buffer->text->DefaultHeight());
   
	value -= add_value;

	if (value < 0)
		value = 0;
   
	fl_set_slider_value(scrollbar, value);
   
	ScrollCB(scrollbar,(long)this); 
	return 0;
}


int BufferView::ScrollDown(long time)
{
	if (_buffer == 0) return 0;
	if (!screen)
		return 0;
   
	double value= fl_get_slider_value(scrollbar);
	double min, max;
	fl_get_slider_bounds(scrollbar, &min, &max);

	if (value == max)
		return 0;
   
	float add_value =  (_buffer->text->DefaultHeight()
			    + (float)(time) * (float)(time) * 0.125);
   
	if (add_value > work_area->h)
		add_value = (float) (work_area->h -
				     _buffer->text->DefaultHeight());
   
	value += add_value;
   
	if (value > max)
		value = max;
   
	fl_set_slider_value(scrollbar, value);
   
	ScrollCB(scrollbar,(long)this); 
	return 0;
}


void BufferView::ScrollUpOnePage(long /*time*/)
{
	if (_buffer == 0) return;
	if (!screen)
		return;
   
	long y = screen->first;

	if (!y) return;
   
	Row* row = _buffer->text->GetRowNearY(y);
	y = y - work_area->h + row->height;
	
	fl_set_slider_value(scrollbar, y);
   
	ScrollCB(scrollbar,(long)this); 
}


void BufferView::ScrollDownOnePage(long /*time*/)
{
	if (_buffer == 0) return;
	if (!screen)
		return;
   
	double min, max;
	fl_get_slider_bounds(scrollbar, &min, &max);
	long y = screen->first;
   
	if (y > _buffer->text->height - work_area->h)
		return;
   
	y += work_area->h;
	_buffer->text->GetRowNearY(y);
	
	fl_set_slider_value(scrollbar, y);
   
	ScrollCB(scrollbar,(long)this); 
}


int BufferView::work_area_handler(FL_OBJECT * ob, int event,
				  FL_Coord, FL_Coord ,
				  int /*key*/, void *xev)
{
	static int x_old = -1;
	static int y_old = -1;
	static long scrollbar_value_old = -1;
	
	XEvent* ev = (XEvent*) xev;
	BufferView *view = (BufferView*) ob->u_vdata;

	// If we don't have a view yet; return
	if (!view || quitting) return 0;

	switch (event){   
	case FL_DRAW:
		view->workAreaExpose(); 
		break;
	case FL_PUSH:
		view->WorkAreaButtonPress(ob, 0,0,0,ev,0);
		break; 
	case FL_RELEASE:
		view->WorkAreaButtonRelease(ob, 0,0,0,ev,0);
		break;
	case FL_MOUSE:
		if (ev->xmotion.x != x_old || 
		    ev->xmotion.y != y_old ||
		    view->current_scrollbar_value != scrollbar_value_old) {
			x_old = ev->xmotion.x;
			y_old = ev->xmotion.y;
			scrollbar_value_old = view->current_scrollbar_value;
			view->WorkAreaMotionNotify(ob, 0,0,0,ev,0);
		}
		break;
	// Done by the raw callback:
	//  case FL_KEYBOARD: WorkAreaKeyPress(ob, 0,0,0,ev,0); break;
	case FL_FOCUS:
		if (!view->_owner->getMiniBuffer()->shows_no_match)
			view->_owner->getMiniBuffer()->Init();
		view->_owner->getMiniBuffer()->shows_no_match = false;
		view->work_area_focus = true;
		fl_set_timer(view->timer_cursor, 0.4);
		break;
	case FL_UNFOCUS:
		view->_owner->getMiniBuffer()->ExecCommand();
		view->work_area_focus = false;
		break;
	case FL_ENTER:
		SetXtermCursor(view->_owner->getForm()->window);
		// reset the timer
		view->lyx_focus = true;
		fl_set_timer(view->timer_cursor, 0.4);
		break;
	case FL_LEAVE: 
		if (!input_prohibited)
			XUndefineCursor(fl_display,
					view->_owner->getForm()->window);
		view->lyx_focus = false; // This is not an absolute truth
		// but if it is not true, it will be changed within a blink
		// of an eye. ... Not good enough... use regulare timeperiod
		//fl_set_timer(view->timer_cursor, 0.01); // 0.1 sec blink
		fl_set_timer(view->timer_cursor, 0.4); // 0.4 sec blink
		break;
	case FL_DBLCLICK: 
		// select a word 
		if (view->_buffer && !view->_buffer->the_locking_inset) {
			if (view->screen && ev->xbutton.button == 1) {
				view->screen->HideCursor();
				view->screen->ToggleSelection(); 
				view->_buffer->text->SelectWord();
				view->screen->ToggleSelection(false);
				/* This will fit the cursor on the screen
				 * if necessary */ 
				view->_buffer->update(0); 
			}
		}
		break;
	case FL_TRPLCLICK:
		// select a line
		if (view->_buffer && view->screen && ev->xbutton.button == 1) {
			view->screen->HideCursor(); 
			view->screen->ToggleSelection(); 
			view->_buffer->text->CursorHome();
			view->_buffer->text->sel_cursor =
				view->_buffer->text->cursor;
			view->_buffer->text->CursorEnd();
			view->_buffer->text->SetSelection();
			view->screen->ToggleSelection(false); 
			/* This will fit the cursor on the screen
			 * if necessary */ 
			view->_buffer->update(0); 
		}
		break;
	case FL_OTHER:
		view->WorkAreaSelectionNotify(ob,
					      view->_owner->getForm()->window,
					      0,0,ev,0); 
		break;
	}
	return 1;
}

int BufferView::WorkAreaMotionNotify(FL_OBJECT *ob, Window,
				     int /*w*/, int /*h*/,
				     XEvent *ev, void */*d*/)
{

	if (_buffer == 0) return 0;
	if (!screen) return 0;

	// Check for inset locking
	if (_buffer->the_locking_inset) {
		LyXCursor cursor = _buffer->text->cursor;
		_buffer->the_locking_inset->
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

		_buffer->text->
			SetCursorFromCoordinates(ev->xbutton.x - ob->x,
						 ev->xbutton.y - ob->y +
						 screen->first);
      
		if (!_buffer->text->selection)
		    _buffer->update(-3); // Maybe an empty line was deleted
      
		_buffer->text->SetSelection();
		screen->ToggleToggle();
		if (screen->FitCursor())
			updateScrollbar(); 
		screen->ShowCursor();
	}
	return 0;
}


extern int bibitemMaxWidth(const class LyXFont &);

// Single-click on work area
int BufferView::WorkAreaButtonPress(FL_OBJECT *ob, Window,
			int /*w*/, int /*h*/, XEvent *ev, void */*d*/)
{
	last_click_x = -1;
	last_click_y = -1;

	if (_buffer == 0) return 0;
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
		
	if (_buffer->the_locking_inset) {
		// We are in inset locking mode
		
		/* Check whether the inset was hit. If not reset mode,
		   otherwise give the event to the inset */
		if (inset_hit != 0) {
			_buffer->the_locking_inset->
				InsetButtonPress(inset_x, inset_y, button);
			return 0;
		} else {
			UnlockInset(_buffer->the_locking_inset);
		}
	}
	
	selection_possible = true;
	screen->HideCursor();
	
	// Right button mouse click on a table
	if (button == 3 &&
	    (_buffer->text->cursor.par->table ||
	     _buffer->text->MouseHitInTable(x, y+screen->first))) {
		// Set the cursor to the press-position
		_buffer->text->SetCursorFromCoordinates(x, y + screen->first);
		bool doit = true;
		
		// Only show the table popup if the hit is in the table, too
		if (!_buffer->text->HitInTable(_buffer->text->cursor.row, x))
			doit = false;
		
		// Hit above or below the table?
		if (doit) {
			long y_tmp = y + screen->first;
			Row*  row =  _buffer->text->GetRowNearY(y_tmp);
#if 0
			// Isn't this empty code anyway? (Lgb)
			if (row->par != _buffer->text->cursor.par)
				doit = true;
		}
		
		if (doit) {
#endif
			if (!_buffer->text->selection) {
				screen->ToggleSelection();
				_buffer->text->ClearSelection();
				_buffer->text->FullRebreak();
				screen->Update();
				updateScrollbar();
			}
			// Popup table popup when on a table.
			// This is obviously temporary, since we should be
		       	// able to 
		       	// popup various context-sensitive-menus with the
		       	// the right mouse. So this should be done more
		       	// general in the future. Matthias.
			selection_possible = false;
			_owner->getLyXFunc()->Dispatch(LFUN_LAYOUT_TABLE,
						       "true");
			return 0;
		}
	}
	
	int screen_first = screen->first;
	
	// Middle button press pastes if we have a selection
	bool paste_internally = false;
	if (button == 2  // && !_buffer->the_locking_inset
	    && _buffer->text->selection) {
		_owner->getLyXFunc()->Dispatch(LFUN_COPY);
		paste_internally = true;
	}
	
	// Clear the selection
	screen->ToggleSelection();
	_buffer->text->ClearSelection();
	_buffer->text->FullRebreak();
	screen->Update();
	updateScrollbar();
		
	// Single left click in math inset?
	if (inset_hit != 0 && inset_hit->Editable() == 2) {
		// Highly editable inset, like math
		selection_possible = false;
		_owner->updateLayoutChoice();
		_owner->getMiniBuffer()->Set(inset_hit->EditMessage());
		inset_hit->Edit(inset_x, inset_y);
		return 0;
	} 

	// Right click on a footnote flag opens float menu
	if (button == 3) { 
#if 0
		// Isn't this empty code anyway? (Lgb)
		if (_buffer->text->cursor.par->footnoteflag == 
		    LyXParagraph::OPEN_FOOTNOTE) {
			selection_possible = false;
			lyxerr.debug("LyX: Sorry not implemented yet.");
			return 0;
		}
	}
	
	if (button == 3) {
#endif
		selection_possible = false;
		return 0;
	}
	
	_buffer->text->SetCursorFromCoordinates(x, y + screen_first);
	_buffer->text->FinishUndo();
	_buffer->text->sel_cursor = _buffer->text->cursor;
	_buffer->text->cursor.x_fix = _buffer->text->cursor.x;
	
	_owner->updateLayoutChoice();
	if (screen->FitCursor()){
		updateScrollbar();
		selection_possible = false;
	}

	// Insert primary selection with middle mouse
	// if there is a local selection in the current buffer, insert this
	if (button == 2) { //  && !_buffer->the_locking_inset){
		if (paste_internally)
			_owner->getLyXFunc()->Dispatch(LFUN_PASTE);
		else
			_owner->getLyXFunc()->Dispatch(LFUN_PASTESELECTION,
						       "paragraph");
		selection_possible = false;
		return 0;
	}
	}
	goto out;
 wheel: {
	// I am not quite sure if this is the correct place to put this,
	// but it will not cause any harm.
	// Patch from Mark Huang (markman@mit.edu) to make LyX recognise
	// button 4 and 5. This enables LyX use use the scrollwhell on
	// certain mice for something useful. (Lgb)
	// Added wheel acceleration detection code. (Rvdk)
	static Time lastTime = 0;
	int diff = ev->xbutton.time - lastTime;
	int scroll = int(1.0 + (4.0/(abs(diff)+1.0))*200.0);
	switch (button) {
	case 5:
		ScrollUp(scroll);
		break;
	case 4:
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


int BufferView::WorkAreaButtonRelease(FL_OBJECT *ob, Window ,
			  int /*w*/, int /*h*/, XEvent *ev, void */*d*/)
{
	if (_buffer == 0 || screen == 0) return 0;

	int const x = ev->xbutton.x - ob->x;
	int const y = ev->xbutton.y - ob->y;

	// If we hit an inset, we have the inset coordinates in these
	// and inset_hit points to the inset.  If we do not hit an
	// inset, inset_hit is 0, and inset_x == x, inset_y == y.
	int inset_x = x;
	int inset_y = y;
	Inset * inset_hit = checkInsetHit(inset_x, inset_y);

	if (_buffer->the_locking_inset) {
		// We are in inset locking mode.

		/* LyX does a kind of work-area grabbing for insets.
		   Only a ButtonPress Event outside the inset will 
		   force a InsetUnlock. */
		_buffer->the_locking_inset->
			InsetButtonRelease(inset_x, inset_y, 
					   ev->xbutton.button);
		return 0;
	}
  
	selection_possible = false;
        if (_buffer->text->cursor.par->table) {
                int cell = _buffer->text->
                        NumberOfCell(_buffer->text->cursor.par,
                                     _buffer->text->cursor.pos);
                if (_buffer->text->cursor.par->table->IsContRow(cell) &&
                    _buffer->text->cursor.par->table->
                    CellHasContRow(_buffer->text->cursor.par->table->
                                   GetCellAbove(cell))<0) {
                        _buffer->text->CursorUp();
                }
        }
	
	if (ev->xbutton.button >= 2)
		return 0;

	// Make sure that the press was not far from the release
	if ((abs(last_click_x - x) >= 5) ||
	    (abs(last_click_y - y) >= 5)) {
		return 0;
	}

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
			_buffer->text->SetCursorParUndo();
		}

		_owner->getMiniBuffer()->Set(inset_hit->EditMessage());
		inset_hit->Edit(inset_x, inset_y);
		return 0;
	}

	// check whether we want to open a float
	if (_buffer->text) {
		bool hit = false;
		char c = ' ';
		if (_buffer->text->cursor.pos <
		    _buffer->text->cursor.par->Last()) {
			c = _buffer->text->cursor.par->
				GetChar(_buffer->text->cursor.pos);
		}
		if (c == LYX_META_FOOTNOTE || c == LYX_META_MARGIN
		    || c == LYX_META_FIG || c == LYX_META_TAB
		    || c == LYX_META_WIDE_FIG || c == LYX_META_WIDE_TAB
                    || c == LYX_META_ALGORITHM){
			hit = true;
		} else if (_buffer->text->cursor.pos - 1 >= 0) {
			c = _buffer->text->cursor.par->
				GetChar(_buffer->text->cursor.pos - 1);
			if (c == LYX_META_FOOTNOTE || c == LYX_META_MARGIN
			    || c == LYX_META_FIG || c == LYX_META_TAB
			    || c == LYX_META_WIDE_FIG 
			    || c == LYX_META_WIDE_TAB
			    || c == LYX_META_ALGORITHM){
				// We are one step too far to the right
				_buffer->text->CursorLeft();
				hit = true;
			}
		}
		if (hit == true) {
			ToggleFloat();
			selection_possible = false;
			return 0;
		}
	}

	// Do we want to close a float? (click on the float-label)
	if (_buffer->text->cursor.row->par->footnoteflag ==
	    LyXParagraph::OPEN_FOOTNOTE
	    && _buffer->text->cursor.pos == 0
	    && _buffer->text->cursor.row->previous &&
	    _buffer->text->cursor.row->previous->par->
	    footnoteflag != LyXParagraph::OPEN_FOOTNOTE){
		LyXFont font (LyXFont::ALL_SANE);
		font.setSize(LyXFont::SIZE_SMALL);

		int box_x = 20; // LYX_PAPER_MARGIN;
		box_x += font.textWidth("Mwide-figM", 10);

		int screen_first = screen->first;

		if (x < box_x
		    && y + screen_first > _buffer->text->cursor.y -
		    _buffer->text->cursor.row->baseline
		    && y + screen_first < _buffer->text->cursor.y -
		    _buffer->text->cursor.row->baseline
		    + font.maxAscent()*1.2 + font.maxDescent()*1.2) {
			ToggleFloat();
			selection_possible = false;
			return 0;
		}
	}

	// Maybe we want to edit a bibitem ale970302
	if (_buffer->text->cursor.par->bibkey && x < 20 + 
	    bibitemMaxWidth(lyxstyle.TextClass(_buffer->
					params.textclass)->defaultfont)) {
		_buffer->text->cursor.par->bibkey->Edit(0, 0);
	}

	return 0;
}


/* 
 * Returns an inset if inset was hit. 0 otherwise.
 * If hit, the coordinates are changed relative to the inset. 
 * Otherwise coordinates are not changed, and false is returned.
 */
Inset * BufferView::checkInsetHit(int &x, int &y)
{
	if (!getScreen())
		return 0;
  
	int y_tmp = y + getScreen()->first;
  
	LyXCursor cursor = _buffer->text->cursor;
	if (cursor.pos < cursor.par->Last() 
	    && cursor.par->GetChar(cursor.pos) == LYX_META_INSET
	    && cursor.par->GetInset(cursor.pos)
	    && cursor.par->GetInset(cursor.pos)->Editable()) {

		// Check whether the inset really was hit
		Inset* tmpinset = cursor.par->GetInset(cursor.pos);
		LyXFont font = _buffer->text->GetFont(cursor.par, cursor.pos);
		if (x > cursor.x
		    && x < cursor.x + tmpinset->Width(font) 
		    && y_tmp > cursor.y - tmpinset->Ascent(font)
		    && y_tmp < cursor.y + tmpinset->Descent(font)) {
			x = x - cursor.x;
			// The origin of an inset is on the baseline
			y = y_tmp - (cursor.y); 
			return tmpinset;
		}
	} else if (cursor.pos - 1 >= 0 
		   && cursor.par->GetChar(cursor.pos - 1) == LYX_META_INSET
		   && cursor.par->GetInset(cursor.pos - 1)
		   && cursor.par->GetInset(cursor.pos - 1)->Editable()) {
		_buffer->text->CursorLeft();
		Inset * result = checkInsetHit(x, y);
		if (result == 0) {
			_buffer->text->CursorRight();
			return 0;
		} else {
			return result;
		}
	}
	return 0;
}


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
	if (_buffer != 0) {
		if (widthChange) {
			// All buffers need a resize
			bufferlist.resize();
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
			_owner->getMiniBuffer()->Init();

		} else if (screen) screen->Redraw();
	} else {
		// Grey box when we don't have a buffer
		fl_winset(FL_ObjWin(work_area));
		fl_rectangle(1, work_area->x, work_area->y,
			     work_area->w, work_area->h, FL_GRAY63);
	}

	// always make sure that the scrollbar is sane.
	updateScrollbar();
	_owner->updateLayoutChoice();
	return 1;
}


// Callback for cursor timer
void BufferView::CursorToggleCB(FL_OBJECT *ob, long)
{
	BufferView *view = (BufferView*) ob->u_vdata;
	
	/* quite a nice place for asyncron Inset updating, isn't it? */
	// actually no! This is run even if no buffer exist... so (Lgb)
	if (view && !view->_buffer) {
		goto set_timer_and_return;
	}
#ifdef WITH_WARNINGS
#warning NOTE!
#endif

	// On my quest to solve the gs rendre hangups I am now
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
//#if 0
	{
	int status = 1;
	int pid = waitpid((pid_t)0, &status, WNOHANG);
	if (pid == -1) // error find out what is wrong
		; // ignore it for now.
	else if (pid > 0)
		sigchldhandler(pid, &status);
	}
//#endif
	if (InsetUpdateList) 
		UpdateInsetUpdateList();

	if (view && !view->screen){
		goto set_timer_and_return;
	}

	if (view->lyx_focus && view->work_area_focus) {
		if (!view->_buffer->the_locking_inset){
			view->screen->CursorToggle();
		} else {
			view->_buffer->the_locking_inset->
				ToggleInsetCursor();
		}
		goto set_timer_and_return;
	} else {
		// Make sure that the cursor is visible.
		if (!view->_buffer->the_locking_inset){
			view->screen->ShowCursor();
		} else {
			if (!view->_buffer->the_locking_inset->isCursorVisible())
				view->_buffer->the_locking_inset->
					ToggleInsetCursor();
		}

		// This is only run when work_area_focus or lyx_focus is false.
		Window tmpwin;
		int tmp;
		XGetInputFocus(fl_display, &tmpwin, &tmp);
		lyxerr.debug(LString("tmpwin: ") + int(tmpwin));
		lyxerr.debug(LString("window: ")
			     + int(view->_owner->getForm()->window));
		lyxerr.debug(LString("work_area_focus: ")
			     + int(view->work_area_focus));
		lyxerr.debug(LString("lyx_focus      : ")
			     + int(view->lyx_focus));
		if (tmpwin != view->_owner->getForm()->window) {
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


int BufferView::WorkAreaSelectionNotify(FL_OBJECT *, Window win,
			    int /*w*/, int /*h*/, XEvent *event, void */*d*/)
{
	if (_buffer == 0) return 0;
	if (event->type != SelectionNotify)
		return 0;

	Atom tmpatom;
	unsigned long ul1;
	unsigned long ul2;
	unsigned char* uc = 0;
	int tmpint;
	screen->HideCursor();
	BeforeChange();
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

		if (uc){
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
        
		if (uc){
			if (!ascii_type)
				_buffer->text->
					InsertStringA((char*)uc);
			else
				_buffer->text->
					InsertStringB((char*)uc);
			free(uc);
			uc = 0;
		}
    
		_buffer->update(1);
	}
	return 0;
}


void BufferView::cursorPrevious()
{
	if (!currentBuffer()->text->cursor.row->previous) return;
	
	long y = getScreen()->first;
	Row* cursorrow = currentBuffer()->text->cursor.row;
	currentBuffer()->text->
	  SetCursorFromCoordinates(currentBuffer()->text->
				   cursor.x_fix,
				   y);
	currentBuffer()->text->FinishUndo();
	/* this is to allow jumping over large insets */
	if ((cursorrow == currentBuffer()->text->cursor.row))
	  currentBuffer()->text->CursorUp();
	
  	if (currentBuffer()->text->cursor.row->height < work_area->h)
	  getScreen()->Draw(currentBuffer()->text->cursor.y
  			    - currentBuffer()->text->cursor.row->baseline
  			    + currentBuffer()->text->cursor.row->height
  			    - work_area->h +1 );
}


void BufferView::cursorNext()
{
	if (!currentBuffer()->text->cursor.row->next) return;
	
	long y = getScreen()->first;
	currentBuffer()->text->GetRowNearY(y);
	Row* cursorrow = currentBuffer()->text->cursor.row;
	currentBuffer()->text->
		SetCursorFromCoordinates(currentBuffer()->text->
					 cursor.x_fix, 
					 y + work_area->h);
	currentBuffer()->text->FinishUndo();
	/* this is to allow jumping over large insets */
	if ((cursorrow == currentBuffer()->text->cursor.row))
	  currentBuffer()->text->CursorDown();
	
 	if (currentBuffer()->text->cursor.row->height < work_area->h)
 	  getScreen()->Draw(currentBuffer()->text->cursor.y
 			    - currentBuffer()->text->cursor.row->baseline);
}


bool BufferView::available() const
{
	if (_buffer && _buffer->text) return true;
	return false;
}


void BufferView::savePosition()
{
	backstack->push(currentBuffer()->getFileName(),
			currentBuffer()->text->cursor.x,
			currentBuffer()->text->cursor.y);
}


void BufferView::restorePosition()
{
	int  x, y;
	LString fname = backstack->pop(&x, &y);
	
	BeforeChange();
	Buffer *b = (bufferlist.exists(fname)) ? bufferlist.getBuffer(fname):
		bufferlist.loadLyXFile(fname); // don't ask, just load it
	setBuffer(b);
	currentBuffer()->text->SetCursorFromCoordinates(x, y);
	currentBuffer()->update(0);
} 

