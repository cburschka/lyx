
#include "dimension.h"

void Dimension::operator+=(Dimension const & dim)
{
	if (a < dim.a)
		a = dim.a;
	if (d < dim.d)
		d = dim.d;
	w += dim.w;
}
