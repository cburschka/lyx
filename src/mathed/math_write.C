/*
 *  File:        math_write.h
 *  Purpose:     Write math paragraphs in LaTeX
 *  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx> 
 *  Created:     January 1996
 *  Description: 
 *
 *  Dependencies: Xlib, XForms
 *
 *  Copyright: 1996, 1997 Alejandro Aguilar Sierra
 *
 *   Version: 0.8beta, Mathed & Lyx project.
 *
 *   You are free to use and modify this code under the terms of
 *   the GNU General Public Licence version 2 or later.
 */

#include <config.h>

#include "LString.h"
#include "math_inset.h"
#include "math_iter.h"
#include "math_parser.h"
#include "support/lstrings.h"
#include "debug.h"

using std::ostream;
using std::endl;

extern char const * latex_mathenv[];
extern char * latex_mathspace[];

// quite a hack i know. Should be done with return values...
static int number_of_newlines;

char const * math_font_name[] = {
   "mathrm",
   "mathcal",
   "mathbf",
   "mathsf",
   "mathtt",
   "mathit",
   "textrm"
};


void
MathSpaceInset::Write(ostream & os, bool /* fragile */)
{
   if (space >= 0 && space < 6) {
	   os << '\\' << latex_mathspace[space] << ' ';
   }
}


void
MathDotsInset::Write(ostream & os, bool /* fragile */)
{
	os << '\\' << name << ' ';
}


void MathSqrtInset::Write(ostream & os, bool fragile)
{
	os << '\\' << name << '{';
	MathParInset::Write(os, fragile); 
	os << '}';
}


void MathDelimInset::Write(ostream & os, bool fragile)
{
    latexkeys * l = (left != '|') ? lm_get_key_by_id(left, LM_TK_SYM): 0;
    latexkeys * r = (right != '|') ? lm_get_key_by_id(right, LM_TK_SYM): 0;
    os << "\\left";
    if (l) {
	    os << '\\' << l->name << ' ';
    } else {
        if (left == '{' || left == '}') {
		os << '\\' << char(left) << ' ';
        } else {
		os << char(left) << ' ';
        }
    }
   MathParInset::Write(os, fragile);
   os << "\\right";
   if (r) {
	   os << '\\' << r->name << ' ';
   } else {
       if (right == '{' || right == '}') {
	       os << '\\' << char(right) << ' ';
      } else {
	      os << char(right) << ' ';
      }
   }
}


void MathDecorationInset::Write(ostream & os, bool fragile)
{
	latexkeys * l = lm_get_key_by_id(deco, LM_TK_WIDE);
	if (fragile &&
	    (strcmp(l->name, "overbrace") == 0 ||
	     strcmp(l->name, "underbrace") == 0 ||
	     strcmp(l->name, "overleftarrow") == 0 ||
	     strcmp(l->name, "overrightarrow") == 0))
		os << "\\protect";
	os << '\\' << l->name << '{';
	MathParInset::Write(os, fragile);  
	os << '}';
}


void MathAccentInset::Write(ostream & os, bool fragile)
{
	latexkeys * l = lm_get_key_by_id(code, LM_TK_ACCENT);
	os << '\\' << l->name;
	if (code!= LM_not)
		os << '{';
	else
		os << ' ';
	
	if (inset) {
		inset->Write(os, fragile);
	} else {
		if (fn>= LM_TC_RM && fn<= LM_TC_TEXTRM) {
			os << '\\'
			   << math_font_name[fn-LM_TC_RM]
			   << '{';
		}
		if (MathIsSymbol(fn)) {
			latexkeys * l = lm_get_key_by_id(c, LM_TK_SYM);
			if (l) {
				os << '\\' << l->name << ' ';
			}
		} else
			os << char(c);
		
		if (fn>= LM_TC_RM && fn<= LM_TC_TEXTRM)
			os << '}';
	}
	
	if (code!= LM_not)
		os << '}';
}


