/*
 *  File:        math_write.h
 *  Purpose:     Write math paragraphs in LaTeX
 *  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx> 
 *  Created:     January 1996
 *  Description: 
 *
 *  Dependencies: Xlib, XForms
 *
 *  Copyright: (c) 1996, 1997 Alejandro Aguilar Sierra
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
MathSpaceInset::Write(ostream & os)
{
#ifdef USE_OSTREAM_ONLY
   if (space >= 0 && space < 6) {
	   os << '\\' << latex_mathspace[space] << ' ';
   }
#else
   if (space >= 0 && space < 6) {
       string output;
       MathSpaceInset::Write(output);
       os << output;
   }
#endif
}


#ifndef USE_OSTREAM_ONLY
void
MathSpaceInset::Write(string & outf)
{ 
   if (space >= 0 && space < 6) {
       outf += '\\';
       outf += latex_mathspace[space];
       outf += ' ';
   }
}
#endif


void
MathDotsInset::Write(ostream & os)
{
#ifdef USE_OSTREAM_ONLY
	os << '\\' << name << ' ';
#else
   string output;
   MathDotsInset::Write(output);
   os << output;
#endif
}


#ifndef USE_OSTREAM_ONLY
void
MathDotsInset::Write(string & outf)
{
   outf += '\\';
   outf += name;
   outf += ' ';
}
#endif


void MathSqrtInset::Write(ostream & os)
{
#ifdef USE_OSTREAM_ONLY
	os << '\\' << name << '{';
	MathParInset::Write(os); 
	os << '}';
#else
   string output;
   MathSqrtInset::Write(output);
   os << output;
#endif
}


#ifndef USE_OSTREAM_ONLY
void MathSqrtInset::Write(string & outf)
{ 
   outf += '\\';
   outf += name;
   outf += '{';
   MathParInset::Write(outf);  
   outf += '}';
}
#endif


void MathDelimInset::Write(ostream & os)
{
#ifdef USE_OSTREAM_ONLY
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
   MathParInset::Write(os);
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
#else
    string output;
    MathDelimInset::Write(output);
    os << output;
#endif
}


#ifndef USE_OSTREAM_ONLY
void MathDelimInset::Write(string & outf)
{
    latexkeys * l = (left != '|') ? lm_get_key_by_id(left, LM_TK_SYM): 0;
    latexkeys * r = (right != '|') ? lm_get_key_by_id(right, LM_TK_SYM): 0;
    outf += "\\left";
    if (l) {
        outf += '\\';
	outf += l->name;
	outf += ' ';
    } else {
        if (left == '{' || left == '}') {
	    outf += '\\';
	    outf += char(left);
	    outf += ' ';
        } else {
	    outf += char(left);
	    outf += ' ';
        }
    }
   MathParInset::Write(outf);  
   outf += "\\right";
   if (r) {
       outf += '\\';
       outf += r->name;
       outf += ' ';
   } else {
       if (right == '{' || right == '}') {
	   outf += '\\';
	   outf += char(right);
	   outf += ' ';
      } else {
	   outf += char(right);
	   outf += ' ';
      }
   }        
}
#endif


void MathDecorationInset::Write(ostream & os)
{
#ifdef USE_OSTREAM_ONLY
	latexkeys * l = lm_get_key_by_id(deco, LM_TK_WIDE);
	os << '\\' << l->name << '{';
	MathParInset::Write(os);  
	os << '}';
#else
   string output;
   MathDecorationInset::Write(output);
   os << output;
#endif
}


#ifndef USE_OSTREAM_ONLY
void MathDecorationInset::Write(string & outf)
{ 
   latexkeys * l = lm_get_key_by_id(deco, LM_TK_WIDE);
   outf += '\\';
   outf += l->name;
   outf += '{';
   MathParInset::Write(outf);  
   outf += '}';
}
#endif


void MathAccentInset::Write(ostream & os)
{
#ifdef USE_OSTREAM_ONLY
	latexkeys * l = lm_get_key_by_id(code, LM_TK_ACCENT);
	os << '\\' << l->name;
	if (code!= LM_not)
		os << '{';
	else
		os << ' ';
	
	if (inset) {
		inset->Write(os);
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
#else
    string output;
    MathAccentInset::Write(output);
    os << output;
#endif
}


#ifndef USE_OSTREAM_ONLY
void MathAccentInset::Write(string & outf)
{ 
    latexkeys * l = lm_get_key_by_id(code, LM_TK_ACCENT);
    outf += '\\';
    outf += l->name;
    if (code!= LM_not)
      outf += '{';
    else
      outf += ' ';

    if (inset) {
      inset->Write(outf);
    } else {
      if (fn>= LM_TC_RM && fn<= LM_TC_TEXTRM) {
        outf += '\\';
        outf += math_font_name[fn-LM_TC_RM];
        outf += '{';
      }
      if (MathIsSymbol(fn)) {
          latexkeys *l = lm_get_key_by_id(c, LM_TK_SYM);
          if (l) {
	    outf += '\\';
	    outf += l->name;
	    outf += ' ';
          }
      } else
        outf += char(c);

      if (fn>= LM_TC_RM && fn<= LM_TC_TEXTRM)
        outf += '}';
    }

    if (code!= LM_not)
      outf += '}';
}
#endif


void MathBigopInset::Write(ostream & os)
{
#ifdef USE_OSTREAM_ONLY
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
#else
   string output;
   MathBigopInset::Write(output);
   os << output;
#endif
}


#ifndef USE_OSTREAM_ONLY
void MathBigopInset::Write(string & outf)
{ 
    bool limp = GetLimits();
    
    outf += '\\';
    outf += name;
    
    if (limp && !(sym!= LM_int && sym!= LM_oint && (GetStyle() == LM_ST_DISPLAY)))
      outf += "\\limits ";
    else 
    if (!limp && (sym!= LM_int && sym!= LM_oint && (GetStyle() == LM_ST_DISPLAY)))
	outf += "\\nolimits ";
    else 
      outf += ' ';
}
#endif


void MathFracInset::Write(ostream & os)
{
#ifdef USE_OSTREAM_ONLY
	os << '\\' << name << '{';
	MathParInset::Write(os);
	os << "}{";
	den->Write(os);
	os << '}';
#else
   string output;
   MathFracInset::Write(output);
   os << output;
#endif
}


#ifndef USE_OSTREAM_ONLY
void MathFracInset::Write(string & outf)
{ 
   outf += '\\';
   outf += name;
   outf += '{';
   MathParInset::Write(outf);  
   outf += "}{";
   den->Write(outf);  
   outf += '}';
}
#endif


void MathParInset::Write(ostream & os)
{
#ifdef USE_OSTREAM_ONLY
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
				p->Write(os);
				if (cx == LM_TC_UP || cx == LM_TC_DOWN)
					os << '}';
				data.Next();
			} else
				switch(cx) {
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
						if (crow->getLabel()) {
							os << "\\label{"
							   << crow->getLabel()
							   << "} ";
						}
						crow = crow->getNext();
					}
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
		if (crow->getLabel()) {
			os << "\\label{"
			   << crow->getLabel()
			   << "} ";
		}
	}
#if 1
	while (brace > 0) {
		os << '}';
		--brace;
	}
#else
	// Something like this should work too:
	os << string(brace, '}'); // not one-off error I hope.
#endif
#else
   if (!array) return;
   string output;
   MathParInset::Write(output);
   os << output;
#endif
}


#ifndef USE_OSTREAM_ONLY
void MathParInset::Write(string & outf)
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
	   outf += '\\';
	   outf += l->name;
	   outf += ' ';
       }
   }
   while (data.OK()) {
      byte cx = data.GetChar();
      if (cx>= ' ') {
	 int ls;
	 byte * s = data.GetString(ls);
       
	 if (data.FCode()>= LM_TC_RM && data.FCode()<= LM_TC_TEXTRM) {
	     outf += '\\';
	     outf += math_font_name[data.FCode()-LM_TC_RM];
	     outf += '{';
	 }
	 while (ls>0) {
	    if (MathIsSymbol(data.FCode())) {
		l = lm_get_key_by_id(*s,(data.FCode() == LM_TC_BSYM)?LM_TK_BIGSYM:LM_TK_SYM);
	       if (l) {
		 outf += '\\';
		 outf += l->name;
		 outf += ' ';
	       } else { 
		       lyxerr << "Illegal symbol code[" << *s
			      << " " << ls << " " << data.FCode() << "]";
	       }
	    } else {
	       // Is there a standard logical XOR?
	       if ((data.FCode() == LM_TC_TEX && *s!= '{' && *s!= '}') ||
		   (data.FCode() == LM_TC_SPECIAL))
		 outf += '\\';
	       else {
		  if (*s == '{') ++brace;
		  if (*s == '}') --brace;
	       }
	       if (*s == '}' && data.FCode() == LM_TC_TEX && brace<0) 
		       lyxerr <<"Math warning: Unexpected closing brace."
			      << endl;
	       else	       
		 outf += char(*s);
	    }
	    ++s; --ls;
	 }
	 if (data.FCode()>= LM_TC_RM && data.FCode()<= LM_TC_TEXTRM)
	   outf += '}';
      } else     
      if (MathIsInset(cx)) {
	 MathedInset *p = data.GetInset();
	 if (cx == LM_TC_UP)
	   outf += "^{";
	 if (cx == LM_TC_DOWN)
	   outf += "_{";
	 p->Write(outf);
	 if (cx == LM_TC_UP || cx == LM_TC_DOWN)
	   outf += '}';
	 data.Next();
      } else
      	switch(cx) {
	 case LM_TC_TAB:
	    {
	       outf += " & ";
	       data.Next();
	       break;
	    }
	 case LM_TC_CR:
	    {
		if (crow) {
		    if (!crow->isNumbered()) {  
		        outf += "\\nonumber ";
		    }
		    if (crow->getLabel()) {
		        outf += "\\label{";
		        outf += crow->getLabel();
			outf += "} ";
		    }
		    crow = crow->getNext();
		}
	       outf += "\\\\\n";
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
	    outf += "\\nonumber ";
	}
	if (crow->getLabel()) {
	    outf += "\\label{";
	    outf += crow->getLabel();
	    outf += "} ";
	}
    }
   while (brace>0) {
      outf += '}';
      --brace;
   }
}
#endif


void MathMatrixInset::Write(ostream & os)
{
#ifdef USE_OSTREAM_ONLY
    if (GetType() == LM_OT_MATRIX){
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
    MathParInset::Write(os);
    if (GetType() == LM_OT_MATRIX){
	    os << "\n\\end{"
	       << name
	       << '}';
	++number_of_newlines;
    }
#else
    string output;
    MathMatrixInset::Write(output);
    os << output;
#endif
}


#ifndef USE_OSTREAM_ONLY
void MathMatrixInset::Write(string & outf)
{
    if (GetType() == LM_OT_MATRIX){
	outf += "\\begin{";
	outf += name;
	outf += '}';
	if (v_align == 't' || v_align == 'b') {
	    outf += '[';
	    outf += char(v_align);
	    outf += ']';
	}
	outf += '{';
	outf += h_align;
	outf += "}\n";
	++number_of_newlines;
    }
    MathParInset::Write(outf);
    if (GetType() == LM_OT_MATRIX){
	outf += "\n\\end{";
	outf += name;
	outf += '}';
	++number_of_newlines;
    }
}
#endif


void mathed_write(MathParInset * p, ostream & os, int * newlines,
		  char fragile, char const * label)
{
#ifdef USE_OSTREAM_ONLY
   number_of_newlines = 0;
   short mathed_env = p->GetType();

   if (mathed_env == LM_EN_INTEXT) {
	   if (fragile) os << "\\protect";
	   os << "\\( "; // changed from " \\( " (Albrecht Dress)
   } 
   else {
	   // Thinko!
	   // Is this '\n' really needed, what can go wrong
	   //if it is not there?
#warning Thinko!
#if 0
     if (!suffixIs(outf, '\n')) {
       // in batchmode we need to make sure
       // a space before an equation doesn't
       // make the LaTeX output different 
       // compared to "Exported LaTeX"  ARRae
       // Modified to work in a cleaner and hopefully more general way
       // (JMarc)
       outf += "\n";
       ++number_of_newlines;
     }
#endif
     if (mathed_env == LM_EN_DISPLAY){
	     os << "\\[\n";
     } else {
	     os << "\\begin{"
		<< latex_mathenv[mathed_env]
		<< "}\n";
     }
     ++number_of_newlines;
   }
   
   if (label && label[0] > ' ' && mathed_env == LM_EN_EQUATION){
	   os << "\\label{"
	      << label
	      << "}\n";
     ++number_of_newlines;
   }

   p->Write(os);
   
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
#else
   string output;
   mathed_write(p, output, newlines, fragile, label);
   os << output;
#endif
}


#ifndef USE_OSTREAM_ONLY
void mathed_write(MathParInset * p, string & outf, int * newlines,
                  char fragile, char const * label)
{
   number_of_newlines = 0;
   short mathed_env = p->GetType();

   if (mathed_env == LM_EN_INTEXT) {
     if (fragile) outf += "\\protect";
     outf += "\\( "; // changed from " \\( " (Albrecht Dress)
   } 
   else {
     if (!suffixIs(outf, '\n')) {
       // in batchmode we need to make sure
       // a space before an equation doesn't
       // make the LaTeX output different 
       // compared to "Exported LaTeX"  ARRae
       // Modified to work in a cleaner and hopefully more general way
       // (JMarc)
       outf += "\n";
       ++number_of_newlines;
     }
     if (mathed_env == LM_EN_DISPLAY){
       outf += "\\[\n";
     }
     else {
       outf += "\\begin{";
       outf += latex_mathenv[mathed_env];
       outf += "}\n";
     }
     ++number_of_newlines;
   }
   
   if (label && label[0]>' ' && mathed_env == LM_EN_EQUATION){
     outf += "\\label{";
     outf += label;
     outf += "}\n";
     ++number_of_newlines;
   }

   p->Write(outf);
   
   if (mathed_env == LM_EN_INTEXT){
     if (fragile) outf += "\\protect";
     outf += " \\)";
   }
   else if (mathed_env == LM_EN_DISPLAY){
     outf += "\\]\n";
     ++number_of_newlines;
   }
   else {
     outf += "\n\\end{";
     outf += latex_mathenv[mathed_env];
     outf += "}\n";
     number_of_newlines += 2;
   }
   *newlines = number_of_newlines;
}
#endif
