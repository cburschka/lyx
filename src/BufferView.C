// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-1999 The LyX Team.
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
#include "up.xpm"
#include "down.xpm"
#include "debug.h"
#include "lyxdraw.h"
#include "lyx_gui_misc.h"
#include "BackStack.h"
#include "lyxtext.h"
#include "lyx_cb.h"
#include "gettext.h"
#include "layout.h"

using std::find_if;

extern BufferList bufferlist;
void sigchldhandler(pid_t pid, int * status);

extern void SetXtermCursor(Window win);
extern bool input_prohibited;
extern bool selection_possible;
extern void BeforeChange();
extern char ascii_type;
extern int UnlockInset(UpdatableInset * inset);
extern void ToggleFloat();
extern void MenuPasteSelection(char at);
extern InsetUpdateStruct * InsetUpdateList;
extern void UpdateInsetUpdateList();
extern void FreeUpdateTimer();

// This is _very_ temporary
FL_OBJECT * figinset_canvas;

BufferView::BufferView(LyXView * o, int xpos, int ypos,
		       int width, int height)
	: owner_(o)
{
	buffer_ = 0;
	text = 0;
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
	fl_set_focus_object(owner_->getForm(), work_area);
	work_area_focus = true;
	lyx_focus = false;
}


BufferView::~BufferView()
{
	delete text;
}

// This is only the very first implemetation and use of the TextCache,
// operations on it needs to be put into a class or a namespace, that part
// is _NOT_ finished so don't bother to come with too many comments on it
// (unless you have some nice ideas on where/how to do it)
//
// I think we need a TextCache that is common for all BufferViews,
// please tell if you don't agree.
//
// Q. What are we caching?
// A. We are caching the screen representations (LyXText) of the
//    documents (Buffer,LyXParagraph) for specific BufferView widths.
// Q. Why the cache?
// A. It is not really needed, but it speeds things up a lot
//    when you have more than one document loaded at once since a total
//    rebreak (reformatting) need not be done when switching between
//    documents. When the cache is in function a document only needs to be
//    formatted upon loading and when the with of the BufferView changes.
//    Later it will also be unneccessary to reformat when having two
//    BufferViews of equal width with the same document, a simple copy
//    of the LyXText structure will do.
// Invariant for the TextCache:
//        - The buffer of the text  in the TextCache _must_ exists
//          in the bufferlist.
//        - For a text in the TextCache there _must not_ be an equivalent
//          text in any BufferView. (same buffer and width).
// Among others this mean:
//        - When a document is closed all trace of it must be removed from
//          the TextCache.
// Scenarios:
//    I believe there are only three possible scenarios where the two first
//    are also covered by the third.
//        - The simplest scenario is what we have now, a single
//          BufferView only.  
//          o Opening
//            Nothing to do with the TextCache is done when opening a file.
//          o Switching
//            We switch from buffer A to buffer B.
//            * A's text is cached in TextCache.
//            * We make a search for a text in TextCache that fits B
//              (same buffer and same width).
//          o Horizontal resize
//            If the BufferView's width (LyXView) is horizontally changed all
//            the entries in the TextCache are deleted. (This causes
//            reformat of all loaded documents when next viewed)
//          o Close
//            When a buffer is closed we don't have to do anything, because
//            to close a single buffer it is required to only exist in the
//            BufferView and not in the TextCache. Upon LFUN_QUIT we
//            don't really care since everything is deleted anyway.
//        - The next scenario is when we have several BufferViews (in one or
//          more LyXViews) of equal width.
//          o Opening
//            Nothing to do with the TextCache is done when opening a file.
//          o Switching
//            We switch from buffer A to buffer B.
//            * If A is in another Bufferview we do not put it into TextCache.
//              else we put A into TextCache.
//            * If B is viewed in another BufferView we make a copy of its
//              text and use that, else we search in TextCache for a match.
//              (same buffer same width)
//          o Horizontal resize
//            If the BufferView's width (LyXView) is horisontaly changed all
//            the entries in the TextCache is deleted. (This causes
//            reformat of all loaded documents when next viewed)
//          o Close
//        - The last scenario should cover both the previous ones, this time
//          we have several BufferViews (in one or more LyXViews) with no
//          limitations on width. (And if you wonder why the two other
//          senarios are needed... I used them to get to this one.)
//          o Opening
//            Nothing to do with the TextCache is done when opening a file.
//          o Switching
//            We switch from buffer A to buffer B.
//          o Horisontal rezize
//          o Close

typedef vector<LyXText*> TextCache;
TextCache textcache;

class text_fits {
public:
	text_fits(Buffer * b, unsigned short p)
		: buf(b), pw(p) {}
	bool operator()(TextCache::value_type & vt) {
		if (vt->params == buf && vt->paperWidth() == pw) return true;
		return false;
	}
private:
	Buffer * buf;
	unsigned short pw;
};


