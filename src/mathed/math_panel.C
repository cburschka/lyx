/*
 *  File:        math_panel.C
 *  Purpose:     Mathed GUI for lyx
 *  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx>
 *  Created:     March 28, 1996
 * 
 *  Dependencies: Xlib, Xpm, XForms, Lyx
 *
 *  Copyright: 1996, Alejandro Aguilar Sierra 
 *
 *   You are free to use and modify it under the terms of
 *   the GNU General Public Licence version 2 or later.
 */

#include <config.h>

#include FORMS_H_LOCATION

#include "lyx_gui_misc.h" 
#include "math_panel.h"
#include "symbol_def.h"
#include "formula.h"
#include "lyxfunc.h"
#include "gettext.h"

/* Bitmaps */
#include "delim.xbm"
#include "delim0.xpm"
#include "delim.xpm"
#include "deco.xbm"
#include "deco.xpm"
#include "space.xpm"
#include "sqrt.xpm"
#include "frac.xpm"
#include "matrix.xpm"
#include "equation.xpm"

static LyXFunc * lyxfunc= 0;

//static FD_panel* symb_form= 0;

FD_panel  * fd_panel;
FD_delim  * fd_delim;
FD_deco   * fd_deco;
FD_space  * fd_space;
FD_matrix * fd_matrix;

int delim_code[] = {   
   '(', ')', LM_lceil,  LM_rceil,  LM_uparrow,  LM_Uparrow,
   '[', ']', LM_lfloor,  LM_rfloor,  LM_updownarrow, LM_Updownarrow,
   '{', '}',  '/', LM_backslash,  LM_downarrow,  LM_Downarrow,
   LM_langle,  LM_rangle, '|', LM_Vert, '.', 0
};

// indexes to get the left and right versions of each delimiter
// Contributed by Pablo De Napoli (pdenapo@dm.uba.ar)
int delim_lversion[] = { 0,0,2,2,4,5,
                         6,6,8,8,10,11,
                         12,12,14,15,16,17,
                         18,18,20,21,22,23 };

int delim_rversion[] = { 1,1,3,3,4,5,
                        7,7,9,9,10,11,
                        13,13,14,15,16,17,
                        19,19,20,21,22,23 };


static char const * deco_code[] = {
   "widehat", "widetilde", "overbrace", "overleftarrow", "overrightarrow", 
   "overline", "underbrace", "underline"
};


static char const * func_code[] = {
    "arccos", "arcsin", "arctan", "arg", "bmod",
    "cos", "cosh", "cot", "coth", "csc", "deg",
    "det", "dim", "exp", "gcd", "hom", "inf", "ker",
    "lg", "lim", "liminf", "limsup", "ln", "log",
    "max", "min", "sec", "sin", "sinh", "sup",
    "tan", "tanh"
};

static char h_align_str[80] = "c";

/* callbacks for form panel */
void button_cb(FL_OBJECT * ob, long data)
{   
   extern void free_symbols_form();
   switch (data)  {
    case MM_GREEK:
    case MM_VARSIZE:
    case MM_BRELATS:
    case MM_ARROW:
    case MM_BOP:
    case MM_MISC: 
      {	   
	 BitmapMenu * menu = static_cast<BitmapMenu *>(ob->u_vdata);
	 menu->Show();  
	 break;
      }
    case MM_FRAC:
       lyxfunc->Dispatch(LFUN_INSERT_MATH, "frac");
      break;
    case MM_SQRT:
       lyxfunc->Dispatch(LFUN_INSERT_MATH, "sqrt");
      break;
    case MM_DELIM:
      fl_show_form(fd_delim->delim, FL_PLACE_MOUSE, FL_FULLBORDER, _("Delimiter"));
       fl_set_form_atclose(fd_delim->delim, CancelCloseBoxCB, 0);
      break;
    case MM_DECO:
      fl_show_form(fd_deco->deco, FL_PLACE_MOUSE, FL_FULLBORDER, _("Decoration"));
       fl_set_form_atclose(fd_deco->deco, CancelCloseBoxCB, 0);
      break;
    case MM_SPACE:
      fl_show_form(fd_space->space, FL_PLACE_MOUSE, FL_FULLBORDER, _("Spacing"));
       fl_set_form_atclose(fd_space->space, CancelCloseBoxCB, 0);
      break;
    case MM_MATRIX:
      fl_show_form(fd_matrix->matrix, FL_PLACE_MOUSE, FL_FULLBORDER, _("Matrix"));
       fl_set_form_atclose(fd_matrix->matrix, CancelCloseBoxCB, 0);
      break;
    case MM_EQU:
       lyxfunc->Dispatch(LFUN_MATH_DISPLAY);
      break;
    case MM_FUNC:
      {
	  int i = fl_get_browser(fd_panel->func_browse) - 1;
	  lyxfunc->Dispatch(LFUN_INSERT_MATH, func_code[i]);
	  break;
      }
    case 100:
      free_symbols_form();
      break;
   }
}


