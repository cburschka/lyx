#include <config.h>

#include "math_xiter.h"
#include "math_parinset.h"
#include "math_rowst.h"
#include "array.h"
#include "mathed/support.h"
#include "debug.h"

using std::endl;

MathedXIter::MathedXIter()
	: MathedIter(), sx(0), sw(0)
{
	x = y = size = 0;
	p = 0;
	crow = 0;
}


MathParInset * MathedXIter::getPar() const
{
	return p;
}


void MathedXIter::GetPos(int & xx, int & yy) const
{ 
	if (p) 
		p->GetXY(xx, yy);
	else {
		xx = 0;
		yy = 0;
	}        
	xx += x;
	yy += y;
}


int MathedXIter::GetX() const
{ 
	int xx;
	int dummy_y;
	GetPos(xx, dummy_y);
	return xx; 
}


int MathedXIter::GetY() const
{ 
	int dummy_x;
	int yy;
	GetPos(dummy_x, yy);
	return yy; 
}


void MathedXIter::GetIncPos(int & xx, int & yy) const
{ 
	xx = x;
	yy = y; 
}


void MathedXIter::getAD(int & a, int & d) const
{ 
	if (crow) {
		a = crow->ascent();
		d = crow->descent();
	} else 
		if (p) {
			a = p->Ascent();
			d = p->Descent();
		} else {
			a = d = 0;
		}
}



void MathedXIter::Clean(int pos2)
{
	if (!array) {
		lyxerr << "Math error: Attempting to clean a void array." << endl;
		return;
	} 
	
	int pos1 = pos;
	
	if (pos2 < pos1) {
		GoBegin();
		while (pos < pos2 && OK()) {
			Next();
		}
		pos2 = pos1;
		pos1 = pos;
	}
	
	ipush();
	while (OK() && pos < pos2) {
		if (IsInset()) {
			MathedInset * inset = GetInset();
			Next();
			if (inset->GetType()!= LM_OT_MACRO_ARG)
				delete inset;
			continue;
		} 
		if (IsCR()) {
			if (crow) {
				MathedRowSt * r = crow->getNext();
				if (r) {
					crow->setNext(r->getNext());
					delete r;
				}	   
			}
		}
		Next();
	}    
	ipop();
	
	if (pos2 <= array->last()) {
		pos = pos1;
		join(pos2);
		checkTabs();
	} 
}


void MathedXIter::Merge(MathedArray * a0)
{
	if (!a0) {
		lyxerr[Debug::MATHED]
			<< "Math error: Attempting to merge a void array." << endl;
		
		return;
	}
	// All insets must be clonned
	MathedIter it(a0);
	MathedArray * a = it.Copy();

#if 0
	array->insert(array->begin() + pos,
		      a->begin(), a->end());
#else
	// make room for the data 
	split(a->last());
	array->mergeF(a, pos, a->last());
#endif
	int pos1 = pos;
	int pos2 = pos + a->last();
	
	goPosAbs(pos1);
	
	// Complete rows
	while (pos < pos2 && OK()) {
		if (IsCR()) {
			if (p && p->Permit(LMPF_ALLOW_CR)) {
				MathedRowSt * r = new MathedRowSt(ncols+1);
				if (crow) {
					r->setNext(crow->getNext());
					crow->setNext(r);
				} else {
					r->setNext(0);
				}
				crow = r;
			} else {
				Delete();
				--pos2;
			}
		}
		Next();    
	}
	pos2 = getPos();
	goPosAbs(pos1);
	checkTabs();
	goPosAbs(pos2);
	
	delete a;
}




MathedXIter::MathedXIter(MathParInset * pp)
	: p(pp) 
{ 
	x = y = 0;
	sx = sw = 0;   
	limits = false;
	if (p) 
		SetData(p);
	else {
		crow = 0;
		size = 0;
	}
}


void MathedXIter::SetData(MathParInset * pp)
{
	p = pp;
	x = y = 0;
	array = p->GetData();
	ncols = p->GetColumns();
	crow = p->getRowSt();
	if (p->Permit(LMPF_ALLOW_CR))
		flags |= MthIF_CR;
	if (p->Permit(LMPF_ALLOW_TAB))
		flags |= MthIF_Tabs;
	
	if (crow) {
		x = crow->getTab(0);
		y = crow->getBaseline();
	} 
	if (!array) {
		array = new MathedArray; // this leaks
		p->setData(array);
	}
	size = p->GetStyle();
	Reset();
}


string const MathedXIter::GetString() const
{
	string s = MathedIter::GetString();
	x += mathed_string_width(fcode(), size, s);
	return s;
}


