/*
 *  File:        math_symbols.C
 *  Purpose:     User interface to math symbols
 *  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx>
 *  Created:     November 1995
 *  Version:     0.8 28/03/96
 *  Description: Provides a GUI to introduce mathematical
 *               symbols in lyx.
 *
 *  Dependencies: Xlib, XForms, Lyx
 *
 *  Copyright: 1995, 1996, Alejandro Aguilar Sierra 
 *
 *   You are free to use and modify it under the terms of
 *   the GNU General Public Licence version 2 or later.
 */

#include <config.h>
#include XPM_H_LOCATION

#ifdef __GNUG__
#pragma implementation "math_panel.h"
#endif

#include <algorithm>

#include "lyx_main.h"
#include "buffer.h"
#include "BufferView.h"
#include "minibuffer.h"
#include "lyxrc.h"
#include "LyXView.h"
#include "support/lstrings.h"
#include "debug.h"
#include "lyxfunc.h"
#include "lyxtext.h"

#include "formula.h"

#include "math_panel.h"                 
#include "math_parser.h"

using std::max;
using std::endl;
using std::ostream;

extern int greek_kb_flag;

extern BufferView * current_view;

/* Bitmaps */
#include "greek.xbm"
#include "arrows.xbm"
#include "brel.xbm"
#include "bop.xbm"
#include "misc.xbm"           
#include "varsz.xbm"           
#include "dots.xbm"

/* Latex code for those bitmaps */
static char const * latex_greek[] =  {
   "Gamma", "Delta", "Theta", "Lambda", "Xi", "Pi",
   "Sigma", "Upsilon", "Phi", "Psi", "Omega",
   "alpha", "beta", "gamma", "delta", "epsilon", "varepsilon", "zeta",
   "eta", "theta", "vartheta", "iota", "kappa", "lambda", "mu",
   "nu", "xi", "pi", "varpi", "rho", "sigma", "varsigma",
   "tau", "upsilon", "phi", "varphi", "chi", "psi", "omega", ""
};

static char const * latex_brel[] = {
  "leq", "geq", "equiv", "models", 
  "prec", "succ", "sim", "perp", 
  "preceq", "succeq", "simeq", "mid", 
  "ll", "gg", "asymp", "parallel", 
  "subset", "supset", "approx", "smile", 
  "subseteq", "supseteq", "cong", "frown", 
  "sqsubseteq", "sqsupseteq", "doteq", "neq", 
  "in", "ni", "propto", "notin", 
  "vdash", "dashv", "bowtie", ""
};

static char const * latex_arrow[] = {
  "downarrow", "leftarrow", "Downarrow", "Leftarrow", 
  "hookleftarrow", "rightarrow", "uparrow", "Rightarrow", "Uparrow",
  "hookrightarrow", "updownarrow", "Leftrightarrow", "leftharpoonup", 
  "rightharpoonup", "rightleftharpoons", "leftrightarrow", "Updownarrow", 
  "leftharpoondown", "rightharpoondown", "mapsto",
  "Longleftarrow", "Longrightarrow", "Longleftrightarrow", 
  "longleftrightarrow", "longleftarrow", "longrightarrow", "longmapsto",
  "nwarrow", "nearrow", "swarrow", "searrow",  "",
};

char const * latex_varsz[] = {
"sum", "int", "oint", 
"prod", "coprod", "bigsqcup", 
"bigotimes", "bigodot", "bigoplus", 
"bigcap", "bigcup", "biguplus", 
"bigvee", "bigwedge", ""
};

static char const * latex_bop[] = {   
  "pm", "cap", "diamond", "oplus", 
  "mp", "cup", "bigtriangleup", "ominus", 
  "times", "uplus", "bigtriangledown", "otimes", 
  "div", "sqcap", "triangleright", "oslash", 
  "cdot", "sqcup", "triangleleft", "odot", 
  "star", "vee", "amalg", "bigcirc", 
  "setminus", "wedge", "dagger", "circ", 
  "bullet", "wr", "ddagger", ""
};

static char const * latex_misc[] = {
  "nabla", "partial", "infty", "prime", "ell", 
  "emptyset", "exists", "forall", "imath",  "jmath",
  "Re", "Im", "aleph", "wp", "hbar", 
  "angle", "top", "bot", "Vert", "neg", 
  "flat", "natural", "sharp", "surd", "triangle", 
  "diamondsuit", "heartsuit", "clubsuit", "spadesuit", "" 
};

static char const * latex_dots[] = { 
   "ldots", "cdots", "vdots", "ddots"
};

extern char const ** mathed_get_pixmap_from_icon(int d);
extern "C" void math_cb(FL_OBJECT*, long);
static char const ** pixmapFromBitmapData(char const *, int, int);
void math_insert_symbol(char const * s);
bool math_insert_greek(char c);

