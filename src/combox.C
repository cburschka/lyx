// -*- C++ -*-
/*
 *  Combox: A combination of two objects (a button and a browser) is
 *          encapsulated to get a combobox-like object. All XForms 
 *          functions are hidden.         
 * 
 *  GNU Copyleft (C) 1996 Alejandro Aguilar Sierra <asierra@servidor.unam.mx>
 *                        and the LyX Team.
 * 
 *  Dependencies:  Only XForms, but created to be used with LyX.
 * 
 */ 

/* Change log:
 *  
 *  2/06/1996,   Alejandro Aguilar Sierra 
 *    Created and tested.
 *  
 *  4/06/1996,   Alejandro Aguilar Sierra 
 *    Added droplist mode (a button with a black down arrow at right)
 *    and support for middle and right buttons, as XForms choice object.
 *
 *  6/06/1996,   Lars Gullik Bjønnes
 *    Added a combox with an input object. and a pre and a post handle.
 * 
 *  22/07/96,    Alejandro Aguilar Sierra 
 *    Assigned to the browser its own popup window. No more need of
 *    external pre and post handlers to simulate the wanted behaviour.
 * 
 */ 

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "combox.h"
#include <cstring>

#include "error.h"

Combox::Combox(combox_type t): type(t)
{
   browser = button = 0;
   callback = 0;
   label = 0;
   cb_arg = 0;
   _pre = 0;
   _post = 0;
   sel = 0;
   form = 0;
}


Combox::~Combox()
{
   remove();
}


void Combox::clear()
{
	if (browser) fl_clear_browser(browser);   
	sel = 0;
	if (type == FL_COMBOX_INPUT)
		fl_set_input(label, "");
	else
		fl_set_object_label(label, "");
	is_empty = true;
}


void Combox::remove()
{
	//lyxerr.print(string("Button: ") + int(button));
	if (button) {
		fl_delete_object(button);
		fl_free_object(button); 
	}
	
	//lyxerr.print(string("Label: ") + int(label));
	if (label && label!=button) {
		fl_delete_object(label);
		fl_free_object(label); 
	}
	
	//lyxerr.print(string("Form: ") + int(form));
	//lyxerr.print(string("Browser: ") + int(browser));
	if (form && browser) {
	   fl_delete_object(browser);
	   fl_free_object(browser);
	   fl_free_form(form);
	}
	button = 0; 
	browser = 0; 
	label = 0;
	form = 0;
	sel = 0;
	is_empty = true;
}


void Combox::addline(char const* text)
{
	if (!browser) return;
	fl_add_browser_line(browser, text);
	
	// By default the first item is selected
	if (!sel) {
		sel = 1;
		if (type == FL_COMBOX_INPUT)
			fl_set_input(label, text);
		else
			fl_set_object_label(label, text); 
	}
	is_empty = false;
}


bool Combox::select_text(char const* t)
{
	if (!browser || !t) return false;
	for (int i=1; i<=fl_get_browser_maxline(browser); i++) {
		if (!strcmp(t, fl_get_browser_line(browser, i))) {
			select(i);
			return true;
		}
	}
	return false;  // t does not exist in browser
}


void Combox::select(int i)
{
	if (!browser || !button) return;
	if (i>0 && i<=fl_get_browser_maxline(browser)) sel = i; 
	fl_deactivate_object(button);
	
	if (type == FL_COMBOX_INPUT)
		fl_set_input(label, fl_get_browser_line(browser, sel));
	else
		fl_set_object_label(label, fl_get_browser_line(browser, sel)); 
	fl_activate_object(button); 
}


