#include <config.h>

#include "math_parinset.h"
#include "math_iter.h"
#include "array.h"
#include "math_xiter.h"
#include "LColor.h"
#include "mathed/support.h"
#include "Painter.h"
#include "math_parser.h"
#include "math_rowst.h"
#include "math_parinset.h"
#include "debug.h"

using std::endl;

extern int number_of_newlines;


MathedRowSt * MathParInset::getRowSt() const
{
	return 0;
}


MathParInset::MathParInset(short st, string const & nm, short ot)
	: MathedInset(nm, ot, st)
{
	ascent = 8;
	width = 4;
	descent = 0;
	flag = 1;
	if (objtype == LM_OT_SCRIPT)
		flag |= LMPF_SCRIPT;
}


MathedInset * MathParInset::Clone()
{
	return new MathParInset(*this);
}


void MathParInset::setData(MathedArray const & a)
{
	array = a;
	
	// A standard paragraph shouldn't have any tabs nor CRs.
	MathedIter it(&array);
	while (it.OK()) {
		char c = it.GetChar();
		if (c == LM_TC_TAB || c == LM_TC_CR) 
			it.Delete();
		else
			it.Next();
	}
}


void 
MathParInset::draw(Painter & pain, int x, int y)
{
	byte cxp = 0;
	int xp = 0;
	int asc = df_asc;
	int des = 0;
	bool limits = false;
	
	xo_ = x;
	yo_ = y;
	MathedXIter data(this);
	if (array.empty()) {
		//MathedXIter data(this);
		data.GetPos(x, y);
		pain.rectangle(x, y - df_asc, df_width, df_asc,
			       LColor::mathline);
		return;
	}
	//MathedXIter data(this);
	data.GoBegin();
	while (data.OK()) {
		data.GetPos(x, y);
		byte cx = data.GetChar();
		if (cx >= ' ') {
			string const s = data.GetString();
			drawStr(pain, data.fcode(), size(), x, y, s);
			mathed_char_height(LM_TC_CONST, size(), 'y', asc, des);
			limits = false;
		} else {
			if (cx == 0)
				break;
			if (MathIsInset(cx)) {
				int yy = y;
				MathedInset * p = data.GetInset();
				if (cx == LM_TC_UP) {
					if (limits) {
						x -= (xp > p->Width()) ?
							p->Width() + (xp - p->Width()) / 2 : xp;  
						yy -= (asc + p->Descent() + 4);
					} else
						yy -= (p->Descent() > asc) ?
							p->Descent() + 4 : asc;
				} else if (cx == LM_TC_DOWN) {
					if (limits) {
						x -= (xp > p->Width()) ?
							p->Width() + (xp - p->Width()) / 2 : xp;
						yy += des + p->Ascent() + 2;
					} else
						yy += des + p->Ascent() / 2;
				} else {
					asc = p->Ascent();
					des = p->Descent();
				}
				p->draw(pain, x, yy);
				if (cx != LM_TC_UP && cx != LM_TC_DOWN) {
					limits = p->GetLimits();
					if (limits)
						xp = p->Width();
				}
				data.Next();
			} else if (cx == LM_TC_TAB) {
				if (cxp == cx
				    || cxp == LM_TC_CR || data.IsFirst()) {
					pain.rectangle(x, y - df_asc,
						       df_width, df_asc,
						       LColor::mathline);
				}
				data.Next();
				limits = false;
			} else if (cx == LM_TC_CR) {
				if (cxp == LM_TC_TAB
				    || cxp == LM_TC_CR || data.IsFirst()) {
					pain.rectangle(x, y - df_asc,
						       df_width, df_asc,
						       LColor::mathline);
				}
				data.Next();
				limits = false;
			} else {	 
				lyxerr << "GMathed Error: Unrecognized code[" << cx << "]" << endl;
				break;
			}
		}
		cxp = cx;
	}
	if (cxp == LM_TC_TAB || cxp == LM_TC_CR) { 
		data.GetPos(x, y);
		pain.rectangle(x, y - df_asc, df_width, df_asc,
			       LColor::mathline);
	}
}