class show_text {
public:
	show_text(ostream & o) : os(o) {}
	void operator()(TextCache::value_type & vt) {
		os << "Buffer: " << vt->params
		   << "\nWidth: " << vt->paperWidth() << endl;
	}
private:
	ostream & os;
};

void showTextCache(string const & str)
{
	lyxerr << "TextCache: " << str << endl;
	for_each(textcache.begin(), textcache.end(), show_text(lyxerr));
}

	      
void BufferView::buffer(Buffer * b)
{
	lyxerr[Debug::INFO] << "Setting buffer in BufferView" << endl;
	if (buffer_) {
		buffer_->InsetSleep();
		buffer_->delUser(this);
		// Put the old text into the TextCache.
		textcache.push_back(text);
		showTextCache("buffer");
		// delete text;
		text = 0;
	}

	// Set current buffer
	buffer_ = b;

	if (bufferlist.getState() == BufferList::CLOSING) return;
	
	// Nuke old image
	// screen is always deleted when the buffer is changed.
	if (screen)
		delete screen;
	screen = 0;

	// If we are closing the buffer, use the first buffer as current
	if (!buffer_) {
		buffer_ = bufferlist.first();
	}

	if (buffer_) {
		lyxerr[Debug::INFO] << "  Buffer addr: " << buffer_ << endl;
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
	        buffer_->InsetWakeup();
	} else {
		lyxerr[Debug::INFO] << "  No Buffer!" << endl;
		owner_->getMenus()->hideMenus();
		updateScrollbar();
		fl_redraw_object(work_area);
		// Also remove all remaining text's from the testcache.
		showTextCache("buffer delete all");
		while (!textcache.empty()) {
			LyXText * tt = textcache.front();
			textcache.erase(textcache.begin());
			delete tt;
		}
	}
	// should update layoutchoice even if we don't have a buffer.
	owner_->updateLayoutChoice();
	owner_->getMiniBuffer()->Init();
	owner_->updateWindowTitle();
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
			       text);
}


void BufferView::resize()
{
	// This will resize the buffer. (Asger)
	if (buffer_)
		resizeCurrentBuffer();
}


static bool lgb_hack = false;

