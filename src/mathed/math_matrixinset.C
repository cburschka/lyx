#include <config.h>

#include "math_matrixinset.h"
#include "math_rowst.h"
#include "math_xiter.h"

extern int number_of_newlines;

MathMatrixInset::MathMatrixInset(int m, int n, short st)
	: MathParInset(st, "array", LM_OT_MATRIX), nc(m), nr(0), ws_(m),
	v_align(0), h_align(nc, 'c')
{
    row = 0;
    flag = 15;
    if (n > 0) {
	    row = new MathedRowSt(nc+1);
	MathedXIter it(this);
	for (int j = 1; j < n; ++j) it.addRow();
	nr = n;
	if (nr == 1 && nc > 1) {
	    for (int j = 0; j < nc - 1; ++j) 
	      it.Insert('T', LM_TC_TAB);
	}
    } else if (n < 0) {
	    row = new MathedRowSt(nc + 1);
	nr = 1;
    }
}


MathMatrixInset::MathMatrixInset(MathMatrixInset * mt)
	: MathParInset(mt->GetStyle(), mt->GetName(), mt->GetType()),
	  nc(mt->nc), nr(0), ws_(mt->nc), v_align(mt->v_align), h_align(mt->h_align)
{
    MathedIter it;
    it.SetData(mt->GetData());
    array = it.Copy();
    if (mt->row != 0) {
	MathedRowSt * r, * ro= 0, * mrow = mt->row;
	//mrow = mt->row; // This must be redundant...
	while (mrow) {
	    r = new MathedRowSt(nc + 1);
	    r->setNumbered(mrow->isNumbered());
	    //if (mrow->label) 
	      r->setLabel(mrow->getLabel());
	    if (!ro) 
	      row = r;
	    else
	      ro->setNext(r);
	    mrow = mrow->getNext();
	    ro = r;
	    ++nr;
	} 
    } else         
      row = 0;
    flag = mt->flag;
}


MathMatrixInset::~MathMatrixInset()
{
    MathedRowSt * r = row;
    while (r) {
	MathedRowSt * q = r->getNext();
	delete r;
	r = q;
    }
}


MathedInset * MathMatrixInset::Clone()
{
    return new MathMatrixInset(this);
}


void MathMatrixInset::SetAlign(char vv, string const & hh)
{
   v_align = vv;
   h_align = hh.substr(0, nc); // usr just h_align = hh; perhaps
}


// Check the number of tabs and crs
void MathMatrixInset::SetData(MathedArray * a)
{
    if (!a) return;
    MathedIter it(a);
    int nn = nc - 1;
    nr = 1;
    // count tabs per row
    while (it.OK()) {
	if (it.IsTab()) {
	    if (nn < 0) { 
		it.Delete();
		continue;
	    } else {
//	      it.Next();
		--nn;
	    }
	}
	if (it.IsCR()) {
	    while (nn > 0) {
		it.Insert(' ', LM_TC_TAB);
		--nn;
	    }
	    nn = nc - 1;
	    ++nr;
	}
	it.Next();
    }
    it.Reset();

    // Automatically inserts tabs around bops
    // DISABLED because it's very easy to insert tabs 
    array = a;
}


void MathMatrixInset::draw(Painter & pain, int x, int baseline)
{
    MathParInset::draw(pain, x, baseline);
}



void MathMatrixInset::Metrics()
{
    int i, hl, h = 0;
    MathedRowSt * cprow= 0;

    if (!row) {
//	lyxerr << " MIDA ";
	MathedXIter it(this);
	row = it.adjustVerticalSt();
    } 
    
    // Clean the arrays      
    MathedRowSt * cxrow = row;
    while (cxrow) {   
	for (i = 0; i <= nc; ++i) cxrow->setTab(i, 0);
	cxrow = cxrow->getNext();
    }
    
    // Basic metrics
    MathParInset::Metrics();
	    
    if (nc <= 1 && !row->getNext()) {
	row->ascent(ascent);
	row->descent(descent);
    }
    
    // Vertical positions of each row
    cxrow = row;     
    while (cxrow) {
	for (i = 0; i < nc; ++i) {
	    if (cxrow == row || ws_[i] < cxrow->getTab(i))
		    ws_[i] = cxrow->getTab(i);
	    if (cxrow->getNext() == 0 && ws_[i] == 0) ws_[i] = df_width;
	}
	
	cxrow->setBaseline((cxrow == row) ?
			   cxrow->ascent() :
	           cxrow->ascent() + cprow->descent()
			   + MATH_ROWSEP + cprow->getBaseline());
	h += cxrow->ascent() + cxrow->descent() + MATH_ROWSEP; 	
	cprow = cxrow;
	cxrow = cxrow->getNext();
    }
    
    hl = Descent();
    h -= MATH_ROWSEP;

    //  Compute vertical align
    switch (v_align) {
     case 't': ascent = row->getBaseline(); break;
     case 'b': ascent = h - hl; break;
     default:  ascent = (row->getNext()) ? h / 2: h - hl; break;
    }
    descent = h - ascent + 2;
    
    // Increase ws_[i] for 'R' columns (except the first one)
    for (i = 1; i < nc; ++i)
	if (h_align[i] == 'R')
	    ws_[i] += 10*df_width;
    // Increase ws_[i] for 'C' column
    if (h_align[0] == 'C')
	if (ws_[0] < 7*workWidth/8)
	    ws_[0] = 7*workWidth/8;

   // Adjust local tabs
    cxrow = row;
    width = MATH_COLSEP;
    while (cxrow) {   
	    int rg = MATH_COLSEP, ww, lf = 0; //, * w = cxrow->w;
	for (i = 0; i < nc; ++i) {
	    bool isvoid = false;
	    if (cxrow->getTab(i) <= 0) {
		cxrow->setTab(i, df_width);
		isvoid = true;
	    }
	    switch (h_align[i]) {
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
		if (cxrow == row)
		    lf = 0;
		else if (!cxrow->getNext())
		     lf = ws_[i] - cxrow->getTab(i);
		else
		    lf = (ws_[i] - cxrow->getTab(i))/2; 
		break;
	    }
	    ww = (isvoid) ? lf : lf + cxrow->getTab(i);
	    cxrow->setTab(i, lf + rg);
	    rg = ws_[i] - ww + MATH_COLSEP;
	    if (cxrow == row) width += ws_[i] + MATH_COLSEP;
	}
	cxrow->setBaseline(cxrow->getBaseline() - ascent);
	cxrow = cxrow->getNext();
    }
}


void MathMatrixInset::Write(ostream & os, bool fragile)
{
	if (GetType() == LM_OT_MATRIX){
		if (fragile)
			os << "\\protect";
		os << "\\begin{"
		   << name
		   << '}';
		if (v_align == 't' || v_align == 'b') {
			os << '['
			   << char(v_align)
			   << ']';
		}
		os << '{'
		   << h_align
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