void 
MathParInset::Metrics()
{
	byte cx;
	byte cxp = 0;
	int ls;
	int asc = df_asc;
	int des = 0;
	int tb = 0;
	int tab = 0;
	
	bool limits = false;
	
	ascent = df_asc;//mathed_char_height(LM_TC_VAR, size, 'I', asc, des); 
	width = df_width;
	descent = 0;
	if (array.empty()) return;
	
	ascent = 0;
	MathedXIter data(this);
	data.GoBegin();
	while (data.OK()) {
		cx = data.GetChar();      
		if (cx >= ' ') {
			string s = data.GetString();
			mathed_string_height(data.fcode(),
					     size(), s, asc, des);
			if (asc > ascent) ascent = asc;
			if (des > descent) descent = des;
			limits = false;
			mathed_char_height(LM_TC_CONST, size(), 'y', asc, des);
		} else
			if (MathIsInset(cx)) {
				MathedInset * p = data.GetInset();
				p->SetStyle(size());   
				p->Metrics();
				if (cx == LM_TC_UP) {
					asc += (limits) ? p->Height() + 4: p->Ascent() + 
						((p->Descent()>asc) ? p->Descent() - asc + 4: 0);
				} else
					if (cx == LM_TC_DOWN) {
						des += ((limits) ? p->Height() + 4: p->Height() - p->Ascent() / 2);
					} else {
						asc = p->Ascent();
						des = p->Descent();
					}
				if (asc > ascent) ascent = asc;
				if (des > descent) descent = des;
				if (cx!= LM_TC_UP && cx!= LM_TC_DOWN)
					limits = p->GetLimits();
				data.Next();
			} else 
				if (cx == LM_TC_TAB) {
					int x, y;
					data.GetIncPos(x, y);
					if (data.IsFirst() || cxp == LM_TC_TAB || cxp == LM_TC_CR) {
						if (ascent<df_asc) ascent = df_asc;
						tb = x;
					}
					data.setTab(x-tb, tab);
					tb = x;
					++tab;
					limits = false;                   
					data.Next();
				} else
					if (cx == LM_TC_CR) {
						if (tb > 0) {
							int x, y;
							data.GetIncPos(x, y);
							if (data.IsFirst() || cxp == LM_TC_TAB || cxp == LM_TC_CR) {
								if (ascent<df_asc) ascent = df_asc;
								tb = x;
							} 
							data.setTab(x - tb, tab);
						} else //if (GetColumns() == 1) 
							{
								int x, y;
								data.GetIncPos(x, y);
								data.setTab(x, tab);
								if (ascent<df_asc) ascent = df_asc;
							} 
						tb = tab = 0;
						data.subMetrics(ascent, descent);
						ascent = df_asc;   
						descent = 0;
						data.Next();
					} else {
						lyxerr << "Mathed Error: Unrecognized code[" << cx
						       << "]" << endl;
						break;
					}       
		cxp = cx;
	}
	data.GetIncPos(width, ls);
	
	// No matter how simple is a matrix, it is NOT a subparagraph
	if (isMatrix()) {
		if (cxp == LM_TC_TAB) {
			if (ascent<df_asc) ascent = df_asc;
			data.setTab(0, tab);
		} else {
			data.setTab(width - tb, tab);
		}
	}
	
	data.subMetrics(ascent, descent);
}



