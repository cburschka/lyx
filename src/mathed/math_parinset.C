
#include <config.h>

#include "math_parinset.h"
#include "math_mathmlstream.h"
#include "support/LOstream.h"


MathParInset::MathParInset(MathArray const & ar)
{
	cells_[0] = ar;
}


Dimension MathParInset::metrics(MetricsInfo & mi) const
{
	FontSetChanger dummy1(mi.base, "textnormal");
	MathGridInset::metrics(mi);
	return dim_;
}


void MathParInset::draw(PainterInfo & pi, int x, int y) const
{
	FontSetChanger dummy1(pi.base, "textnormal");
	MathGridInset::draw(pi, x, y);
}


void MathParInset::write(WriteStream & os) const
{
	for (idx_type i = 0; i < nargs(); ++i)
		os << cell(i) << "\n";
}


void MathParInset::infoize(std::ostream & os) const
{
	os << "Type: Paragraph ";
}
