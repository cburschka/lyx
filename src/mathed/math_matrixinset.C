#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_matrixinset.h"
#include "math_rowst.h"
#include "math_xiter.h"
#include "support/LOstream.h"

using std::ostream;

extern int number_of_newlines;

MathMatrixInset::MathMatrixInset(int m, int n, short st)
	: MathParInset(st, "array", LM_OT_MATRIX), nc_(m), nr_(0), ws_(m),
	  v_align_(0), h_align_(nc_, 'c'), row_(0)
{
	flag = 15;
	if (n > 0) {
		row_ = new MathedRowSt(nc_ + 1);
		MathedXIter it(this);
		for (int j = 1; j < n; ++j) it.addRow();
		nr_ = n;
		if (nr_ == 1 && nc_ > 1) {
			for (int j = 0; j < nc_ - 1; ++j) 
				it.insert('T', LM_TC_TAB);
		}
	} else if (n < 0) {
		row_ = new MathedRowSt(nc_ + 1);
		nr_ = 1;
	}
}


MathMatrixInset::MathMatrixInset(MathMatrixInset const & mt)
	: MathParInset(mt.GetStyle(), mt.GetName(), mt.GetType()),
	  nc_(mt.nc_), nr_(0), ws_(mt.nc_),
	  v_align_(mt.v_align_), h_align_(mt.h_align_)
{
	array = mt.GetData();
	if (!mt.row_.empty()) {
		MathedRowSt * ro = 0;
		MathedRowSt * mrow = mt.row_.data_;

		while (mrow) {
			MathedRowSt * r = new MathedRowSt(nc_ + 1);
			r->setNumbered(mrow->isNumbered());
			//if (mrow->label) 
			r->setLabel(mrow->getLabel());
			if (!ro) 
				row_ = r;
			else
				ro->setNext(r);
			mrow = mrow->next_;
			ro = r;
			++nr_;
		}
	} else   
		row_ = 0;
	flag = mt.flag;
}


MathMatrixInset::~MathMatrixInset()
{
	MathedRowSt * r = row_.data_;
	while (r) {
		MathedRowSt * q = r->next_;
		delete r;
		r = q;
	}
}



MathedInset * MathMatrixInset::Clone()
{
	return new MathMatrixInset(*this);
}


void MathMatrixInset::SetAlign(char vv, string const & hh)
{
	v_align_ = vv;
	h_align_ = hh.substr(0, nc_); // usr just h_align = hh; perhaps
}


// Check the number of tabs and crs
void MathMatrixInset::setData(MathedArray const & a)
{
	array = a;

	MathedIter it(&array);
	int nn = nc_ - 1;
	nr_ = 1;
	// count tabs per row
	while (it.OK()) {
		if (it.IsTab()) {
			if (nn < 0) { 
				it.Delete();
				continue;
			} else {
				// it.Next();
				--nn;
			}
		}
		if (it.IsCR()) {
			while (nn > 0) {
				it.insert(' ', LM_TC_TAB);
				--nn;
			}
			nn = nc_ - 1;
			++nr_;
		}
		it.Next();
	}
	it.Reset();
	
	// Automatically inserts tabs around bops
	// DISABLED because it's very easy to insert tabs 
}


void MathMatrixInset::draw(Painter & pain, int x, int baseline)
{
	MathParInset::draw(pain, x, baseline);
}



void MathMatrixInset::Metrics()
{
	if (row_.empty()) {
		// lyxerr << " MIDA ";
		MathedXIter it(this);
		row_ = it.adjustVerticalSt();
	} 
	
	// Clean the arrays      
	for (MathedRowContainer::iterator it = row_.begin(); it; ++it)
		for (int i = 0; i <= nc_; ++i)
			it->setTab(i, 0);
	
	// Basic metrics
	MathParInset::Metrics();

	MathedRowContainer::iterator cxrow = row_.begin();
	if (nc_ <= 1 && cxrow.is_last()) {
		cxrow->ascent(ascent);
		cxrow->descent(descent);
	}
	
	// Vertical positions of each row
	MathedRowContainer::iterator cprow = cxrow;
	int h = 0;
	for ( ; cxrow; ++cxrow) {
		for (int i = 0; i < nc_; ++i) {
			if (cxrow == row_.begin() || ws_[i] < cxrow->getTab(i))
				ws_[i] = cxrow->getTab(i);
			if (cxrow.is_last() && ws_[i] == 0)
				ws_[i] = df_width;
		}
		
		cxrow->setBaseline((cxrow == row_.begin()) ?
				   cxrow->ascent() :
				   cxrow->ascent() + cprow->descent()
				   + MATH_ROWSEP + cprow->getBaseline());
		h += cxrow->ascent() + cxrow->descent() + MATH_ROWSEP; 	
		cprow = cxrow;
	}
	
	int const hl = Descent();
	h -= MATH_ROWSEP;
	
	//  Compute vertical align
	switch (v_align_) {
	case 't':
		ascent = row_.begin()->getBaseline();
		break;
	case 'b':
		ascent = h - hl;
		break;
	default:
		ascent = (row_.begin().is_last()) ? h / 2 : h - hl;
		break;
	}
	descent = h - ascent + 2;
	
	// Increase ws_[i] for 'R' columns (except the first one)
	for (int i = 1; i < nc_; ++i)
		if (h_align_[i] == 'R')
			ws_[i] += 10 * df_width;
	// Increase ws_[i] for 'C' column
	if (h_align_[0] == 'C')
		if (ws_[0] < 7 * workWidth / 8)
			ws_[0] = 7 * workWidth / 8;
	
	// Adjust local tabs
	width = MATH_COLSEP;
	for (cxrow = row_.begin(); cxrow; ++cxrow) {   
		int rg = MATH_COLSEP;
		int lf = 0;
		for (int i = 0; i < nc_; ++i) {
			bool isvoid = false;
			if (cxrow->getTab(i) <= 0) {
				cxrow->setTab(i, df_width);
				isvoid = true;
			}
			switch (h_align_[i]) {
			case 'l':
				lf = 0;
				break;
			case 'c':
				lf = (ws_[i] - cxrow->getTab(i))/2; 
				break;
			case 'r':
			case 'R':
				lf = ws_[i] - cxrow->getTab(i);
				break;
			case 'C':
				if (cxrow == row_.begin())
					lf = 0;
				else if (cxrow.is_last())
					lf = ws_[i] - cxrow->getTab(i);
				else
					lf = (ws_[i] - cxrow->getTab(i))/2; 
				break;
			}
			int const ww = (isvoid) ? lf : lf + cxrow->getTab(i);
			cxrow->setTab(i, lf + rg);
			rg = ws_[i] - ww + MATH_COLSEP;
			if (cxrow == row_.begin())
				width += ws_[i] + MATH_COLSEP;
		}
		cxrow->setBaseline(cxrow->getBaseline() - ascent);
	}
}


void MathMatrixInset::Write(ostream & os, bool fragile)
{
	if (GetType() == LM_OT_MATRIX) {
		if (fragile)
			os << "\\protect";
		os << "\\begin{"
		   << name
		   << '}';
		if (v_align_ == 't' || v_align_ == 'b') {
			os << '['
			   << char(v_align_)
			   << ']';
		}
		os << '{'
		   << h_align_
		   << "}\n";
		++number_of_newlines;
	}
	MathParInset::Write(os, fragile);
	if (GetType() == LM_OT_MATRIX){
		os << "\n";
		if (fragile)
			os << "\\protect";
		os << "\\end{"
		   << name
		   << '}';
		++number_of_newlines;
	}
}
