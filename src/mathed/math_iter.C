/*
 *  File:        math_inset.C
 *  Purpose:     Implementation of insets for mathed
 *  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx> 
 *  Created:     January 1996
 *  Description: 
 *
 *  Dependencies: Xlib, XForms
 *
 *  Copyright: (c) 1996, Alejandro Aguilar Sierra
 *
 *   Version: 0.8beta.
 *
 *   You are free to use and modify this code under the terms of
 *   the GNU General Public Licence version 2 or later.
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation "math_iter.h"
#endif

#include "math_iter.h"
#include "math_inset.h"
#include "symbol_def.h"
#include "support/lstrings.h"
#include "debug.h"

const int SizeInset = sizeof(char*) + 2;
const int SizeFont = 2;

extern int mathed_char_width(short type, int style, byte c);
extern int mathed_string_width(short type, int style, byte const* s, int ls);
extern int mathed_char_height(short, int, byte, int&, int&);

// the builtin memcpy() is broken in egcs and gcc 2.95.x on alpha
// stations. We provide a hand-made version instead. 
inline void my_memcpy( void* ps_in, const void* pt_in, unsigned int n )
{
    char* ps = static_cast<char*>(ps_in);
    char* pt = static_cast<char*>(const_cast<void*>(pt_in));
    for( int i = 0; i < n; i++)
	*ps++ = *pt++; 
}


void MathedIter::Reset()
{
    if (array->last>0 && MathIsFont(array->bf[0])) {
	fcode = array->bf[0];
	pos = 1;
    } else {
	fcode = -1;
	pos = 0;
    }
    col = row = 0;
}


byte MathedIter::GetChar()
{
    if (IsFont()) { 
	fcode = array->bf[pos];
	pos++;
    }
    return array->bf[pos];
}


byte* MathedIter::GetString(int& len)
{
    if (IsFont()) { 
	fcode = array->bf[++pos];
	pos++;
    }
    byte *s = &array->bf[pos];
    len = pos;
    while (array->bf[pos]>= ' ' && pos<array->last) pos++;
    len = pos-len;   
   
   return s;
}

MathedInset* MathedIter::GetInset()
{
   if (IsInset()) {
      MathedInset* p;
      my_memcpy(&p, &array->bf[pos+1], sizeof(p));
      return p;
   } else {
	   lyxerr << "Math Error: This is not an inset["
		  << array->bf[pos] << "]" << endl;
     return 0;
   }
}

// An active math inset MUST be derived from MathParInset because it 
// must have at least one paragraph to edit
MathParInset* MathedIter::GetActiveInset()
{
    if (IsActive()) {
	return (MathParInset*)GetInset();
    } 
    
    lyxerr << "Math Error: This is not an active inset" << endl;
    return 0;
}

bool MathedIter::Next()
{  
    if (!OK()) return false;
   
    if (array->bf[pos]<' ') {
	fcode = -1;     
	if (IsTab()) col++;
	if (IsCR())  {
	    col = 0;
	    row++;
	}
    }
	
    if (IsInset())
      pos += sizeof(char*) + 2;
    else 
      pos++;
    
    if (IsFont()) {
	fcode = array->bf[pos++];
    }

    return true;   
}


bool MathedIter::goNextCode(MathedTextCodes code)
{  
    while (Next()) {
	if (array->bf[pos] == code) 
	  return true;
    }
    
    return false;
}


void MathedIter::goPosAbs(int p)
{  
    Reset();
    while (pos<p && Next());
}


void MathedIter::goPosRel(int dp)
{  
    int posx = pos+dp;
 
    // is posx a valid position?
    if (dp<0)
      Reset();
    while (pos<posx && Next());
}


void MathedIter::Insert(byte c, MathedTextCodes t)
{
    if (c<' ') return;
    
    if (t == LM_TC_TAB && col>= ncols-1) 
      return;
    
    // Never more than one space // array->bf[pos-1] gives error from purify:
    //       Reading 1 byte from 0x47b857 in the heap.
    //  Address 0x47b857 is 1 byte before start of malloc'd block at 0x47b858 of 16 bytes.
    if (c == ' ' && (array->bf[pos] == ' ' || array->bf[pos-1] == ' ')) 
      return;
	
    if (IsFont() && array->bf[pos] == t) {
	fcode = t;
	pos++;
    } else
      if (t!= fcode && pos>0 && MathIsFont(array->bf[pos-1])) {
	  pos--;
	  int k;
	  for (k= pos-1; k>= 0 && array->bf[k]>= ' '; k--);
	  fcode = (k >= 0 && MathIsFont(array->bf[k])) ? array->bf[k]: -1;
      }
    short f = (array->bf[pos]<' ') ? 0: fcode;
    int shift = (t == fcode) ? 1: ((f) ? 3: 2);
    
    if (t == LM_TC_TAB || t == LM_TC_CR) {
	shift--;
	c = t;
	if (t == LM_TC_CR) {
	    row++;
	    col = 0;
	} else
	  col++;
    }
 
    if (pos < array->last)
        array->Move(pos, shift);
    else {
	if (array->last+shift>= array->maxsize) {
	    array->Resize(array->last+shift);
	}
	array->last += shift;
	array->bf[array->last] = '\0';
    }
    if (t != fcode) {
	if (f)  
	  array->bf[pos+shift-1] = fcode;
 	if (c>= ' ') {
	    array->bf[pos++] = t;
	    fcode = t;
	} else {
	    fcode = 0;
	}
    }      
    array->bf[pos++] = c;
}


// Prepare to insert a non-char object
void MathedIter::split(int shift)
{
   if (pos < array->last) {
      bool fg = false;
      if (array->bf[pos]>= ' ') {
	 if (pos> 0 && MathIsFont(array->bf[pos-1]))
	   pos--;
	 else { 
	    fg = true; 
	    shift++;
	 }
      }      
      array->Move(pos, shift);
      if (fg) array->bf[pos+shift-1] = fcode;
   } else {
      if (array->last+shift>= array->maxsize) {
	  array->Resize(array->last+shift);
      }
      array->last += shift;
   }
   array->bf[array->last] = '\0';
}


// I assume that both pos and pos2 are legal positions
void MathedIter::join(int pos2)
{   
    if (!OK() || pos2<= pos)
      return;    

    short f= fcode;	    
    if (pos>0 && array->bf[pos]>= ' ' && MathIsFont(array->bf[pos-1])) 
      pos--;	
    	    
    if (MathIsFont(array->bf[pos2-1]))
      pos2--;
    
    if (array->bf[pos2]>= ' ') {
	for (int p= pos2; p>0; p--) 
	  if (MathIsFont(array->bf[p])) {
	      f = array->bf[p];
	      break;
	  }
	array->bf[pos++] = f;
    }    

    array->Move(pos2, pos-pos2);
}

void MathedIter::Insert(MathedInset* p, int type)
{
    int shift = SizeInset;
    if (!MathIsInset(type))
      type = LM_TC_INSET;
    split(shift);
    array->bf[pos] = type;
    my_memcpy(&array->bf[pos+1], &p, sizeof(p));
    pos += SizeInset;
    array->bf[pos-1] = type;
    array->bf[array->last] = '\0';
    fcode = -1;
}


bool MathedIter::Delete()
{   
   if (!OK())
     return false;
   
   int shift = 0;
   byte c = GetChar();
   if (c>= ' ') { 
      if (MathIsFont(array->bf[pos-1]) && array->bf[pos+1]<' ') {
	 int i;
	 shift = 2;
	 pos--;
	 for (i= pos-1; i>0 && !MathIsFont(array->bf[i]); i--);
	 if (i>0 && MathIsFont(array->bf[i]))
	   fcode = array->bf[i];
      } else
	shift = 1;      
   } else {
      if (MathIsInset(array->bf[pos]))
	shift = sizeof(char*) + 2;
     else 
      if (c == LM_TC_TAB || c == LM_TC_CR) {
	 shift++;
//	 lyxerr <<"Es un tab.";
      }
     else {
	     lyxerr << "Math Warning: expected inset." << endl;
     }
   } 
    
   if (shift!= 0) {
      array->Move(pos+shift, -shift);
      if (pos>= array->last) 
	 pos = (array->last>0) ? array->last: 0;
       return true;
   } else
     return false;
}


LyxArrayBase *MathedIter::Copy(int pos1, int pos2)
{
   if (!array) {
//      lyxerr << "Math error: Attempting to copy a void array." << endl;
      return 0;
   }
      
//   int posx = pos;
   ipush(); 
   LyxArrayBase *t= array, *a;
    
   if (pos1>0 || pos2<= array->last) {       
       short fc= 0;
       if (pos1>0 && array->bf[pos1]>' ') {
	   for (int p= pos1; p>= 0; p--) 
	     if (MathIsFont(array->bf[p])) {
		 if (p!= pos1-1)
		   fc = array->bf[p];
		 else
		   pos1--;
		 break;
	     }
       }

       if (pos2>0 && array->bf[pos2]>= ' ' && MathIsFont(array->bf[pos2-1])) 
	 pos2--;

       int dx = pos2 - pos1;
       a = new LyxArrayBase(dx+LyxArrayBase::ARRAY_MIN_SIZE);
//       lyxerr << "VA " << pos2 << " " << pos2 << " " << dx << endl;
       my_memcpy(&a->bf[(fc) ? 1: 0], &array->bf[pos1], dx);
       if (fc) {
	   a->bf[0] = fc;
	   dx++;
       }
       a->last = dx;
       a->bf[dx] = '\0';
   }  else   
      a = new LyxArrayBase(*array);
   SetData(a);
   while (OK()) {
      if (IsInset()) {
	 MathedInset* inset = GetInset();
	 inset = inset->Clone();
	 my_memcpy(&array->bf[pos+1], &inset, sizeof(inset));
      }
      Next();
   }
//   pos = posx;
   array = t;
   ipop(); 
   return a;
}


void MathedIter::Clear()
{
   if (!array) {
	   lyxerr << "Math error: Attempting to clean a void array." << endl;
      return;
   }   
   Reset();  
   while (OK()) {
      if (IsInset()) {
	 MathedInset* inset = GetInset();
	  if (inset->GetType()!= LM_OT_MACRO_ARG)
	    delete inset;
	  Delete();
      } else
	Next();
   }
}


// Check consistency of tabs and crs
void MathedIter::checkTabs()
{
    ipush();
    
//    MathedIter:Reset();
    while (OK()) {
        if ((IsTab() && col>= ncols-1) || (IsCR() && !(MthIF_CR&flags))) {
            Delete();
            continue;
        }
        if (IsCR() && col<ncols-2) {
            Insert(' ', LM_TC_TAB);
	}
        MathedIter::Next();
    }
    if (col<ncols-2) {
	Insert(' ', LM_TC_TAB);
    }
    ipop();
}         


//  Try to adjust tabs in the expected place, as used in eqnarrays
//  Rules:
//   - If there are a relation operator, put tabs around it
//   - If tehre are not a relation operator, put everything in the
//     3rd column.
void MathedIter::adjustTabs()
{

}         


void MathedXIter::Clean(int pos2)
{
    if (!array) {
	    lyxerr << "Math error: Attempting to clean a void array." << endl;
	return;
    } 
    
    int pos1 = pos;
    
    if (pos2<pos1) {  
	GoBegin();
	while (pos<pos2 && OK()) { Next();
	}
	pos2 = pos1;
	pos1 = pos;
    }

    ipush();
    while (OK() && pos<pos2) {
	if (IsInset()) {
	    MathedInset* inset = GetInset();
	    Next();
	    if (inset->GetType()!= LM_OT_MACRO_ARG)
	      delete inset;
	    continue;
	} 
	if (IsCR()) {
	    if (crow) {
		MathedRowSt *r = crow->next;
		if (r) {
		    crow->next = r->next;
		    delete r;
		}	   
	    }
	}
	Next();
    }    
    ipop();
    
    if (pos2<= array->Last()) {
	pos = pos1;
	join(pos2);
	checkTabs();
    } 
}


void MathedXIter::Merge(LyxArrayBase *a0)
{
    if (!a0) {
	    lyxerr[Debug::MATHED]
		    << "Math error: Attempting to merge a void array." << endl;

	return;
    }
    // All insets must be clonned
    MathedIter it(a0);
    LyxArrayBase *a = it.Copy();
    
    // make rom for the data 
    split(a->Last());
    array->MergeF(a, pos, a->Last());

    int pos1= pos, pos2 = pos + a->Last(); // pos3= 0;

    goPosAbs(pos1);
    
    // Complete rows
    while (pos<pos2 && OK()) {
	if (IsCR()) {
	    if (p && p->Permit(LMPF_ALLOW_CR)) {
		MathedRowSt *r = new MathedRowSt(ncols+1);
		if (crow) {
		    r->next = crow->next;
		    crow->next = r;
		  } else {
		      r->next = 0;
		  }
		crow = r;
	    } else {
		Delete();
		pos2--;
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


//-----------  XIter


MathedXIter::MathedXIter(MathParInset* pp): p(pp) 
{ 
    x = y = 0;
    sx = sw = 0;   
    limits = false;
    s_type = 0;  
    if (p) 
      SetData(p);
    else {
	crow = 0;
	size = 0;
    }
}

void MathedXIter::SetData(MathParInset *pp)
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
	    array = new LyxArrayBase; // this leaks
	p->SetData(array);
    }
    size = p->GetStyle();
    Reset();
}

byte* MathedXIter::GetString(int& ls)
{  
   static byte s[255];
   byte const *sxs =  MathedIter::GetString(ls);
   if (ls>0) {
       strncpy(reinterpret_cast<char*>(s), reinterpret_cast<const char*>(sxs), ls);
       x += mathed_string_width(fcode, size, s, ls);
       return &s[0];
   } 	    
    return 0;
}


bool MathedXIter::Next()
{  
//    lyxerr << "Ne[" << pos << "]";
   if (!OK()) return false;
   int w= 0;
//   lyxerr << "xt ";
   if (IsInset()) {
      MathedInset* px = GetInset();
      w = px->Width();
      if (px->GetType() == LM_OT_SCRIPT) {
	 if (w>sw) sw = w;
	 w = 0;
      } else
	sx = (px->GetLimits()) ? w: 0;
   } else {  
      byte c = GetChar();
      if (c>= ' ') {
//	  lyxerr << "WD[" << fcode << " " << size << " " << c << endl;
	  w = mathed_char_width(fcode, size, c);
      } else
      if (c == LM_TC_TAB && p) {
//	 w = p->GetTab(col+1);
	  w = (crow) ? crow->getTab(col+1): 0;
	 //lyxerr << "WW[" << w << "]";
      } else
      if (c == LM_TC_CR && p) {
	  x = 0;
	  if (crow && crow->next) {
	      crow = crow->next;
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
      if ((sw>0 || sx>0) && GetChar()!= LM_TC_UP && GetChar()!= LM_TC_DOWN) {
	  if (sw>0)
	    w = (sx>sw) ? 0: sw-sx;
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
   while (posx>pos && OK()) Next();  
}


bool MathedXIter::Prev()
{  
    if (pos == 0 || (pos == 1 && GetChar()>= ' '))
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


bool MathedXIter::goNextColumn()
{  
    int rowp = row, colp= col;
    while (Next() && col == colp);
    
    return (col!= colp+1 || rowp!= row);
}


bool MathedXIter::Up()
{
    if (row == 0) return false;
    int xp = x, rowp = row, colp= col;
    GoBegin();
    while (row<rowp-1) Next();
    while (x<xp && OK() && !IsCR()) {
	ipush();
	Next();
    }
    if (col>colp) // || (stck.col == colp && stck.x<= xp && x>xp))
      ipop();
    
    return true;
}


bool MathedXIter::Down()
{
	int xp = x, colp= col; // , rowp = row
    
    bool res = (IsCR()) ? true: goNextCode(LM_TC_CR);
    if (res) {
        Next();
	ipush();
	while (x<xp && OK()) {
	    ipush();
	    Next();
	}
	if (col>colp || (stck.col == colp && stck.x<= xp && x>xp)) 
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
    MathedRowSt *r = new MathedRowSt(ncols+1);
    if (crow) {
	r->next = crow->next;
	crow->next = r;
    } else {
	crow = r;
	r->next = 0;
    }    
    // Fill missed tabs in current row
    while (col<ncols-1) 
      Insert('T', LM_TC_TAB); 
    //newline
    Insert('K', LM_TC_CR);
    
    ipush();
    if (!IsCR())
      goNextCode(LM_TC_CR);
    
    // Fill missed tabs in new row
    while (col<ncols-1) 
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
	if (IsCR()){
	    break;
	} else if (!IsTab()) {
	    line_empty = false;
	}
    } while (Next());
    int p1 = getPos();
    ipop();
    
    if (line_empty) {
	
	MathedRowSt *r = crow->next;
	if (r) {
	    crow->next = r->next;
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
	  for (int i= 0; i<row; i++) 
	    crow = crow->next;
    }
}


void MathedXIter::fitCoord(int /*xx*/, int yy)
{
    int xo = 0, yo = 0;
    
    GoBegin();
    if (p) 
      p->GetXY(xo, yo);
    // first fit vertically
    while (crow && OK()) {
	if (yy>= yo+y-crow->asc && yy<= yo+y+crow->desc) 
	  break;
	goNextCode(LM_TC_CR);
	Next();
    }
    // now horizontally
//    while (x<xx && Next());
}

