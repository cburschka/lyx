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

/**  Utility to get back from a reference or from a child document.
     @author Alejandro Aguilar Sierra
     @version 970806
 */
class BackStack {
private:
	///
	struct BackStackItem {
		///
		BackStackItem() 
			: x(0), y(0) {}
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
	/**
	   @return returns #true# if the stack is empty, #false# otherwise.
	 */
	bool empty() const {
		return stakk.empty();
	}
private:
	///
	std::stack<BackStackItem> stakk;
};

#endif
