/**
 * \file combox.c
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
#include "combox.h"
#include "freebrowser.h"

extern void fl_add_child(FL_OBJECT *, FL_OBJECT *);
extern void fl_addto_freelist(void *);


typedef struct {
     /** A pointer to the parent widget */
    FL_OBJECT * combox;

    FL_FREEBROWSER * freebrowser;
    int browser_height;

    /** The browser will be displayed either below or above the main body. */
    int browser_position;

    /**  button_state displays a down or up arrow depending on whether the
     *	 browser is visible or not.
     *   Click on it to toggle the browser.
     */
    FL_OBJECT * button_state;

    /**  button_chosen displays the current selection from the browser.
     *	 Click on it to toggle the browser.
     */
    FL_OBJECT * button_chosen;
} COMBOX_SPEC;


enum { ACTIVATE, DEACTIVATE };

enum { COMBOX_OPEN, COMBOX_CLOSED };


/* function declarations */
static int combox_pre(FL_OBJECT *, int, FL_Coord, FL_Coord, int, void *);
static int combox_post(FL_OBJECT *, int, FL_Coord, FL_Coord, int, void *);
static int combox_handle(FL_OBJECT *, int, FL_Coord, FL_Coord, int, void *);

static void update_button_chosen(FL_FREEBROWSER * fb, int action);
static void chosen_cb(FL_OBJECT * ob, long data);
static void state_cb(FL_OBJECT * ob, long data);

static void show_browser(COMBOX_SPEC * sp);
static void set_activation(FL_OBJECT * ob, int activation);
static void set_state_label(COMBOX_SPEC * sp, int state);
static void attrib_change(COMBOX_SPEC * sp);


