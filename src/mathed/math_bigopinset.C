#include "math_bigopinset.h"
#include "Painter.h"
#include "mathed/support.h"
#include "support/LOstream.h"


using std::ostream;

MathBigopInset::MathBigopInset(string const & name, int id)
	: MathInset(0, name), sym_(id)
{}


MathInset * MathBigopInset::clone() const
{
	return new MathBigopInset(*this);
}


void MathBigopInset::write(ostream & os, bool /* fragile */) const
{
	//bool f = sym_ != LM_int && sym_ != LM_oint && size() == LM_ST_DISPLAY;
	os << '\\' << name();
}


void MathBigopInset::writeNormal(ostream & os) const
{
	os << "[bigop " << name() << "] ";
}


void MathBigopInset::metrics(MathStyles st)
{
	//cerr << "\nBigopDraw\n";
	size(st);
	
	if (sym_ < 256 || sym_ == LM_oint) {
		ssym_ = string();
		ssym_ += (sym_ == LM_oint) ? LM_int : sym_;
		code_ = LM_TC_BSYM;
	} else {
		ssym_ = name();
		code_ = LM_TC_TEXTRM;
	}

	mathed_string_dim(code_, size(), ssym_, ascent_, descent_, width_);
	if (sym_ == LM_oint)
		width_ += 2;
}


void MathBigopInset::draw(Painter & pain, int x, int y)
{  
	xo(x);
	yo(y);

	drawStr(pain, code_, size_, x, y, ssym_);

	if (sym_ == LM_oint) {
		int xx = x - 1;
		int yy = y - (ascent_ - descent_) / 2;
		pain.arc(xx, yy, width_, width_, 0, 360 * 64, LColor::mathline);
	}
}
