// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright (C) 1997-1999 The LyX Team.
 *
 * ======================================================*/

#ifndef BACK_STACK_H
#define BACK_STACK_H

#include "LString.h"

// Created by Alejandro Aguilar Sierra, 970806

/**  Utility to get back from a reference or from a child document.
 */
class BackStack {
public:
	///
	struct BackStackItem {
		///
		void set(string f, int xx, int yy) {
			fname = f;  x = xx;  y = yy;
		}
		/// Filename
		string fname;
		/// Cursor x-position
		int x;
		/// Cursor y-position
		int y;
	};
	///
	BackStack(int n) : item(new BackStackItem[n]) , imax(n), i(0) {}
	///
	~BackStack() {
		delete[] item;
	}
	///
	void push(string f, int x, int y) {
		if (i < imax) 
			item[i++].set(f, x, y);
	}
	///
	string & pop(int *x, int *y) {
		if (i > 0) i--;
		*x = item[i].x;
		*y = item[i].y;
		return item[i].fname;
	}
	///
	bool empty() const {
		return i == 0;
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