FL_OBJECT * fl_create_combox(FL_COMBOX_TYPE type,
			     FL_Coord x, FL_Coord y, FL_Coord w, FL_Coord h,
			     char const * label)
{
    FL_OBJECT * ob;
    FL_OBJECT * button;
    COMBOX_SPEC * sp;

    /* The width and x-position of button_state, respectively. */
    FL_Coord const ws = 0.7 * h;
    FL_Coord const xs = x + w - ws;

    /* The width of button_chosen */
    FL_Coord const wc = (type == FL_DROPLIST_COMBOX) ? (w - ws) : w;

    ob = fl_make_object(FL_COMBOX, type, x, y, w, h, label, combox_handle);
    ob->align = FL_ALIGN_LEFT;

    sp = ob->spec = fl_calloc(1, sizeof(COMBOX_SPEC));
    sp->combox = ob;
    sp->browser_height = 100;
    sp->browser_position = FL_FREEBROWSER_BELOW;

    sp->freebrowser = fl_create_freebrowser(sp);
    sp->freebrowser->callback = update_button_chosen;

    sp->button_state = 0;
    if (type == FL_DROPLIST_COMBOX) {
	sp->button_state = fl_add_button(FL_NORMAL_BUTTON, xs, y, ws, h, "");

	button = sp->button_state;
	fl_set_object_lalign(button, FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
	fl_set_object_callback(button, state_cb, 0);
	fl_set_object_posthandler(button, combox_post);
	fl_set_object_prehandler(button,  combox_pre);
	set_state_label(sp, COMBOX_CLOSED);

	set_activation(button, DEACTIVATE);
	button->parent = ob;
	button->u_vdata = sp;
    }

    sp->button_chosen = fl_add_button(FL_NORMAL_TEXT, x, y, wc, h, "");

    button = sp->button_chosen;
    fl_set_object_boxtype(button, FL_FRAME_BOX);
    fl_set_object_lalign(button, FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_callback(button, chosen_cb, 0);
    fl_set_object_posthandler(button, combox_post);
    fl_set_object_prehandler(button,  combox_pre);
    set_activation(button, DEACTIVATE);
    button->parent = ob;
    button->u_vdata = sp;

    return ob;
}


FL_OBJECT * fl_add_combox(FL_COMBOX_TYPE type,
			  FL_Coord x, FL_Coord y, FL_Coord w, FL_Coord h,
			  char const * label)
{
    FL_OBJECT * ob = fl_create_combox(type, x, y, w, h, label);
    COMBOX_SPEC * sp = ob->spec;

    if (sp->button_state)
	fl_add_child(ob, sp->button_state);
    fl_add_child(ob, sp->button_chosen);

    fl_add_object(fl_current_form, ob);
    return ob;
}


void fl_set_combox_browser_height(FL_OBJECT * ob, int bh)
{
    COMBOX_SPEC * sp;

    if (!ob || ob->objclass != FL_COMBOX)
	return;

    sp = ob->spec;
    sp->browser_height = bh;
}


void fl_set_combox_position(FL_OBJECT * ob, FL_COMBOX_POSITION position)
{
    COMBOX_SPEC * sp;

    if (!ob || ob->objclass != FL_COMBOX)
	return;

    sp = ob->spec;
    sp->browser_position = (position == FL_COMBOX_ABOVE) ?
	FL_FREEBROWSER_ABOVE : FL_FREEBROWSER_BELOW;

    set_state_label(sp, COMBOX_CLOSED);
}


void fl_clear_combox(FL_OBJECT * ob)
{
    COMBOX_SPEC * sp;
    FL_OBJECT * browser;

    if (!ob || ob->objclass != FL_COMBOX)
	return;

    sp = ob->spec;
    browser = sp->freebrowser->browser;

    fl_clear_browser(browser);
    fl_set_object_label(sp->button_chosen, "");
}


void fl_addto_combox(FL_OBJECT * ob, char const * text)
{
    COMBOX_SPEC * sp;
    FL_OBJECT * browser;
    int i;
    int j;
    char line[128];

    if (!ob || ob->objclass != FL_COMBOX)
	return;

    sp = ob->spec;
    browser = sp->freebrowser->browser;

    /* Split the string on '|' boundaries. */
    i = j = 0;
    for (; text[i] != '\0'; ++i) {
	if (text[i] == '|') {
	    line[j] = '\0';
	    fl_add_browser_line(browser, line);
	    j = 0;
	} else {
	    line[j++] = text[i];
	}
    }

    if (j != 0)
	{
	    line[j] = '\0';
	    fl_add_browser_line(browser, line);
	}

    /* By default the first item is selected */
    if (!fl_get_browser(browser)) {
	fl_set_object_label(sp->button_chosen, text);
	set_activation(sp->button_chosen, ACTIVATE);
	if (sp->button_state)
	    set_activation(sp->button_state,  ACTIVATE);
    }
}


void fl_set_combox(FL_OBJECT * ob, int sel)
{
    COMBOX_SPEC * sp;
    FL_OBJECT * browser;

    if (!ob || ob->objclass != FL_COMBOX)
	return;

    sp = ob->spec;
    browser = sp->freebrowser->browser;

    if (sel < 1 || sel > fl_get_browser_maxline(browser))
	return;

    fl_select_browser_line(browser, sel);
    fl_set_object_label(sp->button_chosen, fl_get_browser_line(browser, sel));
}


int fl_get_combox(FL_OBJECT * ob)
{
    COMBOX_SPEC * sp;
    FL_OBJECT * browser;

    if (!ob || ob->objclass != FL_COMBOX)
	return 0;

    sp = ob->spec;
    browser = sp->freebrowser->browser;
    return fl_get_browser(browser);
}


char const * fl_get_combox_text(FL_OBJECT * ob)
{
    COMBOX_SPEC * sp;

    if (!ob || ob->objclass != FL_COMBOX)
	return 0;

    sp = ob->spec;
    return sp->button_chosen->label;
}


char const * fl_get_combox_line(FL_OBJECT * ob, int line)
{
    COMBOX_SPEC * sp;
    FL_OBJECT * browser;
    int maxlines;

    if (line < 1 || !ob || ob->objclass != FL_COMBOX)
	return 0;

    sp = ob->spec;
    browser = sp->freebrowser->browser;

    maxlines = fl_get_browser_maxline(browser);
    if (line > maxlines)
	return 0;

    return fl_get_browser_line(browser, line);
}


int fl_get_combox_maxitems(FL_OBJECT * ob)
{
    COMBOX_SPEC * sp;
    FL_OBJECT * browser;

    if (!ob || ob->objclass != FL_COMBOX)
	return 0;

    sp = ob->spec;
    browser = sp->freebrowser->browser;
    return fl_get_browser_maxline(browser);
}


void fl_show_combox_browser(FL_OBJECT * ob)
{
    if (!ob || ob->objclass != FL_COMBOX)
	return;

    show_browser(ob->spec);
}


void fl_hide_combox_browser(FL_OBJECT * ob)
{
    COMBOX_SPEC * sp;

    if (!ob || ob->objclass != FL_COMBOX)
	return;

    sp = ob->spec;
    fl_free_freebrowser(sp->freebrowser);
}


static int combox_pre(FL_OBJECT * ob, int ev, FL_Coord mx, FL_Coord my,
		      int key, void *xev)
{
    COMBOX_SPEC * sp = ob->u_vdata;
    FL_OBJECT * combox = sp->combox;

    return combox->prehandle ?
	combox->prehandle(combox, ev, mx, my, key, xev) : 0;
}


static int combox_post(FL_OBJECT * ob, int ev, FL_Coord mx, FL_Coord my,
		       int key, void *xev)
{
    COMBOX_SPEC * sp = ob->u_vdata;
    FL_OBJECT * combox = sp->combox;

    return combox->posthandle ?
	combox->posthandle(combox, ev, mx, my, key, xev) : 0;
}


static int combox_handle(FL_OBJECT * ob, int event, FL_Coord mx, FL_Coord my,
			 int key, void * ev)
{
    if (!ob || ob->objclass != FL_COMBOX)
	return 0;

    switch (event) {
    case FL_DRAW:
	attrib_change(ob->spec);
	/* Fall through */
    case FL_DRAWLABEL:
	fl_draw_object_label(ob);
 	break;
    case FL_SHORTCUT:
	show_browser(ob->spec);
	break;
    case FL_FREEMEM: {
	COMBOX_SPEC * sp = ob->spec;
	fl_free_freebrowser(sp->freebrowser);
	/* children take care of themselves, but we must make sure that
	   sp itself is free-d eventually. */
	fl_addto_freelist(sp);
	break;
    }
    }
    return 0;
}


static void set_activation(FL_OBJECT * ob, int activation)
{
    switch (activation) {
    case ACTIVATE:
	fl_activate_object(ob);
	fl_set_object_lcol(ob, FL_LCOL);
	break;
    case DEACTIVATE:
	fl_deactivate_object(ob);
	fl_set_object_lcol(ob, FL_INACTIVE);
    }
}


static void show_browser(COMBOX_SPEC * sp)
{
    FL_OBJECT * ob = sp->combox;

    /* The browser dimensions. */
    FL_Coord const bw = ob->w + 20;
    FL_Coord const bh = sp->browser_height;

    FL_Coord const abs_x = ob->form->x + ob->x;

    FL_Coord abs_y = ob->form->y + ob->y;
    abs_y += (sp->browser_position == FL_FREEBROWSER_BELOW) ? ob->h : -bh;

    set_state_label(sp, COMBOX_OPEN);
    fl_show_freebrowser(sp->freebrowser, abs_x, abs_y, bw, bh);
}


static void state_cb(FL_OBJECT * ob, long data)
{
    show_browser(ob->u_vdata);
}


static void chosen_cb(FL_OBJECT * ob, long data)
{
    show_browser(ob->u_vdata);
}


static void update_button_chosen(FL_FREEBROWSER * fb, int action)
{
    COMBOX_SPEC * sp = fb->parent;

    FL_OBJECT * browser = sp->freebrowser->browser;
    FL_OBJECT * combox = sp->combox;
    if (!browser || !combox) return;

    set_state_label(sp, COMBOX_CLOSED);

    if (action == 1) {
	int const sel = fl_get_browser(browser);
	char const * const text = fl_get_browser_line(browser, sel);
	fl_set_object_label(sp->button_chosen, text);
	fl_call_object_callback(combox);
    }
}


static void set_state_label(COMBOX_SPEC * sp, int state)
{
    char const * const up   = "@2<-";
    char const * const down = "@2->";
    char const * label = 0;

    if (!sp->button_state)
	return;

    if (sp->browser_position == FL_FREEBROWSER_BELOW) {
	label = (state == COMBOX_OPEN) ? up : down;
    } else {
	label = (state == COMBOX_OPEN) ? down : up;
    }
    fl_set_object_label(sp->button_state, label);
    fl_redraw_object(sp->button_state);
}


static void attrib_change(COMBOX_SPEC * sp)
{
    FL_OBJECT * parent = sp->combox;
    FL_OBJECT * button = sp->button_chosen;

    button->boxtype = parent->boxtype;
    button->col1    = parent->col1;
    button->col2    = parent->col2;
    button->bw      = parent->bw;

    if (sp->button_state) {
	button = sp->button_state;

	/* The boxtype is not changed */
	button->col1    = parent->col1;
	button->col2    = parent->col2;
	button->bw      = parent->bw;
    }
}