void Combox::add(int x, int y, int w, int hmin, int hmax)
{  
	FL_OBJECT *obj;
	
	switch(type) {
	case FL_COMBOX_DROPLIST:
	{
		button = obj = fl_add_button(FL_NORMAL_BUTTON,
					     x+w-22,y,22,hmin,"@2->");
		fl_set_object_color(obj,FL_MCOL, FL_MCOL);
		fl_set_object_dblbuffer(obj, 1);
		fl_set_object_callback(obj,combo_cb,0);
	        label = obj = fl_add_button(FL_NORMAL_TEXT,x,y,w-22,hmin,"");
		fl_set_object_boxtype(obj,FL_DOWN_BOX);
		fl_set_object_color(obj,FL_MCOL,FL_BLACK);
		fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
		fl_set_object_dblbuffer(obj, 1);
		fl_set_object_callback(obj,combo_cb,0);
		break;
	}
	case FL_COMBOX_NORMAL:
	{
		button = obj = fl_add_button(FL_NORMAL_BUTTON,x,y,w,hmin,"");
		fl_set_object_color(obj,FL_MCOL, FL_MCOL);
		fl_set_object_boxtype(obj,FL_DOWN_BOX);
		fl_set_object_callback(obj,combo_cb,0);
		fl_set_object_color(obj,FL_MCOL,FL_BLACK);
		label = button;
		break;
	}
	case FL_COMBOX_INPUT:
	{
		button = obj = fl_add_button(FL_NORMAL_BUTTON,
					     x+w-22,y,22,hmin,"@2->");
		fl_set_object_color(obj,FL_MCOL, FL_MCOL);
		fl_set_object_callback(obj,combo_cb,0);
		label = obj = fl_add_input(FL_NORMAL_INPUT,x,y,w-22,hmin,"");
		fl_set_object_boxtype(obj,FL_DOWN_BOX);
		fl_set_object_return(obj, FL_RETURN_END_CHANGED);
		fl_set_object_callback(obj,input_cb,0);
		//fl_set_object_color(obj,FL_MCOL,FL_BLACK);
		//fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
		break;
	}
	} // end of switch

	label->u_vdata = (void*)this;
	button->u_vdata = (void*)this;

	// Hmm, it seems fl_create_browser is broken in xforms 0.86.
	// We have to work around that by creating the dropped browser form
	// at this point already. However, this means that we have
	// to do a little hacking: (Asger)
	FL_FORM * current_form = fl_current_form;
	fl_end_form();

	bw = w+20; bh = hmax-hmin-12;

	form = fl_bgn_form(FL_NO_BOX, bw, bh);
	browser = obj = fl_add_browser(FL_HOLD_BROWSER, 0, 0, bw,bh,"");
	fl_set_object_boxtype(obj,FL_UP_BOX);
	fl_set_object_color(obj,FL_MCOL, FL_YELLOW);
	fl_set_object_gravity(obj, NorthWestGravity, NorthWestGravity);
	fl_set_object_callback(obj,combo_cb,2);
	fl_end_form();
	browser->u_vdata = (void*)this;
	form->u_vdata = browser;
	fl_register_raw_callback(form, ButtonPressMask|KeyPressMask,peek_event);

	// And revert to adding to the old form (Asger)
	fl_addto_form(current_form);
}


static Window save_window;


void Combox::Redraw()
{
	if (browser) fl_redraw_object(browser);
	if (button) fl_redraw_object(button);
	if (label) fl_redraw_object(label);
}
 
void Combox::Show()
{
	if (_pre) _pre();
	
	int tmp;
	XGetInputFocus(fl_display, &save_window, &tmp); //BUG-Fix Dietmar
	XFlush(fl_display);
	if (button && type != FL_COMBOX_NORMAL) {
		fl_set_object_label(button, "@2<-");	      
		fl_redraw_object(button);
	}
	int x = label->form->x + label->x, y = label->form->y + label->y;
	fl_set_form_position(form, x, y + label->h);
	fl_show_form(form, FL_PLACE_POSITION, FL_NOBORDER, "");
        if (sel>0) {
		fl_set_browser_topline(browser, sel);
		fl_select_browser_line(browser, sel);
	}
	XGrabPointer(fl_display, form->window, false,
		     ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
		     GrabModeAsync, GrabModeAsync,
		     0, 0,0);
	XFlush(fl_display);
}