/* callbacks for form delim */
void delim_cb(FL_OBJECT *, long data)
{
   int left= fd_delim->left->u_ldata, right= fd_delim->right->u_ldata;
   int side= (fl_get_button(fd_delim->right)!= 0);
   
   switch (data) {
    case MM_APPLY:
    case MM_OK:
      {
#ifdef HAVE_SSTREAM
	      std::ostringstream ost;
	      ost << delim_code[left] << ' ' << delim_code[right];
	      lyxfunc->Dispatch(LFUN_MATH_DELIM, ost.str().c_str());
#else
	      char s[80];
	      ostrstream ost(s, 80);
	      ost << delim_code[left] << ' ' << delim_code[right] << '\0';
	      lyxfunc->Dispatch(LFUN_MATH_DELIM, ost.str());
#endif
	      if (data == MM_APPLY) break;
      }
    case MM_CLOSE: fl_hide_form(fd_delim->delim); break;
    case 2: 
      {
	  int i = fl_get_bmtable(fd_delim->menu);
	  int button = fl_get_bmtable_numb(fd_delim->menu);
	  bool both = (button==FL_MIDDLE_MOUSE);
	  
	  if (i>= 0) {

	      if (side || (button== FL_RIGHT_MOUSE)) {
		  right = i;
	      } else {
		  left = i;
		  if (both)
		    right = delim_rversion[i];
	      }	  
	  }
	  Pixmap p1, p2;
	  p1 = fl_get_pixmap_pixmap(fd_delim->pix, &p1, &p2);
	  fl_draw_bmtable_item(fd_delim->menu, left, p1, 0, 0);
	  fl_draw_bmtable_item(fd_delim->menu, right, p1, 16, 0);
	  fl_redraw_object(fd_delim->pix);
	  
	  fd_delim->left->u_ldata = left;
	  fd_delim->right->u_ldata = right;

	  break;
      }
    case 3: break;
    case 4: break;
   }
}

/* callbacks for form matrix */
void matrix_cb(FL_OBJECT *, long data)
{
   static char v_align_c[] = "tcb";
 
   switch (data) {
    case MM_APPLY:
    case MM_OK: 
      {
	 char c = v_align_c[fl_get_choice(fd_matrix->valign)-1];
	 char const * sh = fl_get_input(fd_matrix->halign);
	 int nx = int(fl_get_slider_value(fd_matrix->columns)+0.5);
	 int ny = int(fl_get_slider_value(fd_matrix->rows)+0.5);
	 if (data == MM_OK) fl_hide_form(fd_matrix->matrix);
#ifdef HAVE_SSTREAM
	 std::ostringstream ost;
	 ost << nx << ' ' << ny << ' ' << c << sh;
	 lyxfunc->Dispatch(LFUN_INSERT_MATRIX, ost.str().c_str());
#else
	 char s[80];
	 ostrstream ost(s, 80);
	 ost << nx << ' ' << ny << ' ' << c << sh << '\0';
	 lyxfunc->Dispatch(LFUN_INSERT_MATRIX, ost.str());
#endif
	 break;
      }
    case MM_CLOSE: fl_hide_form(fd_matrix->matrix); break;
    case 2: 
      {
	 int nx = int(fl_get_slider_value(fd_matrix->columns)+0.5);
	 for (int i = 0; i < nx; ++i) h_align_str[i] = 'c';
	 //memset(h_align_str, 'c', nx);
	 h_align_str[nx] = '\0';
//	 fl_freeze_form(fd_form_main->form_main);
//	fl_addto_form(fd_form_main->form_main);

	 fl_set_input(fd_matrix->halign, h_align_str);	
	 fl_redraw_object(fd_matrix->halign); 	 
	 break;
      }
   }
}

/* callbacks for form deco */
void deco_cb(FL_OBJECT *, long data)
{
   switch (data) {
    case MM_APPLY:
    case MM_OK:
      { 
	 int i = fl_get_bmtable(fd_deco->menu);
	 lyxfunc->Dispatch(LFUN_INSERT_MATH, deco_code[i]);
	 if (data == MM_APPLY) break;
      }
    case MM_CLOSE: fl_hide_form(fd_deco->deco); break;
   }
}


