// -*- C++ -*-
/*
 *  Combox: A combination of two objects (a button and a browser) is
 *          encapsulated to get a combobox-like object. All XForms 
 *          functions are hidden.         
 * 
 *  GNU Copyleft 1996 Alejandro Aguilar Sierra <asierra@servidor.unam.mx>
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
 */ 

#ifndef COMBOX_H
#define COMBOX_H

#ifdef __GNUG__
#pragma interface
#endif

#include FORMS_H_LOCATION
#include <cstdlib>
#include "LString.h"

///
enum combox_type {
	///
	FL_COMBOX_NORMAL,
	///
	FL_COMBOX_DROPLIST,
	///
	FL_COMBOX_INPUT
};

class Combox;

/// callback prototype
typedef void (*FL_COMBO_CB) (int, void *, Combox *);
/// pre post prototype
typedef void (*FL_COMBO_PRE_POST) ();


///
class Combox {
public:
	///
	explicit Combox(combox_type t = FL_COMBOX_NORMAL);
	///
	~Combox();

	/** To add this object to a form. Note that there are two heights
	 for normal (button) and expanded (browser) mode each.
	*/
	void add(int x, int y, int w, int hmin, int hmax);
	
	/// Add lines. Same as for fl_browser object
	void addline(string const &);
	/// Add lines. Same as for fl_browser object
	void addto(string const &);
	
	/// Returns the selected item
	int get();
   
	/// Returns a pointer to the selected line of text
	string const getline();
   
	///  Select an arbitrary item
	void select(int);
	///
        bool select_text(string const &);
   
	///  Clear all the list
	void clear();

	/// Is the combox cleared (empty)
	bool empty() { return is_empty; }
	
	/// Remove the objects from the form they are in. 
	void remove();

	/**  Assign a callback to this object. The callback should be a void
	 function with a int and a void pointer as parameters.
	*/
	void setcallback(FL_COMBO_CB, void *);
   
        ///  Pre handler
	void setpre(FL_COMBO_PRE_POST);
	/// Post handler
	void setpost(FL_COMBO_PRE_POST);
	
	///  XForms attributes
	void resize(unsigned);
	///
	void gravity(unsigned, unsigned);
	///
	void activate();
	///
	void deactivate();
	///
        void shortcut(string const &, int);
	///
	void Redraw();
	///
	void Show();
	///
	static void combo_cb(FL_OBJECT *, long);
	///
	static void input_cb(FL_OBJECT *, long);
	///
        static int  peek_event(FL_FORM *, void *);
 protected:
        /// At least Hide should not be public
	void Hide(int who = 0);
	///
	FL_OBJECT * browser;
 private:
	///
	combox_type type;
	///
        int bw, bh;
	///
	int sel;
	///
	bool is_empty;
	///
	FL_COMBO_CB callback;
	///
	void * cb_arg;
	///
	FL_COMBO_PRE_POST _pre;
	///
	FL_COMBO_PRE_POST _post;
	///
	FL_OBJECT * button;
	///
	FL_OBJECT * label;
	///
        FL_FORM* form;
};



//-----------------  Inline methods  --------------------------- 

inline
void Combox::addto(string const & text)
{
	if (browser) {
		fl_addto_browser(browser, text.c_str());
		is_empty = false;
	}
}


inline
void Combox::resize(unsigned r)
{
   fl_set_object_resize(button, r);
   if (label!= button) fl_set_object_resize(label, r); 
}


inline
void Combox::gravity(unsigned g1, unsigned g2)
{
   fl_set_object_gravity(button, g1, g2);
   if (label!= button) fl_set_object_gravity(label, g1, g2); 
}


inline
void Combox::shortcut(string const & s, int i)
{
   if (button)
      fl_set_object_shortcut(button, s.c_str(), i);
}


inline
void Combox::setcallback(FL_COMBO_CB cb, void * a = 0)
{
   callback = cb;
   cb_arg = a;
}


inline
void Combox::setpre(FL_COMBO_PRE_POST cb)
{
	_pre = cb;
}


inline
void Combox::setpost(FL_COMBO_PRE_POST cb)
{
	_post = cb;
}


inline
int Combox::get()
{
   return sel;
}


inline
string const Combox::getline()
{
    if (type == FL_COMBOX_INPUT) 
      return fl_get_input(label);
    else
      return (browser && sel > 0) ? fl_get_browser_line(browser, sel) : string();
}

#endif
