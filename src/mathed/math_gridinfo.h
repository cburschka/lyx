// -*- C++ -*-
/**
 * \file math_gridinfo.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_GRIDINFO_H
#define MATH_GRIDINFO_H


struct ColInfo
{
	ColInfo() : align('c'), rightline(0), leftline(false) {}
	char   align;      // column alignment
	string width;      // column width
	string special;    // special column alignment
	int    rightline;  // a line on the right?
	bool   leftline;
};


struct RowInfo
{
	RowInfo() : topline(false), bottomline(false) {}
	bool topline;     // horizontal line above
	int  bottomline;  // horizontal line below
};


struct CellInfo
{
	CellInfo()
		: multi(0), leftline(false), rightline(false),
	   topline(false), bottomline(false)
	{}

	string content;    // cell content
	int multi;         // multicolumn flag
	char align;        // cell alignment
	bool leftline;     // do we have a line on the left?
	bool rightline;	   // do we have a line on the right?
	bool topline;	     // do we have a line above?
	bool bottomline;   // do we have a line below?
};


inline char const * verbose_align(char c)
{
	return c == 'c' ? "center" : c == 'r' ? "right" : c == 'l' ? "left" : "none";
}


#endif