void Combox::Hide(int who)
{  
	if (!who && browser && label) {
		sel = fl_get_browser(browser);
		
		if (type == FL_COMBOX_INPUT)
			fl_set_input(label, fl_get_browser_line(browser, sel));
		else
			fl_set_object_label(label,
					    fl_get_browser_line(browser, sel));		
	        if (callback) callback(sel,cb_arg);
	}
        XUngrabPointer(fl_display, 0);
	XFlush(fl_display);
        if (form) {
		fl_hide_form(form);
		XSetInputFocus(fl_display, save_window,
			       RevertToParent, CurrentTime); // BUG-FIX-Dietmar
		XFlush(fl_display);
        }
	if (button) {
	        if (type != FL_COMBOX_NORMAL){
			fl_set_object_label(button, "@2->");
			fl_redraw_object(button);
		}
	} 
        if (_post) _post();
}


void Combox::activate()
{
	if (browser) fl_activate_object(browser);
	if (button) fl_activate_object(button);
	if (label) fl_activate_object(label);
}


void Combox::deactivate()
{
	if (browser) fl_deactivate_object(browser);
	if (button) fl_deactivate_object(button);
	if (label) fl_deactivate_object(label);
}

void Combox::input_cb(FL_OBJECT *ob, long)
{
	Combox *combo = (Combox*)ob->u_vdata;

	char const *text = fl_get_input(ob);

	combo->addto(text);
	combo->is_empty = false;
}


void Combox::combo_cb(FL_OBJECT *ob, long data)
{
	Combox *combo = (Combox*)ob->u_vdata;
	switch (data) {
	case 0:
	{  
		int i = combo->get();
		switch (fl_get_button_numb(ob)) {
		case 2: 
		{
			combo->select(--i); 
			if (combo->callback)
				combo->callback(combo->sel,
						combo->cb_arg);
			break;
		}
		case 3: 
		{
			combo->select(++i);  
			if (combo->callback)
				combo->callback(combo->sel,
						combo->cb_arg);
			break;
		}
		default: combo->Show(); break;
		}
		break;
	}
	case 2:
		combo->Hide();
		break;
	}
}


int Combox::peek_event(FL_FORM * form, void *xev)
{
	FL_OBJECT *ob = (FL_OBJECT *)form->u_vdata;
	Combox *combo = (Combox*)ob->u_vdata;
	
#if FL_REVISION < 86
	if(((XEvent *)xev)->type==ButtonPress && !ob->belowmouse)
#endif
#if FL_REVISION > 85
// I don't know why belowmouse does not work, but it doesn't. (Asger)
		if (((XEvent *) xev)->type == ButtonPress && (
			((XEvent *)xev)->xbutton.x - ob->x < 0 ||
			((XEvent *)xev)->xbutton.x - ob->x > ob->w ||
			((XEvent *)xev)->xbutton.y - ob->y < 0 ||
			((XEvent *)xev)->xbutton.y - ob->y > ob->h))
#endif
	{
		combo->Hide(1); 
		return 1;
	}
		
	if (((XEvent*)xev)->type != KeyPress) return 0;
	
	char s_r[10];
	static int num_bytes;
	KeySym keysym_return;
	num_bytes = XLookupString(&((XEvent*)xev)->xkey, s_r, 10, 
				  &keysym_return, 0);
	XFlush(fl_display);
	switch (keysym_return) {
	case XK_Down:
		if (fl_get_browser(combo->browser) <
		    fl_get_browser_maxline(combo->browser))
			fl_select_browser_line(combo->browser,
					       fl_get_browser(combo->browser)+1);
		if (fl_get_browser(combo->browser)>=
		    fl_get_browser_topline(combo->browser) +
		    fl_get_browser_screenlines(combo->browser))
			fl_set_browser_topline(combo->browser,
					       fl_get_browser(combo->browser)
					       - fl_get_browser_screenlines(combo->browser)+1);
		if (fl_get_browser(combo->browser)<
		    fl_get_browser_topline(combo->browser))
			fl_set_browser_topline(combo->browser,
					       fl_get_browser(combo->browser));
		return 1; 
	case XK_Up:
		if (fl_get_browser(combo->browser) > 1)
			fl_select_browser_line(combo->browser,
					       fl_get_browser(combo->browser)-1);
		if (fl_get_browser(combo->browser)>=
		    fl_get_browser_topline(combo->browser) +
		    fl_get_browser_screenlines(combo->browser))
			fl_set_browser_topline(combo->browser,
					       fl_get_browser(combo->browser)
					       - fl_get_browser_screenlines(combo->browser)+1);
		if (fl_get_browser(combo->browser) <
		    fl_get_browser_topline(combo->browser))
			fl_set_browser_topline(combo->browser,
					       fl_get_browser(combo->browser));
		return 1;
	case XK_Return:
		combo->Hide();
		return 1;
	case XK_Escape:
		combo->Hide(1);
		return 1;
	}
	return 0;  
}
	

