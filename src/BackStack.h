// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1997-2000 The LyX Team.
 *
 * ====================================================== */

#ifndef BACK_STACK_H
#define BACK_STACK_H

#include <stack>

#include "LString.h"

// Created by Alejandro Aguilar Sierra, 970806

/**  Utility to get back from a reference or from a child document.
 */
class BackStack {
private:
	///
	struct BackStackItem {
		///
		BackStackItem(string const & f, int xx, int yy)
			: fname(f), x(xx), y(yy) {}
		/// Filename
		string fname;
		/// Cursor x-position
		int x;
		/// Cursor y-position
		int y;
	};
public:
	///
	void push(string f, int x, int y) {
		BackStackItem bit(f, x, y);
		stakk.push(bit);
	}
	///
	string pop(int * x, int * y) {
		BackStackItem bit = stakk.top();
		*x = bit.x;
		*y = bit.y;
		stakk.pop();
		return bit.fname;
	}
	///
	bool empty() const {
		return stakk.empty();
	}
private:
	///
	std::stack<BackStackItem> stakk;
};

#endif
