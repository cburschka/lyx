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
#include "math_parinset.h"
#include "mathed/support.h"
#include "support/lstrings.h"
#include "debug.h"

using std::ostream;
using std::endl;

extern char const * latex_mathenv[];

// quite a hack i know. Should be done with return values...
int number_of_newlines = 0;


void mathed_write(MathParInset * p, ostream & os, int * newlines,
		  bool fragile, string const & label)
{
   number_of_newlines = 0;
   short mathed_env = p->GetType();

   if (mathed_env == LM_OT_MIN) {
	   if (fragile) os << "\\protect";
	   os << "\\( "; // changed from " \\( " (Albrecht Dress)
   } 
   else {
     if (mathed_env == LM_OT_PAR){
	     os << "\\[\n";
     } else {
	     os << "\\begin{"
		<< latex_mathenv[mathed_env]
		<< "}";
	     if (is_multicolumn(mathed_env)) {
		     if (mathed_env != LM_OT_ALIGNAT
			 && mathed_env != LM_OT_ALIGNATN)
			     os << "%";
		     os << "{" << p->GetColumns()/2 << "}";
	     }
	     os << "\n";
     }
     ++number_of_newlines;
   }
   
   if (!label.empty() && label[0] > ' ' && is_singlely_numbered(mathed_env)) {
	   os << "\\label{"
	      << label
	      << "}\n";
     ++number_of_newlines;
   }

   p->Write(os, fragile);
   
   if (mathed_env == LM_OT_MIN){
	   if (fragile) os << "\\protect";
	   os << " \\)";
   }
   else if (mathed_env == LM_OT_PAR) {
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