void MathParInset::Write(ostream & os, bool fragile)
{
	if (array.empty()) return;
	int brace = 0;
	latexkeys const * l;
	MathedIter data(&array);
	// hack
	MathedRowSt const * crow = getRowSt();   
	data.Reset();
	
	if (!Permit(LMPF_FIXED_SIZE)) { 
		l = lm_get_key_by_id(size(), LM_TK_STY);
		if (l) {
			os << '\\' << l->name << ' ';
		}
	}
	while (data.OK()) {
		byte cx = data.GetChar();
		if (cx >= ' ') {
			string str = data.GetString();
			
			if (data.fcode() >= LM_TC_RM && data.fcode() <= LM_TC_TEXTRM) {
				os << '\\' << math_font_name[data.fcode()-LM_TC_RM] << '{';
			}
			for (string::const_iterator s = str.begin();
			     s != str.end(); ++s) {
				byte c = *s;
				if (MathIsSymbol(data.fcode())) {
					l = lm_get_key_by_id(c, (data.fcode() == LM_TC_BSYM) ?
							     LM_TK_BIGSYM : LM_TK_SYM);
					if (l) {
						os << '\\' << l->name << ' ';
					} else {
#warning this does not compile on gcc 2.97
						//lyxerr << "Illegal symbol code[" << c
						//   << " " << str.end() - s << " " << data.fcode() << "]";
					}
				} else {
					// Is there a standard logical XOR?
					if ((data.fcode() == LM_TC_TEX && c != '{' && c != '}') ||
					    (data.fcode() == LM_TC_SPECIAL))
						os << '\\';
					else {
						if (c == '{') ++brace;
						if (c == '}') --brace;
					}
					if (c == '}' && data.fcode() == LM_TC_TEX && brace < 0) 
						lyxerr <<"Math warning: Unexpected closing brace."
						       << endl;
					else	       
						os << char(c);
				}
			}
			if (data.fcode()>= LM_TC_RM && data.fcode()<= LM_TC_TEXTRM)
				os << '}';
		} else     
			if (MathIsInset(cx)) {
				MathedInset * p = data.GetInset();
				if (cx == LM_TC_UP)
					os << "^{";
				if (cx == LM_TC_DOWN)
					os << "_{";
				p->Write(os, fragile);
				if (cx == LM_TC_UP || cx == LM_TC_DOWN)
					os << '}';
				data.Next();
			} else
				switch (cx) {
				case LM_TC_TAB:
				{
					os << " & ";
					data.Next();
					break;
				}
				case LM_TC_CR:
				{
					if (crow) {
						if (!crow->isNumbered()) {  
							os << "\\nonumber ";
						}
						if (!crow->getLabel().empty()) {
							os << "\\label{"
							   << crow->getLabel()
							   << "} ";
						}
						crow = crow->getNext();
					}
					if (fragile)
						os << "\\protect";
					os << "\\\\\n";
					++number_of_newlines;
					data.Next();
					break;
				}
				default:
					lyxerr << "WMath Error: unrecognized code[" << cx << "]";
					return;
				}     
	}
	
	if (crow) {
		if (!crow->isNumbered()) {
			os << "\\nonumber ";
		}
		if (!crow->getLabel().empty()) {
			os << "\\label{"
			   << crow->getLabel()
			   << "} ";
		}
	}

	if (brace > 0)
		os << string(brace, '}');
}


void MathParInset::clear()
{
	array.clear();
}

bool MathParInset::Inside(int x, int y) 
{
  return (x >= xo() && x <= xo() + width
	  && y <= yo() + descent && y >= yo() - ascent);
}


void MathParInset::GetXY(int & x, int & y) const
{
   x = xo();
   y = yo();
}


void MathParInset::UserSetSize(short sz)
{
   if (sz >= 0) {
       size(sz);      
       flag = flag & ~LMPF_FIXED_SIZE;
   }
}


void MathParInset::SetStyle(short sz) 
{
    if (Permit(LMPF_FIXED_SIZE)) {
	if (Permit(LMPF_SCRIPT)) 
	  sz = (sz < LM_ST_SCRIPT) ? LM_ST_SCRIPT: LM_ST_SCRIPTSCRIPT;
	if (Permit(LMPF_SMALLER) && sz < LM_ST_SCRIPTSCRIPT) {
	    ++sz;
	} 
	MathedInset::SetStyle(sz);
    }
}


bool MathParInset::Permit(short f) const
{
	return bool(f & flag);
}


MathedArray & MathParInset::GetData()
{
	return array;
}


MathedArray const & MathParInset::GetData() const
{
	return array;
}


void MathParInset::setXY(int x, int y)
{
	xo_ = x;
	yo_ = y;
}
