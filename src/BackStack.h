// -*- C++ -*-
/* This file is part of
* ======================================================
* 
*           LyX, The Document Processor
* 	 
*	    Copyright (C) 1997-1998 The LyX Team.
*
*======================================================*/

#ifndef _BACK_STACK_H
#define _BACK_STACK_H

#include "LString.h"

// Created by Alejandro Aguilar Sierra, 970806

/**  Utility to get back from a reference or from a child document.
 */
class BackStack {
public:
	///
	struct BackStackItem {
		///
		void set(LString f, int xx, int yy) {
			fname = f;  x = xx;  y = yy;
		}
		/// Filename
		LString fname;
		/// Cursor x-position
		int x;
		/// Cursor y-position
		int y;   
	};
	///
	BackStack(int n) : imax(n) {
		item = new BackStackItem[imax];
		i = 0;
	}
	///
	~BackStack() {
		delete[] item;
	}
	///
	void push(LString f, int x, int y) {
		if (i<imax) 
			item[i++].set(f, x, y);
	}
	///
	LString &pop(int *x, int *y) {
		if (i>0) i--;
		*x = item[i].x;
		*y = item[i].y;
		return item[i].fname;
	}
private:
	///
	BackStackItem *item;
	///
	int i;
	///
	int imax;
};

#endif