BitmapMenu * BitmapMenu::active = 0;

BitmapMenu::BitmapMenu(int n,  FL_OBJECT * bt, BitmapMenu * prevx)
	: current_(0), bitmaps_(n)
{
   w = h = 0;
   form = 0;
   ww = 2 * FL_abs(FL_BOUND_WIDTH);
   x = y = ww;
   y += 8;
   button = bt;
   button->u_vdata = this;
   prev = prevx;
   next = 0;
   if (prev)
     prev->next = this;
}


BitmapMenu::~BitmapMenu()
{
	delete next;
	if (form->visible) Hide();
	fl_free_form(form);  
}


void BitmapMenu::Hide()
{
   fl_hide_form(form);
   fl_set_button(button, 0);
   active = 0;
}


void BitmapMenu::Show()
{
   if (active)
     active->Hide();
   active = this;
   //   int x = button->form->x + button->x, y = button->form->y + button->y;
   //   fl_set_form_position(form, x, y + button->h);
   fl_set_button(button, 1);
   fl_show_form(form, FL_PLACE_MOUSE, FL_NOBORDER, "");
}

FL_OBJECT *
BitmapMenu::AddBitmap(int id, int nx, int ny, int bw, int bh,
		      unsigned char const * data, Bool vert)
{
   if (current_ >= bitmaps_.size())
     return 0;
   int wx = bw+ww/2, wy = bh+ww/2;
   wx += (wx % nx);
   wy += (wy % ny); 
   FL_OBJECT * obj = fl_create_bmtable(1, x, y, wx, wy, "");   
   fl_set_object_callback(obj, math_cb, id);
   fl_set_object_lcol(obj, FL_BLUE);    
   fl_set_object_boxtype(obj, FL_UP_BOX);   
   fl_set_bmtable_data(obj, nx, ny, bw, bh, data);
   if (vert) { 
      y += wy + 8;
      h = max(y, h);
      w = max(x + wx + ww, w);
   } else  {
      x += wx + 8;
      w = max(x, w);
      h = max(y + wy + ww, h);
   }
   bitmaps_[current_++] = obj;
   return obj;
}

void BitmapMenu::Create()
{
   if (current_ < bitmaps_.size())  {
	   lyxerr << "Error: Bitmaps not created!" << endl;
      return;
   }
   form = fl_bgn_form(FL_UP_BOX, w, h);   
   for (current_ = 0; current_ < bitmaps_.size(); ++current_) {
      fl_add_object(form, bitmaps_[current_]);
      bitmaps_[current_]->u_vdata = this;
   }
   fl_end_form();
   fl_register_raw_callback(form, KeyPressMask, C_peek_event);
}

int BitmapMenu::GetIndex(FL_OBJECT* ob)
{
   if (active == this) {
      int k = 0;
      for (current_ = 0; current_ < bitmaps_.size(); ++current_) {
	 if (bitmaps_[current_] == ob) 
	   return k+fl_get_bmtable(ob);
	 k += fl_get_bmtable_maxitems(bitmaps_[current_]);
      }
   }
   return -1;
}

int peek_event(FL_FORM * /*form*/, void *xev)
{
   if (BitmapMenu::active == 0)
     return 0;
  
   if (static_cast<XEvent *>(xev)->type == ButtonPress)
   {
	 BitmapMenu::active->Hide();
	 return 1;
   }
   if (static_cast<XEvent *>(xev)->type == KeyPress)
   {
      char c[5];
      KeySym keysym;
      XLookupString(&static_cast<XEvent *>(xev)->xkey, &c[0], 5, &keysym, 0);
      if (keysym == XK_Left) 
	BitmapMenu::active->Prev(); else
      if (keysym == XK_Right) 
	BitmapMenu::active->Next(); 
      else 
	BitmapMenu::active->Hide();
      return 1;
   }
   return 0;  
}

// This is just a wrapper.
extern "C" int C_peek_event(FL_FORM *form, void *ptr) {
  return peek_event(form, ptr);
}


extern "C" void math_cb(FL_OBJECT* ob, long data)
{
   BitmapMenu * menu = static_cast<BitmapMenu*>(ob->u_vdata);
   int i = menu->GetIndex(ob);   
   char const *s = 0;

//   lyxerr << "data[" << data << "]";
   if (i<0) return;
   switch (data)  {
    case MM_GREEK: 
      s = latex_greek[i]; 
      break;
    case MM_ARROW: 
      s = latex_arrow[i]; 
      break;  
    case MM_BRELATS: 
      s = latex_brel[i]; 
      break;  
    case MM_BOP: 
      s = latex_bop[i]; 
      break;  
    case MM_VARSIZE: 
      s = latex_varsz[i];  
      break;
    case MM_MISC: 
      s = latex_misc[i];  
      break;
    case MM_DOTS: 
//      lyxerr << "dots[" << latex_dots[i] << " " << i << "]";
      s = latex_dots[i-29];  
      break;
   }
  
   if (s)  {
      if (current_view->available() && lyxrc.display_shortcuts) {
	  current_view->owner()->getMiniBuffer()->Set("Inserting symbol ", s);
      }
      current_view->owner()->getLyXFunc()->Dispatch(LFUN_INSERT_MATH, s);
   }      
   if (menu)  
     menu->Hide(); 
}

