
#include "math_parboxinset.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "lyxlength.h"
#include "debug.h"


MathParboxInset::MathParboxInset()
	: MathNestInset(1), lyx_width_(0), tex_width_("0mm"),
	  position_('c')
{
	lyxerr << "constructing MathParboxInset\n";
}


MathInset * MathParboxInset::clone() const
{
	return new MathParboxInset(*this);
}


void MathParboxInset::setPosition(string const & p)
{
	position_ = p.size() > 0 ? p[0] : 'c';
}


void MathParboxInset::setWidth(string const & w)
{
	tex_width_ = w;
	lyx_width_ = LyXLength(w).inBP();
	lyxerr << "setting " << w << " to " << lyx_width_ << " pixel\n";
}


void MathParboxInset::metrics(MathMetricsInfo & mi) const
{
	MathFontSetChanger dummy(mi.base, "textnormal");

	// we do our own metrics fiddling
	// delete old cache
	rows_.clear();

#if 1

	xcell(0).metrics(mi);
	width_ = xcell(0).width();
	ascent_ = xcell(0).ascent();
	descent_ = xcell(0).descent();

#else

	xcell(0).metricsExternal(mi, rows_);

	int spaces = 0;
	Dimension safe(0, 0, 0);
	Dimension curr(0, 0, 0);
	int safepos = 0;
	int yo = 0;
	for (size_type i = 0, n = cell(0).size(); i != n; ++i) {
		// Special handling of spaces. We reached a safe position for breaking.
		if (cell(0)[i]->getChar() == ' ') {
			safe += curr;
			safepos = i + 1;
			++spaces;
			// restart chunk
			curr = Dimension(0, 0, 0);
			continue;
		}

		// This is a regular item. Go on if we either don't care for
		// the width limit or have not reached that limit.
		curr += rows_[i].dim;
		if (curr.w + safe.w <= lyx_width_) 
			continue;

		// We passed the limit. Create a row entry.
		MathXArray::Row row;
		if (spaces) {
			// but we had a space break before this position.
			row.dim  = safe;
			row.glue = (lyx_width_ - safe.w) / spaces;
			row.end  = safepos;
			i        = safepos;
			spaces   = 0;
		} else {
			// This item is too large and it is the only one.
			// We have no choice but to produce an overfull box.
			row.dim  = curr;   // safe should be 0.
			row.glue = 0;      // does not matter
			row.end  = i + 1;
		}
		yo      += rows_[i].dim.height();
		row.yo   = yo;
		rows_.push_back(row);
	}
	// last row:
	MathXArray::Row row;
	row.dim  = safe;
	row.dim += curr;
	row.end  = cell(0).size();
	row.glue = spaces ? (lyx_width_ - row.dim.w) / spaces : 0;
	yo      += row.dim.height();
	row.yo   = yo;
	rows_.push_back(row);

	// what to report?
	ascent_  = xcell(0).ascent();
	descent_ = xcell(0).descent() + 1;
	width_   = xcell(0).width()   + 2;
#endif
}


void MathParboxInset::draw(MathPainterInfo & pi, int x, int y) const
{
	MathFontSetChanger dummy(pi.base, "textnormal");
#if 1
	xcell(0).draw(pi, x + 1, y);
#else
	xcell(0).drawExternal(pi, x + 1, y, rows_);
#endif
	drawMarkers(pi, x, y);
}


void MathParboxInset::write(WriteStream & os) const
{
	os << "\\parbox";
	if (position_ != 'c')
		os << '[' << position_ << ']';
	os << '{' << tex_width_ << "}{" << cell(0) << '}';
}


void MathParboxInset::infoize(std::ostream & os) const
{
	os << "Box: Parbox " << tex_width_ << ' ';
}

