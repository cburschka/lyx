/*
 *  File:        math_inset.C
 *  Purpose:     Implementation of insets for mathed
 *  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx> 
 *  Created:     January 1996
 *  Description: 
 *
 *  Dependencies: Xlib, XForms
 *
 *  Copyright: 1996, 1997 Alejandro Aguilar Sierra
 *
 *   Version: 0.8beta.
 *
 *   You are free to use and modify this code under the terms of
 *   the GNU General Public Licence version 2 or later.
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_iter.h"
#include "math_inset.h"
#include "symbol_def.h"
#include "lyxfont.h"
#include "mathed/support.h"
#include "Painter.h"


// Initialize some static class variables.
int MathedInset::df_asc;
int MathedInset::df_des;
int MathedInset::df_width;
int MathedInset::workWidth;


MathedInset::MathedInset(string const & nm, short ot, short st)
	: name(nm), objtype(ot), width(0), ascent(0), descent(0), size_(st) 
{}


// In a near future maybe we use a better fonts renderer
void MathedInset::drawStr(Painter & pain, short type, int siz,
			  int x, int y, string const & s)
{
	string st;
	if (MathIsBinary(type))
		for (string::const_iterator it = s.begin();
		     it != s.end(); ++it) {
			st += ' ';
			st += *it;
			st += ' ';
		}
	else
		st = s;
	
	LyXFont const mf = mathed_get_font(type, siz);
	pain.text(x, y, st, mf);
}


int MathedInset::Ascent() const
{
	return ascent;
}


int MathedInset::Descent() const
{
	return descent;
}


int MathedInset::Width() const
{
	return width;
}


int MathedInset::Height() const
{
	return ascent + descent;
}


bool MathedInset::GetLimits() const
{
	return false;
}


void MathedInset::SetLimits(bool) {}   


string const & MathedInset::GetName() const
{
	return name;
}


short MathedInset::GetType() const
{
	return objtype;
}


short MathedInset::GetStyle() const
{
	return size_;
}


void  MathedInset::SetType(short t)
{
	objtype = t;
}


void  MathedInset::SetStyle(short st)
{
	size_ = st;
}


void MathedInset::SetName(string const & n)
{
	name = n;
}


void MathedInset::defaultAscent(int da)
{
	df_asc = da;
}



void MathedInset::defaultDescent(int dd)
{
	df_des = dd;
}


void MathedInset::defaultWidth(int dw)
{
	df_width = dw;
}


short MathedInset::size() const
{
	return size_;
}


void MathedInset::size(short s)
{
	size_ = s;
}


void MathedInset::incSize()
{
	++size_;
}
