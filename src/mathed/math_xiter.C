#include <config.h>

#include "math_xiter.h"
#include "math_parinset.h"
#include "math_rowst.h"
#include "array.h"
#include "mathed/support.h"
#include "debug.h"

using std::endl;

string MathedXIter::error_label = "$mathed-error$";

MathedXIter::MathedXIter()
	: MathedIter(), size_(0), x_(0), y_(0), p_(0), sx_(0), sw_(0),
	  crow_(0)
{
	// should limits_ be initialized?
}


MathedXIter::MathedXIter(MathParInset * pp)
	: x_(0), y_(0), p_(pp), sx_(0), sw_(0), limits_(false)
{ 
	if (p_) 
		SetData(p_);
	else {
		crow_ = 0;
		size_ = 0;
	}
}



MathParInset * MathedXIter::getPar() const
{
	return p_;
}


void MathedXIter::GetPos(int & xx, int & yy) const
{ 
	if (p_) 
		p_->GetXY(xx, yy);
	else {
		xx = 0;
		yy = 0;
	}        
	xx += x_;
	yy += y_;
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
	xx = x_;
	yy = y_; 
}


void MathedXIter::getAD(int & a, int & d) const
{ 
	if (crow_) {
		a = crow_->ascent();
		d = crow_->descent();
	} else 
		if (p_) {
			a = p_->Ascent();
			d = p_->Descent();
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
			if (crow_) {
				MathedRowSt * r = crow_->getNext();
				if (r) {
					crow_->setNext(r->getNext());
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
			if (p_ && p_->Permit(LMPF_ALLOW_CR)) {
				MathedRowSt * r = new MathedRowSt(ncols + 1);
				if (crow_) {
					r->setNext(crow_->getNext());
					crow_->setNext(r);
				} else {
					r->setNext(0);
				}
				crow_ = r;
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


void MathedXIter::SetData(MathParInset * pp)
{
	p_ = pp;
	x_ = y_ = 0;
	array = p_->GetData();
	ncols = p_->GetColumns();
	crow_ = p_->getRowSt();
	if (p_->Permit(LMPF_ALLOW_CR))
		flags |= MthIF_CR;
	if (p_->Permit(LMPF_ALLOW_TAB))
		flags |= MthIF_Tabs;
	
	if (crow_) {
		x_ = crow_->getTab(0);
		y_ = crow_->getBaseline();
	} 
	if (!array) {
		array = new MathedArray; // this leaks
		p_->setData(array);
	}
	size_ = p_->GetStyle();
	Reset();
}


string const MathedXIter::GetString() const
{
	string const s = MathedIter::GetString();
	x_ += mathed_string_width(fcode(), size_, s);
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
			if (w > sw_) sw_ = w;
			w = 0;
		} else
			sx_ = (px->GetLimits()) ? w : 0;
	} else {  
		byte c = GetChar();
		if (c >= ' ') {
//	  lyxerr << "WD[" << fcode << " " << size << " " << c << endl;
			w = mathed_char_width(fcode(), size_, c);
		} else
			if (c == LM_TC_TAB && p_) {
//	 w = p->GetTab(col + 1);
				w = (crow_) ? crow_->getTab(col + 1) : 0;
				//lyxerr << "WW[" << w << "]";
			} else
				if (c == LM_TC_CR && p_) {
					x_ = 0;
					if (crow_ && crow_->getNext()) {
						crow_ = crow_->getNext();
						y_ = crow_->getBaseline();
						w = crow_->getTab(0);
					}
//	  lyxerr << "WW[" << col " " << row << "|" << w << "]";
				} else 
					lyxerr << "No hubo w[" << c << "]!";
	}
	if (MathedIter::Next()) {
//       lyxerr <<"LNX " << pos << endl;
//       if (sw>0 && GetChar()!= LM_TC_UP && GetChar()!= LM_TC_DOWN) {
//	   w = (sx>sw) ? 0: sw-sx;
		if ((sw_ > 0 || sx_ > 0)
		    && GetChar() != LM_TC_UP && GetChar() != LM_TC_DOWN) {
			if (sw_ > 0)
				w = (sx_ > sw_) ? 0 : sw_ - sx_;
			sx_ = sw_ = 0;
		}
		x_ += w;
		return true;
	} else
		return false;
}


void MathedXIter::GoBegin()
{
	Reset();
	x_ = y_ = 0;   
	sw_ = sx_ = 0;
	if (p_) {
		crow_ = p_->getRowSt();
		if (crow_) {
			x_ = crow_->getTab(0);
			y_ = crow_->getBaseline();
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
	int xp = x_;
	int rowp = row;
	int colp = col;
	GoBegin();
	while (row < rowp - 1) Next();
	while (x_ < xp && OK() && !IsCR()) {
		ipush();
		Next();
	}
	if (col > colp) // || (stck.col == colp && stck.x<= xp && x>xp))
		ipop();
	
	return true;
}


bool MathedXIter::Down()
{
	int xp = x_;
	int colp= col;
	// int rowp = row
	
	bool res = (IsCR()) ? true : goNextCode(LM_TC_CR);
	if (res) {
		Next();
		ipush();
		while (x_ < xp && OK()) {
			ipush();
			Next();
		}
		if (col > colp || (stck.col == colp && stck.x <= xp && x_ > xp))
			ipop();
		return true;
	}
	return false;
}


void MathedXIter::addRow()
{
	if (!crow_) {
		lyxerr[Debug::MATHED] << "MathErr: Attempt to insert new"
			" line in a subparagraph. " << this << endl;
		
		return;
	}
	// Create new item for the structure    
	MathedRowSt * r = new MathedRowSt(ncols + 1);
	if (crow_) {
		r->setNext(crow_->getNext());
		crow_->setNext(r);
	} else {
		crow_ = r;
		r->setNext(0);
	}    
	// Fill missed tabs in current row
	while (col < ncols - 1) 
		insert('T', LM_TC_TAB); 
	//newline
	insert('K', LM_TC_CR);
	
	ipush();
	if (!IsCR())
		goNextCode(LM_TC_CR);
	
	// Fill missed tabs in new row
	while (col < ncols - 1) 
		insert('T', LM_TC_TAB);
	ipop();
}


void MathedXIter::delRow()
{
	if (!crow_) {
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
		
		MathedRowSt * r = crow_->getNext();
		if (r) {
			crow_->setNext(r->getNext());
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
	stck.x = x_;
	stck.y = y_;
}


void MathedXIter::ipop()
{ 
	MathedIter::ipop();
	x_ = stck.x;
	y_ = stck.y;
	if (p_) {
		crow_ = p_->getRowSt();
		if (crow_)
			for (int i = 0; i < row; ++i)
				crow_ = crow_->getNext();
	}
}


void MathedXIter::fitCoord(int /*xx*/, int yy)
{
	int xo = 0;
	int yo = 0;
	
	GoBegin();
	if (p_)
		p_->GetXY(xo, yo);
	// first fit vertically
	while (crow_ && OK()) {
		if (yy >= yo + y_ - crow_->ascent()
		    && yy <= yo + y_ + crow_->descent()) 
			break;
		goNextCode(LM_TC_CR);
		Next();
	}
	// now horizontally
//    while (x<xx && Next());
}


void MathedXIter::setTab(int tx, int tab)
{
	if (crow_ && tab <= ncols) {
		crow_->setTab(tab, tx);
	} else
		lyxerr << "MathErr: No tabs allowed here" << endl;
}


void MathedXIter::subMetrics(int a, int d)
{
	if (!crow_) {
		lyxerr[Debug::MATHED]
			<< "MathErr: Attempt to submetric a subparagraph." << endl;
		return;
	}
	crow_->ascent(a);
	crow_->descent(d);
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
	x1 = x_; 
	while (pos < pos2) {
		cx = GetChar();
		if (cx >= ' ') {
			mathed_char_height(fcode(), size_, cx, asc, des);
			if (asc > ascent) ascent = asc;
			if (des > descent) descent = des;
			limit = false;
		} else
			if (MathIsInset(cx)) {
				MathedInset * pp = GetInset();
				if (cx == LM_TC_UP) {
					if (!asc && p_) {
						int xx;
						int yy;
						p_->GetXY(xx, yy);
						static_cast<MathParInset*>(pp)->GetXY(xx, asc);
						asc = yy - asc;
					}
					asc += ((limits_) ? pp->Height() + 4 : pp->Ascent());
				} else if (cx == LM_TC_DOWN) {
					if (!des && p_) {
						int xx;
						int yy;
						p_->GetXY(xx, yy);
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
	width = x_ - x1;
}


bool MathedXIter::setNumbered(bool numb)
{  
	if (crow_) {
		crow_->setNumbered(numb);
		return true;
	}
	
	return false;
}


bool MathedXIter::setLabel(string const & label)
{  
	if (crow_) {
		crow_->setLabel(label);
		return true;
	}
	
	return false;
}


MathedRowSt * MathedXIter::adjustVerticalSt()
{
	GoBegin();
	if (!crow_) {
//	lyxerr << " CRW" << ncols << " ";
		crow_ = new MathedRowSt(ncols + 1); // this leaks
	}
//    lyxerr<< " CRW[" << crow_ << "] ";
	MathedRowSt * mrow = crow_;
	while (OK()) {
		if (IsCR()) {
			if (col >= ncols) ncols = col + 1; 
			MathedRowSt * r = new MathedRowSt(ncols + 1); // this leaks
//	    r->next = crow_->next;
			crow_->setNext(r);
			crow_ = r;
//	    lyxerr << " CX[" << crow_ << "]";
		}   
		Next();	
	}
	return mrow;
}


string const & MathedXIter::getLabel() const
{
	return crow_ ? crow_->getLabel() : error_label;
}


