// -*- C++ -*-
/*
 *  File:        math_panel.h
 *  Purpose:     Mathed GUI for lyx
 *  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx>
 *  Created:     March 28, 1996
 * 
 *  Dependencies: Xlib, Xpm, XForms, Lyx
 *
 *  Copyright: (c) 1996, Alejandro Aguilar Sierra 
 *
 *   You are free to use and modify it under the terms of
 *   the GNU General Public Licence version 2 or later.
 */

#ifndef FD_math_panel_h_
#define FD_math_panel_h_

#ifdef __GNUG__
#pragma interface
#endif

#include "bmtable.h"                 

///
enum  {
   MM_GREEK, MM_ARROW, MM_BOP, MM_BRELATS, MM_VARSIZE, MM_MISC,
   MM_FRAC, MM_SQRT, MM_DELIM, MM_MATRIX, MM_EQU,
   MM_DECO, MM_SPACE, MM_DOTS, MM_FUNC,
   MM_MAX,
   MM_CLOSE = 1024,
   MM_APPLY, MM_OK
};

///
typedef FL_OBJECT* FL_OBJECTP;

/// Class to manage bitmap menu bars
class BitmapMenu {
  ///
   static BitmapMenu *active;
   ///
   friend int peek_event(FL_FORM *, void *);
 protected:
   ///
   BitmapMenu *next, *prev;
   ///
   int nb;             // Number of bitmaps
			    ///
   int i;              // Current bitmap
			    ///
   int ww;             // Border width
			    ///
   int x, y, w, h;
   ///
   FL_FORM *form;
   ///
   FL_OBJECTP *bitmap;
   ///
   FL_OBJECT *button;
 public:
   ///
   BitmapMenu(int n, FL_OBJECT* bt, BitmapMenu* prevx=0);
   ///
   ~BitmapMenu();
   ///
   FL_OBJECT* AddBitmap(int id,
			int nx, int ny, int bw, int bh,
			char* data, Bool vert=True); // Why Bool?
   ///
   void Create();
   ///
   void Hide();
   ///
   void Show();
   ///
   void Prev();
   ///
   void Next();
   ///
   int  GetIndex(FL_OBJECT* ob);
};


inline
void BitmapMenu::Prev()  {
   Hide();
   if (prev)
     prev->Show();
}

inline
void BitmapMenu::Next()  {
   Hide();
   if (next)
     next->Show();
}

#include "math_forms.h"

extern FD_panel * create_math_panel(void);

#endif /* FD_math_panel_h_ */