bool MathedXIter::Next()
{  
//    lyxerr << "Ne[" << pos << "]";
	if (!OK()) return false;
	int w = 0;
//   lyxerr << "xt ";
	if (IsInset()) {
		MathedInset * px = GetInset();
		w = px->Width();
		if (px->GetType() == LM_OT_SCRIPT) {
			if (w > sw) sw = w;
			w = 0;
		} else
			sx = (px->GetLimits()) ? w : 0;
	} else {  
		byte c = GetChar();
		if (c >= ' ') {
//	  lyxerr << "WD[" << fcode << " " << size << " " << c << endl;
			w = mathed_char_width(fcode(), size, c);
		} else
			if (c == LM_TC_TAB && p) {
//	 w = p->GetTab(col + 1);
				w = (crow) ? crow->getTab(col + 1) : 0;
				//lyxerr << "WW[" << w << "]";
			} else
				if (c == LM_TC_CR && p) {
					x = 0;
					if (crow && crow->getNext()) {
						crow = crow->getNext();
						y = crow->getBaseline();
						w = crow->getTab(0);
					}
//	  lyxerr << "WW[" << col " " << row << "|" << w << "]";
				} else 
					lyxerr << "No hubo w[" << c << "]!";
	}
	if (MathedIter::Next()) {
//       lyxerr <<"LNX " << pos << endl;
//       if (sw>0 && GetChar()!= LM_TC_UP && GetChar()!= LM_TC_DOWN) {
//	   w = (sx>sw) ? 0: sw-sx;
		if ((sw > 0 || sx > 0)
		    && GetChar() != LM_TC_UP && GetChar() != LM_TC_DOWN) {
			if (sw > 0)
				w = (sx > sw) ? 0 : sw - sx;
			sx = sw = 0;
		}
		x += w;
		return true;
	} else
		return false;
}


void MathedXIter::GoBegin()
{
	Reset();
	x = y = 0;   
	sw = sx = 0;
	if (p) {
		crow = p->getRowSt();
		if (crow) {
			x = crow->getTab(0);
			y = crow->getBaseline();
		}
	}
}


void MathedXIter::GoLast()
{
	while (Next());
}


void MathedXIter::Adjust()
{
	int posx = pos;
	GoBegin();
	while (posx > pos && OK()) Next();  
}


bool MathedXIter::Prev()
{  
	if (pos == 0 || (pos == 1 && GetChar() >= ' '))
		return false;
	
	int pos2 = pos; // pos1
	GoBegin();
	do {
		ipush();
		Next();
	} while (pos<pos2);
	ipop();
	
	return (!IsCR());
}


void MathedXIter::goNextColumn()
{  
	//int rowp = row;
	int colp = col;
	while (Next() && col == colp)
		;
	
	//return (col != colp + 1 || rowp != row);
}


bool MathedXIter::Up()
{
	if (row == 0) return false;
	int xp = x;
	int rowp = row;
	int colp = col;
	GoBegin();
	while (row < rowp - 1) Next();
	while (x < xp && OK() && !IsCR()) {
		ipush();
		Next();
	}
	if (col > colp) // || (stck.col == colp && stck.x<= xp && x>xp))
		ipop();
	
	return true;
}


bool MathedXIter::Down()
{
	int xp = x;
	int colp= col;
	// int rowp = row
	
	bool res = (IsCR()) ? true : goNextCode(LM_TC_CR);
	if (res) {
		Next();
		ipush();
		while (x < xp && OK()) {
			ipush();
			Next();
		}
		if (col > colp || (stck.col == colp && stck.x <= xp && x > xp))
			ipop();
		return true;
	}
	return false;
}



void MathedXIter::addRow()
{
	if (!crow) {
		lyxerr[Debug::MATHED] << "MathErr: Attempt to insert new"
			" line in a subparagraph. " << this << endl;
		
		return;
	}
	// Create new item for the structure    
	MathedRowSt * r = new MathedRowSt(ncols + 1);
	if (crow) {
		r->setNext(crow->getNext());
		crow->setNext(r);
	} else {
		crow = r;
		r->setNext(0);
	}    
	// Fill missed tabs in current row
	while (col < ncols - 1) 
		Insert('T', LM_TC_TAB); 
	//newline
	Insert('K', LM_TC_CR);
	
	ipush();
	if (!IsCR())
		goNextCode(LM_TC_CR);
	
	// Fill missed tabs in new row
	while (col < ncols - 1) 
		Insert('T', LM_TC_TAB);
	ipop();
}


void MathedXIter::delRow()
{
	if (!crow) {
		lyxerr[Debug::MATHED] << "MathErr: Attempt to delete a line in a subparagraph." << endl;
		return;
	}
	bool line_empty = true;
	ipush();
//    while (Next()) {
	do {
		if (IsCR()) {
			break;
		} else if (!IsTab()) {
			line_empty = false;
		}
	} while (Next());
	int const p1 = getPos();
	ipop();
	
	if (line_empty) {
		
		MathedRowSt * r = crow->getNext();
		if (r) {
			crow->setNext(r->getNext());
			delete r;
		}
		join(p1);
		Delete();
	} else
		Clean(p1);
	
	checkTabs();    
}


