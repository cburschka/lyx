
#include "math_textinset.h"
#include "math_metricsinfo.h"
#include "debug.h"


MathTextInset::MathTextInset()
	: MathNestInset(1)
{}


MathInset * MathTextInset::clone() const
{
	return new MathTextInset(*this);
}


MathInset::idx_type MathTextInset::pos2row(pos_type pos) const
{
	for (pos_type r = 0, n = cache_.nargs(); r < n; ++r) 
		if (pos >= cache_.cellinfo_[r].begin_ && pos <= cache_.cellinfo_[r].end_)
			return r;
	lyxerr << "illegal row for pos " << pos << "\n";
	return 0;
}


void MathTextInset::getPos(idx_type, pos_type pos, int & x, int & y) const
{
	idx_type const i = pos2row(pos);
	pos_type const p = pos - cache_.cellinfo_[i].begin_; 
	cache_.getPos(i, p, x, y);
}


bool MathTextInset::idxUpDown(idx_type &, pos_type & pos, bool up,
	int /*targetx*/) const
{
	// try to move only one screen row up or down if possible
	idx_type i = pos2row(pos);
	MathGridInset::CellInfo const & cell1 = cache_.cellinfo_[i];
	int const x = cells_[0].pos2x(cell1.begin_, pos, cell1.glue_);
	if (up) {
		if (i == 0)
			return false;
		--i;
	} else {
		++i;
		if (i == cache_.nargs())
			return false;
	}
	MathGridInset::CellInfo const & cell2 = cache_.cellinfo_[i];
	pos = cell(0).x2pos(cell2.begin_, x, cell2.glue_);
	return true;
}


void MathTextInset::metrics(MathMetricsInfo & mi) const
{
	cell(0).metrics(mi);

	// we do our own metrics fiddling
	// delete old cache
	cache_ = MathGridInset(1, 0);

	int spaces  = 0;
	int safe    = 0;
	int curr    = 0;
	int begin   = 0;
	int safepos = 0;
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
		char const c = cell(0)[i]->getChar();
		if (c == ' ') {
			//lyxerr << "reached safe pos\n";
			// we don't count the space into the safe pos
			safe += curr;
			// we reset to this safepos if the next chunk does not fit
			safepos = i;
			++spaces;
			// restart chunk with size of the space
			curr = cell(0)[i]->width();
			continue;
		}

		if (c != '\n') {
			// This is a regular char. Go on if we either don't care for
			// the width limit or have not reached that limit.
			curr += cell(0)[i]->width();
			if (!mi.base.restrictwidth || curr + safe <= mi.base.textwidth) 
				continue;
		}

		// We passed the limit. Create a row entry.
		//lyxerr << "passed limit\n";
		cache_.appendRow();
		MathArray & ar = cache_.cell(cache_.nargs() - 1);
		MathGridInset::CellInfo & row = cache_.cellinfo_.back();
		if (c == '\n') {
			// we are here because we hit a hard newline
			row.begin_ = begin;
			row.end_   = i + 1;
			begin      = i + 1;    // next chunk starts after the newline
			spaces     = 0;
		} else if (spaces) {
			// but we had a space break before this position.
			// so retreat to this position
			//lyxerr << "... but had safe pos.\n";
			row.begin_ = begin;
			row.end_   = safepos;  // this is position of the safe space
			i          = safepos;  // i gets incremented at end of loop
			begin      = i + 1;    // next chunk starts after the space
			spaces     = 0;
		} else {
			// This item is too large and it is the only one.
			// We have no choice but to produce an overfull box.
			lyxerr << "... without safe pos\n";
			row.begin_ = begin;
			row.end_   = i + 1;
			begin      = i + 1;
		}
		ar = MathArray(cell(0).begin() + row.begin_, cell(0).begin() + row.end_);
		//lyxerr << "line: " << ar << "\n";
		// in any case, start the new row with empty boxes
		curr = 0;
		safe = 0;
	}
	// last row: put in everything else
	cache_.appendRow();
	MathArray & ar = cache_.cell(cache_.nargs() - 1);
	MathGridInset::CellInfo & row = cache_.cellinfo_.back();
	row.begin_ = begin;
	row.end_   = cell(0).size();
	ar = MathArray(cell(0).begin() + row.begin_, cell(0).begin() + row.end_);
	//lyxerr << "last line: " << ar.data() << "\n";

	// what to report?
	cache_.metrics(mi);
	dim_ = cache_.dimensions();
	//lyxerr << "outer dim: " << dim_ << endl;
}


void MathTextInset::draw(MathPainterInfo & pi, int x, int y) const
{
	cache_.draw(pi, x + 1, y);
}


void MathTextInset::drawSelection(MathPainterInfo & pi,
		idx_type idx1, pos_type pos1, idx_type idx2, pos_type pos2) const
{
	cache_.drawSelection(pi, idx1, pos1, idx2, pos2);	
}
