
#include <config.h>
#include "dimension.h"
#include "math_support.h"

#include <iostream>


void Dimension::operator+=(Dimension const & dim)
{
	if (a < dim.a)
		a = dim.a;
	if (d < dim.d)
		d = dim.d;
	w += dim.w;
}


std::ostream & operator<<(std::ostream & os, Dimension const & dim)
{
	os << " (" << dim.w << 'x' << dim.a << '-' << dim.d << ") ";
	return os;
}


void Dimension::clear(LyXFont const & font)
{
	math_font_max_dim(font, a, d);
	w = 0;
}
