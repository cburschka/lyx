#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_xyarrowinset.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"


MathXYArrowInset::MathXYArrowInset()
	: MathNestInset(1)
{}


MathInset * MathXYArrowInset::clone() const
{
	return new MathXYArrowInset(*this);
}


void MathXYArrowInset::metrics(MathMetricsInfo const & mi) const
{
	MathNestInset::metrics(mi);
}


void MathXYArrowInset::write(WriteStream & os) const
{
	os << "\\ar{";
	MathNestInset::write(os);
	os << "}\n";
}


void MathXYArrowInset::normalize(NormalStream & os) const
{
	os << "[xyarrow ";
	MathNestInset::normalize(os);
	os << "]";
}
