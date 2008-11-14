// -*- C++ -*-
/**
 * \file MathGridInfo.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_GRIDINFO_H
#define MATH_GRIDINFO_H

#include <string>


namespace lyx {


class ColInfo
{
public:
	ColInfo() : align('c'), rightline(0), leftline(false) {}
	char   align;      // column alignment
	docstring width;      // column width
	docstring special;    // special column alignment
	int    rightline;  // a line on the right?
	bool   leftline;
};


class RowInfo
{
public:
	RowInfo() : topline(false), bottomline(false) {}
	bool topline;     // horizontal line above
	int  bottomline;  // horizontal line below
};


class CellInfo
{
public:
	CellInfo()
		: multi(0), leftline(false), rightline(false),
	   topline(false), bottomline(false)
	{}

	docstring content;    // cell content
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



} // namespace lyx

#endif
