#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "xarray.h"
#include "math_inset.h"
#include "mathed/support.h"
#include "math_defs.h"
#include "Painter.h"

using std::max;
using std::min;


MathXArray::MathXArray()
	: width_(0), ascent_(0), descent_(0), xo_(0), yo_(0), style_(LM_ST_TEXT)
{}


void MathXArray::Metrics(MathStyles st)
{
	if (data_.empty()) {
		mathed_char_dim(LM_TC_VAR, st, 'I', ascent_, descent_, width_); 
		return;
	}
	
	ascent_  = 0;
	descent_ = 0;
	width_   = 0;
	style_    = st;

	for (int pos = 0; pos < data_.size(); data_.next(pos)) {
		int asc;
		int des;
		int wid;
		MathInset * p = data_.nextInset(pos);
		if (p) {
			p->Metrics(st);
			asc = p->ascent();
			des = p->descent();
			wid = p->width();
		} else {
			char cx = data_.GetChar(pos); 
			MathTextCodes fc = data_.GetCode(pos); 
			mathed_char_dim(fc, style_, cx, asc, des, wid);
		}
		ascent_  = max(ascent_, asc);
		descent_ = max(descent_, des);
		width_   += wid;
	}
}


void MathXArray::draw(Painter & pain, int x, int y)
{
	xo_ = x;
	yo_ = y;

	if (data_.empty()) {
		pain.rectangle(x, y - ascent_, width_, height(), LColor::mathline);
		return;
	}

	for (int pos = 0; pos < data_.size(); data_.next(pos)) {
		MathInset * p = data_.nextInset(pos);
		if (p) {
			p->draw(pain, x, y);
			x += p->width();
		} else {
			char cx = data_.GetChar(pos);
			MathTextCodes fc = data_.GetCode(pos);
			string s;
			s += cx;
			drawStr(pain, fc, style_, x, y, s);
			x += mathed_char_width(fc, style_, cx);
		}
	}
}


int MathXArray::pos2x(int targetpos) const
{
	int x = 0;
	targetpos = min(targetpos, data_.size());
	for (int pos = 0; pos < targetpos; data_.next(pos)) 
		x += width(pos);
	return x;
}


int MathXArray::x2pos(int targetx) const
{
	int pos   = 0;
	int lastx = 0;
	int currx = 0;
	while (currx < targetx && pos < data_.size()) {
		lastx = currx;
		currx += width(pos);
 		data_.next(pos);
	}
	if (abs(lastx - targetx) < abs(currx - targetx))
		data_.prev(pos);
	return pos;
}

int MathXArray::width(int pos) const
{
	if (pos >= data_.size())
		return 0;

	if (data_.isInset(pos)) 
		return data_.nextInset(pos)->width();
	else 
		return mathed_char_width(data_.GetCode(pos), style_, data_.GetChar(pos));
}

std::ostream & operator<<(std::ostream & os, MathXArray const & ar)
{
	os << ar.data_;
	return os;
}