void BufferView::redraw()
{
	lyxerr[Debug::INFO] << "BufferView::redraw()" << endl;
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

	if (!buffer_) {
		fl_set_slider_value(scrollbar, 0);
		fl_set_slider_size(scrollbar, scrollbar->h);
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
	
	long maximum_height = work_area->h * 3 / 4 + cbth;
	long value = cbsf;

	// set the scrollbar
	double hfloat = work_area->h;
	double maxfloat = maximum_height;
   
	fl_set_slider_value(scrollbar, value);
	fl_set_slider_bounds(scrollbar, 0,
			     maximum_height - work_area->h);
#if FL_REVISION > 85
	double lineh = text->DefaultHeight();
	fl_set_slider_increment(scrollbar, work_area->h-lineh, lineh);
#endif
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
		text = new LyXText(work_area->w, buffer_);
	} else {
		// See if we have a text in TextCache that fits
		// the new buffer_ with the correct width.
		TextCache::iterator it =
			find_if(textcache.begin(),
				textcache.end(),
				text_fits(buffer_,
					  work_area->w));
		if (it != textcache.end()) {
			text = *it;
			// take it out of textcache.
			textcache.erase(it);
			showTextCache("resizeCurrentBuffer");
		} else {
			text = new LyXText(work_area->w, buffer_);
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
	BeforeChange();
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


void BufferView::create_view(int xpos, int ypos, int width, int height)
{
	FL_OBJECT * obj;
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
#if FL_REVISION > 85
	fl_set_border_width(-1);
#else
	fl_set_border_width(-2); // to get visible feedback
#endif
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

#if FL_REVISION > 85
	// Remove the blue feedback rectangle
	fl_set_pixmapbutton_focus_outline(obj, 0);
#endif	

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
#if FL_REVISION > 85
	fl_set_border_width(-1);
#else
	fl_set_border_width(-2); // to get visible feedback
#endif
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

#if FL_REVISION > 85
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


static
void waitForX()
{
#if 0
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
			PropModeAppend,
			reinterpret_cast<unsigned char*>(""), 0);
	XWindowEvent(fl_display, w, PropertyChangeMask, &ev);
#endif
	XSync(fl_get_display(), 0);
}


// Callback for scrollbar slider
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


// Callback for scrollbar down button
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
		if (view->buffer_ && !view->buffer_->the_locking_inset) {
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

int BufferView::WorkAreaMotionNotify(FL_OBJECT * ob, Window,
				     int /*w*/, int /*h*/,
				     XEvent * ev, void * /*d*/)
{
	if (buffer_ == 0) return 0;
	if (!screen) return 0;

	// Check for inset locking
	if (buffer_->the_locking_inset) {
		LyXCursor cursor = text->cursor;
		buffer_->the_locking_inset->
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


extern int bibitemMaxWidth(LyXFont const &);

// Single-click on work area
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
		
		if (buffer_->the_locking_inset) {
			// We are in inset locking mode
		
			/* Check whether the inset was hit. If not reset mode,
			   otherwise give the event to the inset */
			if (inset_hit != 0) {
				buffer_->the_locking_inset->
					InsetButtonPress(inset_x, inset_y,
							 button);
				return 0;
			} else {
				UnlockInset(buffer_->the_locking_inset);
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

	if (buffer_->the_locking_inset) {
		// We are in inset locking mode.

		/* LyX does a kind of work-area grabbing for insets.
		   Only a ButtonPress Event outside the inset will 
		   force a InsetUnlock. */
		buffer_->the_locking_inset->
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
			ToggleFloat();
			selection_possible = false;
			return 0;
		}
	}

	// Do we want to close a float? (click on the float-label)
	if (text->cursor.row->par->footnoteflag == 
	    LyXParagraph::OPEN_FOOTNOTE
	    && text->cursor.pos == 0
	    && text->cursor.row->previous &&
	    text->cursor.row->previous->par->
	    footnoteflag != LyXParagraph::OPEN_FOOTNOTE){
		LyXFont font (LyXFont::ALL_SANE);
		font.setSize(LyXFont::SIZE_SMALL);

		int box_x = 20; // LYX_PAPER_MARGIN;
		box_x += font.textWidth("Mwide-figM", 10);

		int screen_first = screen->first;

		if (x < box_x
		    && y + screen_first > text->cursor.y -
		    text->cursor.row->baseline
		    && y + screen_first < text->cursor.y -
		    text->cursor.row->baseline
		    + font.maxAscent() * 1.2 + font.maxDescent() * 1.2) {
			ToggleFloat();
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


/* 
 * Returns an inset if inset was hit. 0 otherwise.
 * If hit, the coordinates are changed relative to the inset. 
 * Otherwise coordinates are not changed, and false is returned.
 */
Inset * BufferView::checkInsetHit(int & x, int & y)
{
	if (!getScreen())
		return 0;
  
	int y_tmp = y + getScreen()->first;
  
	LyXCursor cursor = text->cursor;
	if (cursor.pos < cursor.par->Last() 
	    && cursor.par->GetChar(cursor.pos) == LyXParagraph::META_INSET
	    && cursor.par->GetInset(cursor.pos)
	    && cursor.par->GetInset(cursor.pos)->Editable()) {

		// Check whether the inset really was hit
		Inset * tmpinset = cursor.par->GetInset(cursor.pos);
		LyXFont font = text->GetFont(cursor.par, cursor.pos);
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
		   && cursor.par->GetChar(cursor.pos - 1) == LyXParagraph::META_INSET
		   && cursor.par->GetInset(cursor.pos - 1)
		   && cursor.par->GetInset(cursor.pos - 1)->Editable()) {
		text->CursorLeft();
		Inset * result = checkInsetHit(x, y);
		if (result == 0) {
			text->CursorRight();
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
	if (buffer_ != 0) {
		if (widthChange) {
			// All buffers need a resize
			bufferlist.resize();

			// Remove all texts from the textcache
			showTextCache("Expose delete all");
			while(!textcache.empty()) {
				LyXText * tt = textcache.front();
				textcache.erase(textcache.begin());
				delete tt;
			}
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
		if (!view->buffer_->the_locking_inset) {
			view->screen->CursorToggle();
		} else {
			view->buffer_->the_locking_inset->
				ToggleInsetCursor();
		}
		goto set_timer_and_return;
	} else {
		// Make sure that the cursor is visible.
		if (!view->buffer_->the_locking_inset) {
			view->screen->ShowCursor();
		} else {
			if (!view->buffer_->the_locking_inset->isCursorVisible())
				view->buffer_->the_locking_inset->
					ToggleInsetCursor();
		}

		// This is only run when work_area_focus or lyx_focus is false.
		Window tmpwin;
		int tmp;
		XGetInputFocus(fl_display, &tmpwin, &tmp);
		if (lyxerr.debugging(Debug::INFO)) {
			lyxerr << "tmpwin: " << tmpwin
			       << "\nwindow: " << view->owner_->getForm()->window
			       << "\nwork_area_focus: " << view->work_area_focus
			       << "\nlyx_focus      : " << view->lyx_focus
			       << endl;
		}
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


bool BufferView::available() const
{
	if (buffer_ && text) return true;
	return false;
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
	
	BeforeChange();
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
