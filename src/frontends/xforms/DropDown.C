/**
 * \file DropDown.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon, moz@compsoc.man.ac.uk
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "DropDown.h"
#include "xforms_helpers.h"

#include <iostream>
#include <cctype>


using std::vector;


namespace {
	
extern "C" void C_CompletedCB(FL_OBJECT * ob, long)
{
	DropDown * d = static_cast<DropDown*>(ob->u_vdata);
	d->completed();
}

 
extern "C" int C_PeekCB(FL_FORM * form, void *xev)
{
	DropDown * d = static_cast<DropDown*>(form->u_vdata);
	return d->peek(static_cast<XEvent*>(xev));
}
 
} // namespace anon

 
DropDown::DropDown(LyXView * lv, FL_OBJECT * ob)
	: lv_(lv)
{
	form_ = fl_bgn_form(FL_NO_BOX, ob->w, 100);
	fl_add_box(FL_UP_BOX, 0, 0, ob->w, 100, "");
	browser_ = fl_add_browser(FL_SELECT_BROWSER, 0, 0, ob->w, 100, "");
	form_->u_vdata = this;
        browser_->u_vdata = this;
	fl_set_object_callback(browser_, C_CompletedCB, 0);
	fl_register_raw_callback(form_, KeyPressMask|ButtonPressMask, C_PeekCB);
	fl_end_form();
}

 
DropDown::~DropDown()
{
	if (form_->visible) 
		fl_hide_form(form_);
	fl_free_form(form_);
}
 

void DropDown::select(vector<string> const & choices, int x, int y, int w)
{
	if (choices.empty())
		return;

	fl_set_form_geometry(form_, x, y-100, w, 100);
	fl_clear_browser(browser_);

	vector<string>::const_iterator cit = choices.begin();
	vector<string>::const_iterator end = choices.end();
	for (; cit != end; ++cit) {
		fl_add_browser_line(browser_, cit->c_str());
	}
	fl_select_browser_line(browser_, 1);
	fl_show_form(form_, FL_PLACE_POSITION, FL_NOBORDER, "");
	XGrabPointer(fl_get_display(), form_->window, false,
		     ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
		     GrabModeAsync, GrabModeAsync, 0, 0, 0);
	XFlush(fl_get_display());
}


void DropDown::line_up()
{
	if (fl_get_browser(browser_) > 1)
		fl_select_browser_line(browser_, fl_get_browser(browser_) - 1);
	if (fl_get_browser(browser_) >= fl_get_browser_topline(browser_) +
	    fl_get_browser_screenlines(browser_))
		fl_set_browser_topline(browser_, fl_get_browser(browser_)
				       - fl_get_browser_screenlines(browser_) + 1);
	if (fl_get_browser(browser_) < fl_get_browser_topline(browser_))
		fl_set_browser_topline(browser_, fl_get_browser(browser_));
}


void DropDown::line_down()
{
	if (fl_get_browser(browser_) < fl_get_browser_maxline(browser_))
		fl_select_browser_line(browser_, fl_get_browser(browser_) + 1);
	if (fl_get_browser(browser_) >= fl_get_browser_topline(browser_) +
	    fl_get_browser_screenlines(browser_))
		fl_set_browser_topline(browser_, fl_get_browser(browser_)
				       - fl_get_browser_screenlines(browser_) + 1);
	if (fl_get_browser(browser_) < fl_get_browser_topline(browser_))
		fl_set_browser_topline(browser_, fl_get_browser(browser_));
}
 

int DropDown::peek(XEvent * xev)
{
	int x,y;
	unsigned int keymask;

	fl_get_mouse(&x, &y, &keymask);
 
	if (xev->type == ButtonPress) {
		if (!(x >= form_->x && x <= (form_->x + form_->w) &&
		      y >= form_->y && y << (form_->y + form_->h))) {
			fl_hide_form(form_);
			return 1;
		}
		XUngrabPointer(fl_get_display(), CurrentTime);
	} else if (xev->type == KeyPress) {
		char s_r[10]; s_r[9] = '\0';
		KeySym keysym_return;
		XLookupString(&xev->xkey, s_r, 10, &keysym_return, 0);
		switch (keysym_return) {
		case XK_Down:
			line_down();
			return 1;
		case XK_Up:
			line_up();
			return 1;
		case XK_Return:
			completed(); 
			return 1;
		case XK_Escape:
			fl_deselect_browser(browser_);
			completed();
			return 1;
		default:
			// FIXME: if someone has a got a way to
			// convince the event to fall back to the
			// minibuffer, I'm glad to hear it.
			// fl_XPutBackEvent() doesn't work. 

			// This is a bit less elegant perhaps, but works
			// well enough. Angus 11 Jan 2002
			if (s_r[0] && isprint(s_r[0])) {
				key_pressed(s_r[0]);
				return 1;
			}
		}
	}
	return 0; 
}


void DropDown::completed()
{
	XUngrabPointer(fl_get_display(), CurrentTime);
	fl_hide_form(form_);
	result.emit(getSelectedStringFromBrowser(browser_));
}


void DropDown::key_pressed(char c)
{
	XUngrabPointer(fl_get_display(), CurrentTime);
	fl_hide_form(form_);
	keypress.emit(c);
}
