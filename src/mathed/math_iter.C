/*
 *  File:        math_inset.C
 *  Purpose:     Implementation of insets for mathed
 *  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx>
 *  Created:     January 1996
 *  Description:
 *
 *  Dependencies: Xlib, XForms
 *
 *  Copyright: 1996, Alejandro Aguilar Sierra
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
#include "array.h"
#include "math_inset.h"
#include "symbol_def.h"
#include "support/lstrings.h"
#include "debug.h"
#include "mathed/support.h"

using std::endl;

const int SizeInset = sizeof(char*) + 2;

//extern int mathed_char_width(short type, int style, byte c);
//extern int mathed_string_width(short type, int style, string const & s);
//extern int mathed_char_height(short, int, byte, int &, int &);


MathedIter::MathedIter()
	: flags(0), fcode_(0), pos(0), row(0), col(0), ncols(0), array(0)
{}


MathedArray * MathedIter::GetData() const
{
	return array;
}


short MathedIter::fcode() const
{
	return fcode_; 
}


void MathedIter::fcode(short c) const
{
	fcode_ = c; 
}


int MathedIter::Empty() const
{
	return array->last() <= 1;
}


int MathedIter::OK() const
{
	return array && (pos < array->last());
}


void MathedIter::Reset()
{
	if (array->last() > 0 && MathIsFont((*array)[0])) {
		fcode((*array)[0]);
		pos   = 1;
	} else {
		fcode(-1);
		pos   = 0;
	}
	col = 0;
	row = 0;
}


byte MathedIter::GetChar() const
{
	if (IsFont()) {
		fcode((*array)[pos]);
		++pos;
	}
	return (*array)[pos];
}


string const MathedIter::GetString() const
{
	if (IsFont()) {
		fcode((*array)[++pos]);
		++pos;
	}

	string s;
	for (; (*array)[pos] >= ' ' && pos < array->last(); ++pos)
		s += (*array)[pos];

	return s;
}


MathedInset * MathedIter::GetInset() const
{
	if (IsInset()) {
		MathedInset * p;
		array->raw_pointer_copy(&p, pos + 1);
		return p;
	} else {
		lyxerr << "Math Error: This is not an inset["
		       << (*array)[pos] << "]" << endl;
		return 0;
	}
}


// An active math inset MUST be derived from MathParInset because it
// must have at least one paragraph to edit
MathParInset * MathedIter::GetActiveInset() const
{
	if (IsActive()) 
		return reinterpret_cast<MathParInset*>(GetInset());

	lyxerr << "Math Error: This is not an active inset" << endl;
	return 0;
}


bool MathedIter::Next()
{
	if (!OK())
		return false;

	if ((*array)[pos] < ' ') {
		fcode(-1);
		if (IsTab())
			++col;
		if (IsCR()) {
			col = 0;
			++row;
		}
	}

	if (IsInset())
		pos += sizeof(char*) + 2;
	else
		++pos;

	if (IsFont()) 
		fcode((*array)[pos++]);

	return true;
}


bool MathedIter::goNextCode(MathedTextCodes code)
{
	while (Next()) {
		if ((*array)[pos] == code)
			return true;
	}

	return false;
}


void MathedIter::goPosAbs(int p)
{
	Reset();
	while (pos < p && Next())
		;
}


void MathedIter::insert(byte c, MathedTextCodes t)
{
	if (c < ' ')
		return;

	if (t == LM_TC_TAB && col >= ncols - 1)
		return;

	// Never more than one space // array->bf[pos-1] gives error from purify:
	// Reading 1 byte from 0x47b857 in the heap.
	//
	// Address 0x47b857 is 1 byte before start of malloc'd block at
	// 0x47b858 of 16 bytes.

	if (c == ' ' && ((*array)[pos] == ' ' || (*array)[pos - 1] == ' '))
		return;

	if (IsFont() && (*array)[pos] == t) {
		fcode(t);
		++pos;
	} else {
		if (t != fcode() && pos > 0 && MathIsFont((*array)[pos - 1])) {
			--pos;
			int k = pos - 1;
			for (; k >= 0 && (*array)[k] >= ' '; --k)
				;
			fcode( (k >= 0 && MathIsFont((*array)[k])) ? (*array)[k] : -1 );
		}
	}

	short const f = ((*array)[pos] < ' ') ? 0 : fcode();
	int shift = (t == fcode()) ? 1 : ((f) ? 3 : 2);

	if (t == LM_TC_TAB || t == LM_TC_CR) {
		--shift;
		c = t;
		if (t == LM_TC_CR) {
			++row;
			col = 0;
		} else
			++col;
	}

	if (pos < array->last())
		array->move(pos, shift);
	else {
		array->need_size(array->last() + shift);
		array->last(array->last() + shift);
		(*array)[array->last()] = '\0';
	}

	if (t != fcode()) {
		if (f)
			(*array)[pos + shift - 1] = fcode();

		if (c >= ' ') {
			(*array)[pos++] = t;
			fcode(t);
		} else 
			fcode(0);
	}

	(*array)[pos++] = c;
}


// Prepare to insert a non-char object
void MathedIter::split(int shift)
{
	if (pos < array->last()) {

		bool fg = false;
		if ((*array)[pos] >= ' ') {
			if (pos> 0 && MathIsFont((*array)[pos - 1]))
				--pos;
			else {
				fg = true;
				++shift;
			}
		}

		array->move(pos, shift);

		if (fg)
			(*array)[pos + shift - 1] = fcode();

	} else {

		array->need_size(array->last() + shift);
		array->last(array->last() + shift);
	}

	(*array)[array->last()] = '\0';
}


// I assume that both pos and pos2 are legal positions
void MathedIter::join(int pos2)
{
	if (!OK() || pos2 <= pos)
		return;

	short f = fcode();
	if (pos > 0 && (*array)[pos] >= ' ' && MathIsFont((*array)[pos - 1]))
		--pos;	

	if (MathIsFont((*array)[pos2 - 1]))
		--pos2;

	if ((*array)[pos2] >= ' ') {
		for (int p = pos2; p > 0; --p) {
			if (MathIsFont((*array)[p])) {
				f = (*array)[p];
				break;
			}
		}
		(*array)[pos++] = f;
	}

	array->move(pos2, pos - pos2);
}


void MathedIter::insertInset(MathedInset * p, int type)
{
#if 0
	if (!MathIsInset(type))
		type = LM_TC_INSET;

	array->insertInset(pos, p, type);
	++pos;
	fcode(-1);
#else
	int const shift = SizeInset;

	if (!MathIsInset(type))
		type = LM_TC_INSET;

	split(shift);

	(*array)[pos] = type;
	array->raw_pointer_insert(p, pos + 1);
	pos += SizeInset;
	(*array)[pos - 1] = type;
	(*array)[array->last()] = '\0';
	fcode(-1);
#endif
}


bool MathedIter::Delete()
{
	if (!OK())
		return false;

	int shift = 0;
	byte const c = GetChar();
	if (c >= ' ') {

		if (MathIsFont((*array)[pos - 1]) && (*array)[pos + 1] < ' ') {
			shift = 2;
			pos--;
			int i = pos - 1;
			for (; i > 0 && !MathIsFont((*array)[i]); --i)
				;
			if (i > 0 && MathIsFont((*array)[i]))
			fcode((*array)[i]);
		} else
			shift = 1;

	} else {

		if (MathIsInset((*array)[pos]))
			shift = sizeof(char*) + 2;
		else if (c == LM_TC_TAB || c == LM_TC_CR) {
			++shift;
		//	 lyxerr <<"Es un tab.";
		} else {
			lyxerr << "Math Warning: expected inset." << endl;
		}

	}

	if (shift != 0) {
		array->move(pos + shift, -shift);
		if (pos >= array->last())
			pos = (array->last() > 0) ? array->last() : 0;
		return true;
	} else
		return false;
}

// Check consistency of tabs and crs
void MathedIter::checkTabs()
{
	ipush();

	// MathedIter:Reset();
	while (OK()) {
		if ((IsTab() && col >= ncols - 1) || (IsCR() && !(MthIF_CR & flags))) {
			Delete();
			continue;
		}

		if (IsCR() && col < ncols - 2) 
			insert(' ', LM_TC_TAB);

		MathedIter::Next();
	}

	if (col < ncols - 2)
		insert(' ', LM_TC_TAB);
	
	ipop();
}


//  Try to adjust tabs in the expected place, as used in eqnarrays
//  Rules:
//   - If there are a relation operator, put tabs around it
//   - If tehre are not a relation operator, put everything in the
//     3rd column.
void MathedIter::adjustTabs()
{}


bool MathedIter::IsInset() const
{
	return MathIsInset((*array)[pos]);
}


bool MathedIter::IsActive() const
{
	return MathIsActive((*array)[pos]);
}


bool MathedIter::IsFont() const
{
	return MathIsFont((*array)[pos]);
}


bool MathedIter::IsScript() const
{
	return MathIsScript((*array)[pos]);
}


bool MathedIter::IsTab() const
{
	return ((*array)[pos] == LM_TC_TAB);
}


bool MathedIter::IsCR() const
{
	return ((*array)[pos] == LM_TC_CR);
}


MathedIter::MathedIter(MathedArray * d)
	: array(d)
{
	pos = 0;
	row = 0;
	col = 0;
	fcode( (array && IsFont()) ? (*array)[0] : 0 );
}


void MathedIter::ipush()
{
	stck.fcode = fcode();
	stck.pos = pos;
	stck.row = row;
	stck.col = col;
}


void MathedIter::ipop()
{
	fcode(stck.fcode);
	pos = stck.pos;
	row = stck.row;
	col = stck.col;
}
