
#include "math_parboxinset.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "lyxlength.h"
#include "debug.h"

using std::vector;

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


int MathParboxInset::screenrows() const
{
	return rows_.size();
}


int MathParboxInset::pos2row(pos_type pos) const
{
	for (int r = 0, n = rows_.size(); r < n; ++r) 
		if (pos >= rows_[r].begin && pos <= rows_[r].end)
			return r;
	lyxerr << "illegal row for pos " << pos << "\n";
	return 0;
}


void MathParboxInset::getPos(idx_type idx, pos_type pos, int & x, int & y) const
{
	int const r = pos2row(pos);
	MathXArray const & ar = cells_[idx];
	x = ar.xo() + ar.pos2x(rows_[r].begin, pos, rows_[r].glue);
	y = ar.yo() + rows_[r].yo;
	// move cursor visually into empty cells ("blue rectangles");
	if (cell(0).empty())
		x += 2;
	//lyxerr << "getPos cursor at pos " << pos << " in row " << r
	//	<< "  x: " << x << " y: " << y << "\n";
}


bool MathParboxInset::idxUpDown(idx_type & idx, pos_type & pos, bool up) const
{
	// try to move only one screen row up or down if possible
	int row = pos2row(pos);
	int const x = cells_[idx].pos2x(rows_[row].begin, pos, rows_[row].glue);
	if (up) {
		if (row == 0)
			return false;
		--row;
	} else {
		++row;
		if (row == screenrows())
			return false;
	}
	pos = xcell(0).x2pos(rows_[row].begin, x, rows_[row].glue);
	return true;
}


void MathParboxInset::metrics(MathMetricsInfo & mi) const
{
	MathFontSetChanger dummy(mi.base, "textnormal");

	// we do our own metrics fiddling
	// delete old cache
	rows_.clear();

#if 0

	dim_ = xcell(0).metrics(mi);

#else

	vector<Dimension> dims;	
	xcell(0).metricsExternal(mi, dims);

	int spaces = 0;
	Dimension safe;
	Dimension curr;
	safe.clear(mi.base.font);
	curr.clear(mi.base.font);
	int begin = 0;
	int safepos = 0;
	int yo = 0;
	for (size_type i = 0, n = cell(0).size(); i < n; ++i) {
		//lyxerr << "at pos: " << i << " of " << n << " safepos: " << safepos
		//	<< " curr: " << curr << " safe: " << safe
		//	<< " spaces: " << spaces << endl;


		//   0      1      2      3       4      5      6
		// <char> <char> <char> <space> <char> <char> <char>
		// ................... <safe>
		//                      ..........................<curr>
		// ....................<safepos>

		// Special handling of spaces. We reached a safe position for breaking.
		if (cell(0)[i]->getChar() == ' ') {
			//lyxerr << "reached safe pos\n";
			// we don't count the space into the safe pos
			safe += curr;
			// we reset to this safepos if the next chunk does not fit
			safepos = i;
			++spaces;
			// restart chunk with size of the space
			curr.clear(mi.base.font);
			curr += dims[i];
			continue;
		}

		// This is a regular char. Go on if we either don't care for
		// the width limit or have not reached that limit.
		curr += dims[i];
		if (curr.w + safe.w <= lyx_width_) 
			continue;

		// We passed the limit. Create a row entry.
		//lyxerr << "passed limit\n";
		MathXArray::Row row;
		if (spaces) {
			// but we had a space break before this position.
			// so retreat to this position
			int glue  = lyx_width_ - safe.w + dims[safepos].w;
			row.dim   = safe;
			row.glue  = glue / spaces;
			row.begin = begin;
			row.end   = safepos;  // this is position of the safe space
			i         = safepos;  // i gets incremented at end of loop
			begin     = i + 1;    // next chunk starts after the space
			//lyxerr << "... but had safe pos. glue: " << row.glue << "\n";
			//lyxerr << " safe.w: " << safe.w
			//	<< "  dim.w: " << dims[safepos].w << " spaces: " << spaces << "\n";
			spaces   = 0;
		} else {
			lyxerr << "... without safe pos\n";
			// This item is too large and it is the only one.
			// We have no choice but to produce an overfull box.
			row.dim   = curr;   // safe should be 0.
			row.glue  = 0;      // does not matter
			row.begin = begin;
			row.end   = i + 1;
			begin     = i + 1;
		}
		row.yo   = yo;
		yo      += row.dim.height();
		rows_.push_back(row);
		// in any case, start the new row with empty boxes
		curr.clear(mi.base.font);
		safe.clear(mi.base.font);
	}
	// last row: put in everything else
	MathXArray::Row row;
	row.dim   = safe;
	row.dim  += curr;
	row.begin = begin;
	row.end   = cell(0).size();
	row.glue  = 0; // last line is left aligned
	row.yo    = yo;
	rows_.push_back(row);

	// what to report?
	dim_.w = lyx_width_;
	dim_.a = rows_.front().dim.a;
	dim_.d = rows_.back().dim.d + yo;
	metricsMarkers();
	xcell(0).setDim(dim_);
#endif
}


void MathParboxInset::draw(MathPainterInfo & pi, int x, int y) const
{
	MathFontSetChanger dummy(pi.base, "textnormal");
#if 0
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
	os << "Box: Parbox " << tex_width_;
}

