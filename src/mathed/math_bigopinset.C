#include "math_bigopinset.h"
#include "Painter.h"
#include "mathed/support.h"
#include "support/LOstream.h"


using std::ostream;

MathBigopInset::MathBigopInset(string const & name, int id)
	: MathUpDownInset(false, false), sym_(id), limits_(0)
{
	SetName(name);
}


MathInset * MathBigopInset::clone() const
{
	return new MathBigopInset(*this);
}


int MathBigopInset::limits() const 
{
	return limits_;	
} 


void MathBigopInset::limits(int limits) 
{  
	limits_ = limits;
}


bool MathBigopInset::hasLimits() const
{
	return limits_ == 1 || (limits_ == 0 && size() == LM_ST_DISPLAY);
}


void MathBigopInset::Write(ostream & os, bool fragile) const
{
	//bool f = sym_ != LM_int && sym_ != LM_oint && size() == LM_ST_DISPLAY;
	os << '\\' << name();
	MathUpDownInset::Write(os, fragile);
}


void MathBigopInset::WriteNormal(ostream & os) const
{
	os << "[bigop " << name() << "] ";
}


void MathBigopInset::Metrics(MathStyles st, int, int)
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

	int wid;
	mathed_string_dim(code_, size(), ssym_, ascent_, descent_, wid);
	if (sym_ == LM_oint)
		wid += 2;
	//cerr << "  asc: " << ascent_ << " des: " << descent_
	//	<< " wid: " << wid << "\n";
	//cerr << "  hasLimits: " << hasLimits() << " up: "
	//	<< up() << " down: " << down() << "\n";
	
	width_ = wid;

	if (hasLimits()) {
		xcell(0).Metrics(st);
		xcell(1).Metrics(st);
		//cerr << "  0: ascent_: " << xcell(0).ascent() << " descent_: " <<
		//	xcell(0).descent() << " width_: " << xcell(0).width() << "\n";
		//cerr << "  1: ascent_: " << xcell(1).ascent() << " descent_: " <<
		//	xcell(1).descent() << " width_: " << xcell(1).width() << "\n";
		if (up()) {
			ascent_  += xcell(0).height() + 1;
			width_   = std::max(width_, xcell(0).width());
			dy0_     = - (ascent_ - xcell(0).ascent());
		}
		if (down()) {
			descent_ += xcell(1).height() + 1;
			width_   = std::max(width_, xcell(1).width());
			dy1_     = descent_ - xcell(1).descent();
		}
		dxx_  = (width_ - wid) / 2;
		dx0_  = (width_ - xcell(0).width()) / 2;
		dx1_  = (width_ - xcell(1).width()) / 2;
		//cerr << "  ascent_: " << ascent_ << " descent_: "
		//	<< descent_ << " width_: " << width_ << "\n";
		//cerr << "  dx0_: " << dx0_ << " dx1_: " << dx1_
		//	<< " dxx_: " << dxx_ << "\n";
		//cerr << "  dy0_: " << dy0_ << " dy1_: " << dy1_
		//	<< "\n";
	} else {
		MathUpDownInset::Metrics(st, ascent_, descent_);
		width_   += wid;
		dx0_     = wid;
		dx1_     = wid;
		dxx_     = 0;
	}
}


void MathBigopInset::draw(Painter & pain, int x, int y)
{  
	xo(x);
	yo(y);

	pain.text(x + dxx_, y, ssym_, mathed_get_font(code_, size()));

	if (up())
		xcell(0).draw(pain, x + dx0_, y + dy0_);
	if (down())
		xcell(1).draw(pain, x + dx1_, y + dy1_);

	if (sym_ == LM_oint) {
		int xx = x - 1;
		int yy = y - (ascent_ - descent_) / 2;
		pain.arc(xx, yy, width_, width_, 0, 360 * 64, LColor::mathline);
	}
}


