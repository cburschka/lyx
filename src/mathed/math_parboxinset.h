#ifndef MATH_PARBOXINSET_H
#define MATH_PARBOXINSET_H

#include "math_nestinset.h"

class MathParboxInset : public MathNestInset {
public:
	///
	MathParboxInset();
	///
	MathParboxInset * asParboxInset() { return this; }
	///
	MathInset * clone() const;
	///
	mode_type currentMode() const { return TEXT_MODE; }
	/// get cursor position
	void getPos(idx_type idx, pos_type pos, int & x, int & y) const;
	///
	void metrics(MathMetricsInfo & mi) const;
	///
	void draw(MathPainterInfo &, int x, int y) const;
	///
	void infoize(std::ostream & os) const;
	///
	void write(WriteStream & os) const;
	///
	void setWidth(string const & width);
	///
	void setPosition(string const & pos);
private:
	/// row corresponding to given position 
	int pos2row(pos_type pos) const;
	/// width on screen
	int lyx_width_;
	/// width for TeX
	string tex_width_;
	/// htb
	char position_;
	/// cached metrics
	mutable std::vector<MathXArray::Row> rows_;
};

#endif
