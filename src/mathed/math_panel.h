// -*- C++ -*-
/*
 *  File:        math_panel.h
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

#ifndef FD_math_panel_h_
#define FD_math_panel_h_

#ifdef __GNUG__
#pragma interface
#endif

#include <vector>
#include "bmtable.h"                 

///
enum SomeMathValues {
	///
	MM_GREEK,
	///
	MM_ARROW,
	///
	MM_BOP,
	///
	MM_BRELATS,
	///
	MM_VARSIZE,
	///
	MM_MISC,
	///
	MM_FRAC,
	///
	MM_SQRT,
	///
	MM_DELIM,
	///
	MM_MATRIX,
	///
	MM_EQU,
	///
	MM_DECO,
	///
	MM_SPACE,
	///
	MM_DOTS,
	///
	MM_FUNC,
	///
	MM_MAX,
	///
	MM_CLOSE = 1024,
	///
	MM_APPLY,
	///
	MM_OK
};


/// Class to manage bitmap menu bars
class BitmapMenu {
	///
	static BitmapMenu * active;
	///
	friend int peek_event(FL_FORM *, void *);
	///
	typedef std::vector<FL_OBJECT *>   bitmaps_type;
	///
	typedef bitmaps_type::size_type    size_type;
	///
	BitmapMenu * next;
	///
	BitmapMenu * prev;
	/// Current bitmap
	size_type current_;
	/// Border width
	int ww;
	///
	int x;
	///
	int y;
	///
	int w;
	///
	int h;
	///
	FL_FORM * form;
	///
	bitmaps_type bitmaps_;
	///
	FL_OBJECT * button;
public:
	///
	BitmapMenu(int n, FL_OBJECT * bt, BitmapMenu * prevx= 0);
	///
	~BitmapMenu();
	///
	FL_OBJECT * AddBitmap(int id,
			      int nx, int ny, int bw, int bh,
			      unsigned char const * data,
			      Bool vert = True); // Why Bool?
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
	int GetIndex(FL_OBJECT * ob);
};


/// This is just a wrapper around peek_event()
extern "C" int C_peek_event(FL_FORM * form, void * ptr);


inline
void BitmapMenu::Prev()
{
	Hide();
	if (prev)
		prev->Show();
}


inline
void BitmapMenu::Next()
{
	Hide();
	if (next)
		next->Show();
}


#include "math_forms.h"

///
extern FD_panel * create_math_panel(void);

#endif /* FD_math_panel_h_ */