/* callbacks for form space */
void space_cb(FL_OBJECT *, long data)
{
   static short sp = -1;
   extern char * latex_mathspace[];
   
   if (data >= 0 && data < 6) 
      sp = short(data);
   else
   switch (data) {
    case MM_APPLY:
    case MM_OK:
      { 
	  if (sp>= 0) 
	    lyxfunc->Dispatch(LFUN_INSERT_MATH, latex_mathspace[sp]);
	 if (data == MM_APPLY) break;
      }
    case MM_CLOSE: fl_hide_form(fd_space->space); break;
   }
}


extern "C" int align_filter(FL_OBJECT *, char const *, char const * cur, int c)
{
   int n = int(fl_get_slider_value(fd_matrix->columns)+0.5) - strlen(cur);
   return ((c == 'c'||c == 'l'||c == 'r') && n>= 0) ? FL_VALID: FL_INVALID;
}


char const ** mathed_get_pixmap_from_icon(int d)
{
   switch (d) {
    case MM_FRAC: return frac;
    case MM_SQRT: return sqrt_xpm;
    case MM_DELIM: return delim;
    case MM_MATRIX: return matrix;
    case MM_EQU: return equation; 
    case MM_DECO: return deco; 
    case MM_SPACE: return space_xpm; 
    default: return 0;
   }
}


FD_panel * create_math_panel( )
{
   fd_panel = create_form_panel();
   fd_delim = create_form_delim();
   fd_deco = create_form_deco();
   fd_space = create_form_space();
   fd_matrix = create_form_matrix();

   /* fill-in form initialization code */
   fl_set_button(fd_delim->left, 1);
   fl_set_pixmap_data(fd_delim->pix, const_cast<char**>(delim0));
   fl_set_bmtable_data(fd_delim->menu, 6, 4, delim_width, delim_height,
		       delim_bits);
   fl_set_bmtable_maxitems(fd_delim->menu, 23);
   
   fl_set_pixmap_data(fd_panel->sqrt, const_cast<char**>(sqrt_xpm));
   fl_set_pixmap_data(fd_panel->frac, const_cast<char**>(frac));
   fl_set_pixmap_data(fd_panel->delim, const_cast<char**>(delim));
   fl_set_pixmap_data(fd_panel->deco, const_cast<char**>(deco));
   fl_set_pixmap_data(fd_panel->space, const_cast<char**>(space_xpm));
   fl_set_pixmap_data(fd_panel->matrix, const_cast<char**>(matrix));
   fl_set_pixmap_data(fd_panel->equation, const_cast<char**>(equation));

   for (int i = 0; i < 32; ++i) {
       fl_add_browser_line(fd_panel->func_browse, func_code[i]);
   }
    
   fl_addto_choice(fd_matrix->valign, _("Top | Center | Bottom"));
   fl_set_choice(fd_matrix->valign, 2);
   fl_set_input(fd_matrix->halign, h_align_str);
   fl_set_input_filter(fd_matrix->halign, align_filter);
   
   fl_set_bmtable_data(fd_deco->menu, 3, 3, deco_width, deco_height,
		       deco_bits);
   fl_set_bmtable_maxitems(fd_deco->menu, 8);

   fd_delim->left->u_ldata = 0;
   fd_delim->right->u_ldata = 1;
    
   return fd_panel;
}

extern BitmapMenu * sym_menu;
extern void  create_symbol_menues(FD_panel *);


void free_symbols_form()
{
   if (fd_panel) {
      fl_hide_form(fd_panel->panel);
      fl_free_form(fd_panel->panel);
      delete sym_menu;
      free(fd_panel);
      fd_panel = 0;  
   }
}


extern "C" int AtClose_symbols_form(FL_FORM *, void *)
{
  free_symbols_form();
  return FL_IGNORE;
}


void show_symbols_form(LyXFunc * lf)
{
    lyxfunc = lf;
    if (!fd_panel) {
	fd_panel = create_math_panel();
	fl_register_raw_callback(fd_panel->panel, 
				 ButtonPressMask|KeyPressMask, C_peek_event);
	create_symbol_menues(fd_panel);	
	fl_set_form_atclose(fd_panel->panel, AtClose_symbols_form, 0);
    }
    if (fd_panel->panel->visible) {
	fl_raise_form(fd_panel->panel);
    } else {
      fl_show_form(fd_panel->panel, FL_PLACE_MOUSE,
		   FL_FULLBORDER, _("Math Panel"));
    }
}
