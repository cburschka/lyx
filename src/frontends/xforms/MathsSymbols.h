/**
 * \file MathsSymbols.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Alejandro Aguilar Sierra
 * \author John Levon
 */

#ifndef MATHS_SYMBOLS_H
#define MATHS_SYMBOLS_H

#include <vector>

#ifdef __GNUG__
#pragma interface
#endif

#include "bmtable.h"

class FormMaths;
 
/// Class to manage bitmap menu bars
class BitmapMenu {
	///
	friend int peek_event(FL_FORM *, void *);
	///
	typedef std::vector<FL_OBJECT *> bitmaps_type;
	///
	typedef bitmaps_type::size_type size_type;
	///
	BitmapMenu * next_;
	///
	BitmapMenu * prev_;
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
	/// Current bitmap
	size_type current_;
	///
	bitmaps_type bitmaps_;
	///
	FL_OBJECT * button;
public:
	///
	BitmapMenu(FormMaths * f, int n, FL_OBJECT * bt, BitmapMenu * prevx = 0);
	///
	~BitmapMenu();
	///
	FL_OBJECT * AddBitmap(int id,
			      int nx, int ny, int bw, int bh,
			      unsigned char const * data,
			      bool vert = true);
	///
	void create();
	///
	void hide();
	///
	void show();
	///
	void prev();
	///
	void next();
	///
	int GetIndex(FL_OBJECT * ob);
 
	/// the parent FormMaths 
	FormMaths * form_;
	///
	static BitmapMenu * active;
};


inline
void BitmapMenu::prev()
{
	hide();
	if (prev_)
		prev_->show();
}


inline
void BitmapMenu::next()
{
	hide();
	if (next_)
		next_->show();
}


#endif /* MATHS_SYMBOLS_H */