char const ** get_pixmap_from_symbol(char const * arg, int wx, int hx)
{
   char const ** data = 0;   		    
   latexkeys * l = in_word_set (arg, strlen(arg));
   if (!l) 
    return 0;
    
   switch (l->token) {
    case LM_TK_FRAC:
      data = mathed_get_pixmap_from_icon(MM_FRAC);
      break;
    case LM_TK_SQRT: 
      data = mathed_get_pixmap_from_icon(MM_SQRT);
      break;
    case LM_TK_BIGSYM:
    case LM_TK_SYM:
       // I have to use directly the bitmap data since the
       // bitmap tables are not yet created when this
       // function is called.
       data = pixmapFromBitmapData(arg, wx, hx);
       break;
   }
   
   return data;
}

bool math_insert_greek(BufferView * bv, char c)
{
   if (bv->available() &&
       (('A' <= c && c <= 'Z') ||
	('a'<= c && c<= 'z')))   {
      string tmp;
      tmp = c;
      if (!bv->theLockingInset() || bv->theLockingInset()->IsTextInset()) {
	 int greek_kb_flag_save = greek_kb_flag;
	 InsetFormula * new_inset = new InsetFormula();
	 bv->beforeChange();
	 if (!bv->insertInset(new_inset)) {
	     delete new_inset;
	     return false;
	 }
//	 Update(1);//BUG
	 new_inset->Edit(bv, 0, 0, 0);
	 new_inset->LocalDispatch(bv, LFUN_SELFINSERT, tmp);
	 if (greek_kb_flag_save < 2) {
		 bv->unlockInset(new_inset); // bv->theLockingInset());
		 bv->text->CursorRight(bv, true);
	 }
      } else
	 if (bv->theLockingInset()->LyxCode() == Inset::MATH_CODE ||
	     bv->theLockingInset()->LyxCode() == Inset::MATHMACRO_CODE)
		static_cast<InsetFormula*>(bv->theLockingInset())->LocalDispatch(bv, LFUN_SELFINSERT, tmp);
	 else
		lyxerr << "Math error: attempt to write on a wrong "
			"class of inset." << endl;
      return true;
   }
   return false;
}


void math_insert_symbol(BufferView * bv, string const & s)
{
   if (bv->available())   {
      if (!bv->theLockingInset() || bv->theLockingInset()->IsTextInset()) {
	 InsetFormula * new_inset = new InsetFormula();
	 bv->beforeChange();
	 if (!bv->insertInset(new_inset)) {
	     delete new_inset;
	     return;
	 }
//	 Update(1);//BUG
	 new_inset->Edit(bv, 0, 0, 0);
	 new_inset->InsertSymbol(bv, s);
      } else
	if (bv->theLockingInset()->LyxCode() == Inset::MATH_CODE ||
	    bv->theLockingInset()->LyxCode() == Inset::MATHMACRO_CODE)
		static_cast<InsetFormula*>(bv->theLockingInset())->InsertSymbol(bv, s);
        else 
		lyxerr << "Math error: attempt to write on a wrong "
			"class of inset." << endl;
   }
}


BitmapMenu * sym_menu= 0;

