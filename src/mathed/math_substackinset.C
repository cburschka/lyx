#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_substackinset.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"


MathSubstackInset::MathSubstackInset()
	: MathGridInset(1, 1)
{}


MathInset * MathSubstackInset::clone() const
{
	return new MathSubstackInset(*this);
}


void MathSubstackInset::metrics(MathMetricsInfo & mi) const
{
	if (mi.base.style == LM_ST_DISPLAY) {
		MathStyleChanger dummy(mi.base, LM_ST_TEXT);
		MathGridInset::metrics(mi);
	} else {
		MathGridInset::metrics(mi);
	}
}


void MathSubstackInset::write(WriteStream & os) const
{
	os << "\\substack{";
	MathGridInset::write(os);
	os << "}\n";
}


void MathSubstackInset::normalize(NormalStream & os) const
{
	os << "[substack ";
	MathGridInset::normalize(os);
	os << "]";
}


void MathSubstackInset::maplize(MapleStream & os) const
{
	os << "substack(";
	MathGridInset::maplize(os);
	os << ")";
}