#ifdef TESTCOMBO
typedef struct {
	FL_FORM *test;
	FL_OBJECT *bar;
	void *vdata;
	long ldata;
} FD_test;

//Combox combo(FL_COMBOX_DROPLIST);
Combox combo(FL_COMBOX_INPUT);

FD_test *fd_test;
   
FD_test *create_form_test(void)
{
   FL_OBJECT *obj;
   FD_test *fdui = (FD_test *) fl_calloc(1, sizeof(*fdui));
   
   fdui->test = fl_bgn_form(FL_NO_BOX, 320, 190);
   obj = fl_add_box(FL_UP_BOX,0,0,320,190,"");
   obj = fl_add_box(FL_DOWN_BOX,10,50,300,110,"");
   obj = fl_add_button(FL_NORMAL_BUTTON,250,10,50,30,_("Done"));
   combo.add(10,15,120,25,135); 
   fl_end_form();

  return fdui;
}

void combo_cb(int i)
{
   fprintf(stderr, "selected %d:%s\n", i, combo.getline());
}

int main(int argc, char *argv[])
{
	//int n1;
   
   // Same defaults as in lyx 
   FL_IOPT cntl;
   cntl.buttonFontSize = FL_NORMAL_SIZE;
   cntl.browserFontSize = FL_NORMAL_SIZE;
   cntl.labelFontSize = FL_NORMAL_SIZE;
   cntl.choiceFontSize = FL_NORMAL_SIZE;
   cntl.inputFontSize = FL_NORMAL_SIZE;
   cntl.borderWidth = -2;
   fl_set_defaults(FL_PDButtonFontSize, &cntl);  
   fl_set_defaults(FL_PDBrowserFontSize, &cntl);  
   fl_set_defaults(FL_PDLabelFontSize, &cntl);  
   fl_set_defaults(FL_PDChoiceFontSize, &cntl);  
   fl_set_defaults(FL_PDInputFontSize, &cntl);  
   fl_set_defaults(FL_PDBorderWidth, &cntl);
   fl_initialize(&argc, argv, 0, 0, 0);
   
   fd_test = create_form_test();

   /* fill-in form initialization code */
   combo.addline("Title");
   combo.addline("Author");
   combo.addline("Date");
   combo.addline("Abstract");
   combo.addline("Chapter");
   combo.addline("Section");
   combo.addline("Subsection");
   combo.addline("List");
   combo.addline("Description");
   combo.addline("Verse");
   combo.addline("Verbatim");
   combo.setcallback(combo_cb);
//   combo.select(4);
   
   /* show the first form */
   fl_show_form(fd_test->test,FL_PLACE_CENTER,FL_FULLBORDER,"test");
   fl_do_forms();
   return 0;
}

#endif