void MathBigopInset::Write(ostream & os, bool /* fragile */)
{
    bool limp = GetLimits();
    
    os << '\\' << name;
    
    if (limp && !(sym != LM_int && sym != LM_oint
		  && (GetStyle() == LM_ST_DISPLAY)))
	    os << "\\limits ";
    else 
    if (!limp && (sym != LM_int && sym != LM_oint
		  && (GetStyle() == LM_ST_DISPLAY)))
	    os << "\\nolimits ";
    else 
	    os << ' ';
}


void MathFracInset::Write(ostream & os, bool fragile)
{
	os << '\\' << name << '{';
	MathParInset::Write(os, fragile);
	os << "}{";
	den->Write(os, fragile);
	os << '}';
}


void MathParInset::Write(ostream & os, bool fragile)
{
	if (!array) return;
	int brace = 0;
	latexkeys * l;
	MathedIter data(array);
	// hack
	MathedRowSt const * crow = getRowSt();   
	data.Reset();
	
	if (!Permit(LMPF_FIXED_SIZE)) { 
		l = lm_get_key_by_id(size, LM_TK_STY);
		if (l) {
			os << '\\' << l->name << ' ';
		}
	}
	while (data.OK()) {
		byte cx = data.GetChar();
		if (cx >= ' ') {
			int ls;
			byte * s = data.GetString(ls);
			
			if (data.FCode() >= LM_TC_RM && data.FCode() <= LM_TC_TEXTRM) {
				os << '\\' << math_font_name[data.FCode()-LM_TC_RM] << '{';
			}
			while (ls > 0) {
				if (MathIsSymbol(data.FCode())) {
					l = lm_get_key_by_id(*s, (data.FCode() == LM_TC_BSYM) ?
							     LM_TK_BIGSYM : LM_TK_SYM);
					if (l) {
						os << '\\' << l->name << ' ';
					} else { 
						lyxerr << "Illegal symbol code[" << *s
						       << " " << ls << " " << data.FCode() << "]";
					}
				} else {
					// Is there a standard logical XOR?
					if ((data.FCode() == LM_TC_TEX && *s != '{' && *s != '}') ||
					    (data.FCode() == LM_TC_SPECIAL))
						os << '\\';
					else {
						if (*s == '{') ++brace;
						if (*s == '}') --brace;
					}
					if (*s == '}' && data.FCode() == LM_TC_TEX && brace < 0) 
						lyxerr <<"Math warning: Unexpected closing brace."
						       << endl;
					else	       
						os << char(*s);
				}
				++s; --ls;
			}
			if (data.FCode()>= LM_TC_RM && data.FCode()<= LM_TC_TEXTRM)
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


void mathed_write(MathParInset * p, ostream & os, int * newlines,
		  bool fragile, string const & label)
{
   number_of_newlines = 0;
   short mathed_env = p->GetType();

   if (mathed_env == LM_EN_INTEXT) {
	   if (fragile) os << "\\protect";
	   os << "\\( "; // changed from " \\( " (Albrecht Dress)
   } 
   else {
     if (mathed_env == LM_EN_DISPLAY){
	     os << "\\[\n";
     } else {
	     os << "\\begin{"
		<< latex_mathenv[mathed_env]
		<< "}\n";
     }
     ++number_of_newlines;
   }
   
   if (!label.empty() && label[0] > ' ' && mathed_env == LM_EN_EQUATION){
	   os << "\\label{"
	      << label
	      << "}\n";
     ++number_of_newlines;
   }

   p->Write(os, fragile);
   
   if (mathed_env == LM_EN_INTEXT){
	   if (fragile) os << "\\protect";
	   os << " \\)";
   }
   else if (mathed_env == LM_EN_DISPLAY) {
	   os << "\\]\n";
     ++number_of_newlines;
   }
   else {
	   os << "\n\\end{"
	      << latex_mathenv[mathed_env]
	      << "}\n";
     number_of_newlines += 2;
   }
   *newlines = number_of_newlines;
}