void  create_symbol_menues(FD_panel * symb_form)
{
   FL_OBJECT * obj; 
   BitmapMenu * menu;
   
   sym_menu = menu = new BitmapMenu(2, symb_form->greek);
   obj = menu->AddBitmap(MM_GREEK, 6, 2, Greek_width, Greek_height, 
			   Greek_bits);         
   fl_set_bmtable_maxitems(obj, 11);
   obj = menu->AddBitmap(MM_GREEK, 7, 4, greek_width, greek_height,
			   greek_bits); 
   menu->Create();
   
   menu = new BitmapMenu(1, symb_form->boperator, menu);
   obj = menu->AddBitmap(MM_BOP, 4, 8, bop_width, bop_height,
                         bop_bits);      
   fl_set_bmtable_maxitems(obj, 31);
   menu->Create();   
      
   menu = new BitmapMenu(1, symb_form->brelats, menu);
   obj = menu->AddBitmap(MM_BRELATS, 4, 9, brel_width, brel_height, 
			 brel_bits);        
   fl_set_bmtable_maxitems(obj, 35);       
   menu->Create();
   
   menu = new BitmapMenu(3, symb_form->arrow, menu);
   obj = menu->AddBitmap(MM_ARROW, 5, 4, arrow_width, arrow_height,
			 arrow_bits);
   obj = menu->AddBitmap(MM_ARROW, 2, 4, larrow_width, larrow_height, 
			 larrow_bits, False);          
   fl_set_bmtable_maxitems(obj, 7);
   obj = menu->AddBitmap(MM_ARROW, 2, 2, darrow_width, darrow_height, 
			 darrow_bits);
   menu->Create(); 
     
   menu = new BitmapMenu(1, symb_form->varsize, menu);
    obj = menu->AddBitmap(MM_VARSIZE, 3, 5, varsz_width, varsz_height, 
			 varsz_bits);         
   fl_set_bmtable_maxitems(obj, 14);
   menu->Create();
      
   menu = new BitmapMenu(2, symb_form->misc, menu);
     obj = menu->AddBitmap(MM_MISC, 5, 6, misc_width, misc_height,
			   misc_bits);         
     fl_set_bmtable_maxitems(obj, 29);
     obj = menu->AddBitmap(MM_DOTS, 4, 1, dots_width, dots_height, 
			   dots_bits);         
   menu->Create();
}

static
char const ** pixmapFromBitmapData(char const * s, int wx, int hx)
{
    int i;
    char const ** data = 0;
    
    int id = -1;
    
    for (i = 0; i < 6; ++i) {
	char const ** latex_str = 0;
	switch (i) {
	 case 0: latex_str = latex_greek; break;
	 case 1: latex_str = latex_bop; break;
	 case 2: latex_str = latex_brel; break;
	 case 3: latex_str = latex_arrow; break;
	 case 4: latex_str = latex_varsz; break;
	 case 5: latex_str = latex_misc; break;
	}
	
	for (int k = 0; latex_str[k][0] > ' '; ++k) {
	    if (strcmp(latex_str[k], s) == 0) {
		id = k;
		break;
	    }
	}
	if (id >= 0) break;
    }
    if (i < 6 && id >= 0) {
	unsigned char const * bdata = 0;
	int w = 0, h = 0, dw = 0, dh = 0;

	lyxerr[Debug::MATHED] << "Imando " << i << ", " << id << endl;
	switch (i) {
	 case 0: 
	    if (id<= 10) {
		w = Greek_width;
		h = Greek_height;
		bdata = Greek_bits;
		dw = 6;  dh = 2;
	    } else {
		w = greek_width;
		h = greek_height;
		bdata = greek_bits;
		dw = 7;  dh = 4;
		id -= 11;
	    }
	    break;
	 case 1:
	    w = bop_width;
	    h = bop_height;
	    bdata = bop_bits;
	    dw = 4;  dh = 8;
	    break;
	 case 2:
	    w = brel_width;
	    h = brel_height;
	    bdata = brel_bits;
	    dw = 4;  dh = 9;
	    break;
	 case 3:
	    if (id<20) {		
		w = arrow_width;
		h = arrow_height;
		bdata = arrow_bits;
		dw = 5;  dh = 4;
	    } else if (id>28) {		
		w = darrow_width;
		h = darrow_height;
		bdata = darrow_bits;
		dw = 2;  dh = 2;
		id -= 29;
	    } else {		
		w = larrow_width;
		h = larrow_height;
		bdata = larrow_bits;
		dw = 2;  dh = 4;
		id -= 20;
	    }
	    break;
	 case 4:
	    w = varsz_width;
	    h = varsz_height;
	    bdata = varsz_bits;
	    dw = 3;  dh = 5;
	    break;
	 case 5:
	    w = misc_width;
	    h = misc_height;
	    bdata = misc_bits;
	    dw = 5;  dh = 6;
	    break;
	}
	int ww = w/dw, hh = h/dh, x, y;
   
	XImage * xima = XCreateImage(fl_get_display(), 0, 1, XYBitmap, 0, 
				    const_cast<char*>(reinterpret_cast<char const *>(bdata)), w, h, 8, 0);
	xima->byte_order = LSBFirst;
	xima->bitmap_bit_order = LSBFirst;
	x = (id % dw)*ww;
	y = (id/dw)*hh;
	if (ww > wx) ww = wx;
	if (hh > hx) hh = hx;
	XImage * sbima = XSubImage(xima, x, y, ww, hh);
	XpmCreateDataFromImage(fl_get_display(), const_cast<char***>(&data), sbima, sbima, 0);
	
	// Dirty hack to get blue symbols quickly
	char * sx = const_cast<char*>(strstr(data[2], "FFFFFFFF"));
	if (sx) {
	    for (int k = 0; k < 8; ++k) sx[k] = '0';
	}

//	XDestroyImage(xima);
    }

    return data;
}