void MathedXIter::ipush()
{ 
	MathedIter::ipush();
	stck.x = x;
	stck.y = y;
}


void MathedXIter::ipop()
{ 
	MathedIter::ipop();
	x = stck.x;
	y = stck.y;
	if (p) {
		crow = p->getRowSt();
		if (crow)
			for (int i = 0; i < row; ++i)
				crow = crow->getNext();
	}
}


void MathedXIter::fitCoord(int /*xx*/, int yy)
{
	int xo = 0;
	int yo = 0;
	
	GoBegin();
	if (p)
		p->GetXY(xo, yo);
	// first fit vertically
	while (crow && OK()) {
		if (yy >= yo + y - crow->ascent() && yy <= yo + y + crow->descent()) 
			break;
		goNextCode(LM_TC_CR);
		Next();
	}
	// now horizontally
//    while (x<xx && Next());
}


void MathedXIter::setTab(int tx, int tab)
{
	if (crow && tab <= ncols) {
		crow->setTab(tab, tx);
	} else
		lyxerr << "MathErr: No tabs allowed here" << endl;
}


void MathedXIter::subMetrics(int a, int d)
{
	if (!crow) {
		lyxerr[Debug::MATHED]
			<< "MathErr: Attempt to submetric a subparagraph." << endl;
		return;
	}
	crow->ascent(a);
	crow->descent(d);
}


// This function is not recursive, as MathPar::Metrics is
void MathedXIter::IMetrics(int pos2, int & width, int & ascent, int & descent)
{  
	byte cx;
	int x1; // ls;
	int asc = 0;
	int des = 0;
	bool limit = false;
	
	descent = ascent = width = 0;
	if (!array) return;
	if (array->empty()) return;
//    if  (pos2 > array->last) return;
	x1 = x; 
	while (pos < pos2) {
		cx = GetChar();
		if (cx >= ' ') {
			mathed_char_height(fcode(), size, cx, asc, des);
			if (asc > ascent) ascent = asc;
			if (des > descent) descent = des;
			limit = false;
		} else
			if (MathIsInset(cx)) {
				MathedInset * pp = GetInset();
				if (cx == LM_TC_UP) {
					if (!asc && p) {
						int xx;
						int yy;
						p->GetXY(xx, yy);
						static_cast<MathParInset*>(pp)->GetXY(xx, asc);
						asc = yy - asc;
					}
					asc += ((limits) ? pp->Height() + 4 : pp->Ascent());
				} else if (cx == LM_TC_DOWN) {
					if (!des && p) {
						int xx;
						int yy;
						p->GetXY(xx, yy);
						static_cast<MathParInset*>(pp)->GetXY(xx, des);
						if (des - pp->Height() < yy && !asc)
							asc = yy - (des - pp->Height());
						des -= yy;
					}
					des += (limit ? pp->Height()+4: pp->Height()-pp->Ascent()/2);
				} else {
					asc = pp->Ascent();
					des = pp->Descent();
				}
				if (asc > ascent) ascent = asc;
				if (des > descent) descent = des;
				if (cx != LM_TC_UP && cx != LM_TC_DOWN)
					limit = pp->GetLimits();
			} else if (cx == LM_TC_TAB) {
				limit = false;                   
			} else {
				lyxerr[Debug::MATHED]
					<< "Mathed Sel-Error: Unrecognized code["
					<< cx << ']' << endl;
				break;
			}
		if (pos < pos2)  Next();
	}
	width = x - x1;
}


bool MathedXIter::setNumbered(bool numb)
{  
	if (crow) {
		crow->setNumbered(numb);
		return true;
	}
	
	return false;
}


bool MathedXIter::setLabel(string const & label)
{  
	if (crow) {
		crow->setLabel(label);
		return true;
	}
	
	return false;
}


MathedRowSt * MathedXIter::adjustVerticalSt()
{
	GoBegin();
	if (!crow) {
//	lyxerr << " CRW" << ncols << " ";
		crow = new MathedRowSt(ncols + 1); // this leaks
	}
//    lyxerr<< " CRW[" << crow << "] ";
	MathedRowSt * mrow = crow;
	while (OK()) {
		if (IsCR()) {
			if (col >= ncols) ncols = col + 1; 
			MathedRowSt * r = new MathedRowSt(ncols + 1); // this leaks
//	    r->next = crow->next;
			crow->setNext(r);
			crow = r;
//	    lyxerr << " CX[" << crow << "]";
		}   
		Next();	
	}
	return mrow;
}


string const &  MathedXIter::getLabel() const
{
	return crow ? crow->getLabel() : error_label;
}


string MathedXIter::error_label = "$mathed-error$";