void MathedXIter::setTab(int tx, int tab)
{
    if (crow && tab<= ncols) {
	crow->w[tab] = tx;
    }
    else 
	    lyxerr << "MathErr: No tabs allowed here" << endl;
}


void MathedXIter::subMetrics(int a, int d)
{
    if (!crow) {
//	lyxerr << "MathErr: Attempt to submetric a subparagraph." << endl;
	return;
    }    
    crow->asc = a;
    crow->desc = d;
}


// This function is not recursive, as MathPar::Metrics is
void MathedXIter::IMetrics(int pos2, int& width, int& ascent, int& descent)
{  
	byte cx, cxp= 0;// *s;
	int x1;// ls;
    int asc= 0, des= 0;
    bool limit = false;
  
    descent = ascent = width = 0;
    if (!array) return;
    if (array->empty()) return;
//    if  (pos2 > array->last) return;
    x1 = x; 
    while (pos<pos2) {
	cx = GetChar();
	if (cx >= ' ') {
	    mathed_char_height(FCode(), size, cx, asc, des);
	    if (asc > ascent) ascent = asc;
	    if (des > descent) descent = des;
	    limit = false;
	} else
	if (MathIsInset(cx)) {
	    MathedInset *pp = GetInset();
	    if (cx == LM_TC_UP) {
		if (!asc && p) {
		    int xx, yy;
		    p->GetXY(xx, yy);
		    static_cast<MathParInset*>(pp)->GetXY(xx, asc);
		    asc = yy - asc;
		}
		asc += ((limits) ? pp->Height()+4: pp->Ascent());
	    } else
	      if (cx == LM_TC_DOWN) {
		  if (!des && p) {
		      int xx, yy;
		      p->GetXY(xx, yy);
		      static_cast<MathParInset*>(pp)->GetXY(xx, des);
		      if (des-pp->Height()<yy && !asc)
			asc = yy - (des-pp->Height());
		      des -= yy;
		  }
		  des += (limit ? pp->Height()+4: pp->Height()-pp->Ascent()/2);
	      } else {
		  asc = pp->Ascent();
		  des = pp->Descent();
	      }
	    if (asc > ascent) ascent = asc;
	    if (des > descent) descent = des;
	    if (cx!= LM_TC_UP && cx!= LM_TC_DOWN)
	      limit = pp->GetLimits();
	} else 
	if (cx == LM_TC_TAB) {
	    limit = false;                   
	}      
	else {
		lyxerr[Debug::MATHED]
			<< "Mathed Sel-Error: Unrecognized code["
			<< cx << ']' << endl;
	    break;
	}       
	if (pos < pos2)  Next();
	cxp = cx;
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


bool MathedXIter::setLabel(char* label)
{  
    if (label && crow) {
	crow->setLabel(label);
	return true;
    }
    
    return false;
}


MathedRowSt *MathedXIter::adjustVerticalSt()
{
    GoBegin();
    if (!crow) {
//	lyxerr << " CRW" << ncols << " ";
	    crow = new MathedRowSt(ncols+1); // this leaks
    }
//    lyxerr<< " CRW[" << crow << "] ";
    MathedRowSt *mrow = crow;
    while (OK()) {
	if (IsCR()) {
	    if (col>= ncols) ncols = col+1; 
	    MathedRowSt *r = new MathedRowSt(ncols+1); // this leaks
//	    r->next = crow->next;
	    crow->next = r;
	    crow = r;
//	    lyxerr << " CX[" << crow << "]";
	}   
	Next();	
    }
    return mrow;
}

