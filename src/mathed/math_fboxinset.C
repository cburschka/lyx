#include <config.h>

#include "math_fboxinset.h"
#include "math_support.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "math_parser.h"
#include "frontends/Painter.h"

using std::auto_ptr;


MathFboxInset::MathFboxInset(latexkeys const * key)
	: MathNestInset(1), key_(key)
{}


auto_ptr<InsetBase> MathFboxInset::clone() const
{
	return auto_ptr<InsetBase>(new MathFboxInset(*this));
}


MathInset::mode_type MathFboxInset::currentMode() const
{
	if (key_->name == "fbox")
		return TEXT_MODE;
	return MATH_MODE;
}


void MathFboxInset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	if (key_->name == "fbox") {
		FontSetChanger dummy(mi.base, "textnormal");
		cell(0).metrics(mi, dim_);
	} else {
		cell(0).metrics(mi, dim_);
	}
	metricsMarkers(5); // 5 pixels margin
	dim = dim_;
}


void MathFboxInset::draw(PainterInfo & pi, int x, int y) const
{
	pi.pain.rectangle(x + 1, y - dim_.ascent() + 1,
		dim_.width() - 2, dim_.height() - 2, LColor::foreground);
	if (key_->name == "fbox") {
		FontSetChanger dummy(pi.base, "textnormal");
		cell(0).draw(pi, x + 5, y);
	} else {
		cell(0).draw(pi, x + 5, y);
	}
}


void MathFboxInset::write(WriteStream & os) const
{
	os << '\\' << key_->name << '{' << cell(0) << '}';
}


void MathFboxInset::normalize(NormalStream & os) const
{
	os << '[' << key_->name << ' ' << cell(0) << ']';
}
