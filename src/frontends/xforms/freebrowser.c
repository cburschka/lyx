/**
 * \file freebrowser.c
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 *
 * This is a rewrite of Alejandro's C++ Combox class, originally written
 * for LyX in 1996. The rewrite turns it into a native xforms widget.
 */
#include <config.h>

#include FORMS_H_LOCATION
#include "freebrowser.h"
#include <ctype.h> /* isprint */

extern void fl_hide_tooltip(void);

static void browser_cb(FL_OBJECT * ob, long data);
static int peek_event(FL_FORM * form, void * xev);


FL_FREEBROWSER * fl_create_freebrowser(void * parent)
{
    FL_FORM * current_form = fl_current_form;
    FL_FREEBROWSER * fb = fl_calloc(1, sizeof(FL_FREEBROWSER));
    FL_OBJECT * ob;

    /* Default size */
    FL_Coord const w = 100;
    FL_Coord const h = 100;

    fb->parent = parent;
    fb->callback = 0;
    fb->want_printable = 0;
    fb->last_printable = 0;

    if (current_form) fl_end_form();
    fb->form = fl_bgn_form(FL_NO_BOX, w, h);

    fb->form->u_vdata = fb;
    fl_register_raw_callback(fb->form, ButtonPressMask|KeyPressMask,
			     peek_event);

    ob = fb->browser = fl_add_browser(FL_HOLD_BROWSER, 0, 0, w, h, "");
    ob->u_vdata = fb;
    fl_set_object_boxtype(ob, FL_UP_BOX);
    fl_set_object_color(ob, FL_MCOL, FL_YELLOW);
    fl_set_object_gravity(ob, NorthWestGravity, SouthEastGravity);
    fl_set_object_callback(ob, browser_cb, 0);
    fl_end_form();

    if (current_form)
	fl_addto_form(current_form);

    return fb;
}


void fl_free_freebrowser(FL_FREEBROWSER * fb)
{
    if (!fb)
	return;

    if (fb->form)
	fl_free_form(fb->form);

    fl_free(fb);
    fb = 0;
}


void fl_show_freebrowser(FL_FREEBROWSER * fb,
			 FL_Coord x, FL_Coord y, FL_Coord w, FL_Coord h)
{
    int tmp;
    int const pos = fl_get_browser(fb->browser);

    fl_set_form_geometry(fb->form, x, y, w, h);

    XGetInputFocus(fl_get_display(), &fb->save_window, &tmp);
    XFlush(fl_get_display());

    fl_show_form(fb->form, FL_PLACE_POSITION, FL_NOBORDER, "");

    fl_select_browser_line(fb->browser, pos);
    fl_set_browser_topline(fb->browser, pos);

    XGrabPointer(fl_get_display(), fb->form->window, 0,
		 ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
		 GrabModeAsync, GrabModeAsync,
		 0, 0, 0);
    XFlush(fl_get_display());

    fl_hide_tooltip();
}


void fl_hide_freebrowser(FL_FREEBROWSER * fb)
{
    XUngrabPointer(fl_get_display(), 0);
    XFlush(fl_get_display());

    if (fb->form->visible)
	fl_hide_form(fb->form);

    XSetInputFocus(fl_get_display(), fb->save_window,
		   RevertToParent, CurrentTime);
    XFlush(fl_get_display());

    if (fb->callback)
	fb->callback(fb, 0);
}


static void browser_cb(FL_OBJECT * ob, long data)
{
    FL_FREEBROWSER * fb = ob->u_vdata;
    fl_hide_freebrowser(fb);
    if (fb->callback)
	fb->callback(fb, 1);
}


static int peek_event(FL_FORM * form, void * ev)
{
    XEvent * xev = ev;
    FL_FREEBROWSER * fb = form->u_vdata;
    FL_OBJECT * browser = fb->browser;

    fb->last_printable = 0;
    fl_hide_tooltip();

    if (xev->type == ButtonPress &&
	((int)(xev->xbutton.x) - (int)(browser->x) < 0 ||
	 (int)(xev->xbutton.x) - (int)(browser->x) > (int)(browser->w) ||
	 (int)(xev->xbutton.y) - (int)(browser->y) < 0 ||
	 (int)(xev->xbutton.y) - (int)(browser->y) > (int)(browser->h))) {
	fl_hide_freebrowser(fb);
	return 1;
    }

    if (xev->type == KeyPress) {
	KeySym keysym_return;
	char s_r[10];
	s_r[9] = '\0';
	XLookupString(&xev->xkey, s_r, 10, &keysym_return, 0);

	XFlush(fl_get_display());

	switch (keysym_return) {
	case XK_Down:
#ifdef XK_KP_Down
	case XK_KP_Down:
#endif
	{
	    int sel = fl_get_browser(browser);
	    int const top = fl_get_browser_topline(browser);
	    int const screenlines = fl_get_browser_screenlines(browser);

	    if (sel < 1 || sel >= fl_get_browser_maxline(browser))
		return 0;

	    ++sel;
	    fl_select_browser_line(browser, sel);

	    if (sel >= top + screenlines)
		fl_set_browser_topline(browser, sel - screenlines + 1);

	    if (sel < top)
		fl_set_browser_topline(browser, sel);

	    return 1;
	}

	case XK_Up:
#ifdef XK_KP_Up
	case XK_KP_Up:
#endif
	{
	    int sel = fl_get_browser(browser);
	    int const top = fl_get_browser_topline(browser);
	    int const screenlines = fl_get_browser_screenlines(browser);

	    if (sel <= 1 || sel > fl_get_browser_maxline(browser))
		return 0;

	    --sel;
	    fl_select_browser_line(browser, sel);

	    if (sel >= top + screenlines)
		fl_set_browser_topline(browser, sel - screenlines + 1);

	    if (sel < top)
		fl_set_browser_topline(browser, sel);

	    return 1;
	}

	case XK_Return:
#ifdef XK_KP_Enter
	case XK_KP_Enter:
#endif
	    fl_hide_freebrowser(fb);
	    if (fb->callback)
		fb->callback(fb, 1);
	    return 1;

	case XK_Escape:
	    fl_hide_freebrowser(fb);
	    return 1;
	default:
	    if (fb->want_printable && s_r[0] && isprint(s_r[0])) {
		fb->last_printable = s_r[0];
		fl_hide_freebrowser(fb);
		if (fb->callback)
		    fb->callback(fb, 1);
		return 1;
	    }
	}
    }
    return 0;
}
