/*
 *  File:        math_inset.C
 *  Purpose:     Implementation of insets for mathed
 *  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx> 
 *  Created:     January 1996
 *  Description: 
 *
 *  Dependencies: Xlib, XForms
 *
 *  Copyright: 1996, 1997 Alejandro Aguilar Sierra
 *
 *   Version: 0.8beta.
 *
 *   You are free to use and modify this code under the terms of
 *   the GNU General Public Licence version 2 or later.
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_iter.h"
#include "math_inset.h"
#include "symbol_def.h"
#include "lyxfont.h"
#include "mathed/support.h"
#include "Painter.h"

int MathedInset::df_asc;
int MathedInset::df_des;
int MathedInset::df_width;
int MathedInset::workWidth;


MathedInset::MathedInset(MathedInset * inset) 
{
   if (inset) {
      name = inset->GetName();
      objtype = inset->GetType();
      size = inset->GetStyle();
      width = inset->Width();
      ascent = inset->Ascent();
      descent = inset->Descent();
   } else {
      objtype = LM_OT_UNDEF;
      size = LM_ST_TEXT;
      width = ascent = descent = 0;
      //name = 0;
   }
}


MathedInset::MathedInset(string const & nm, short ot, short st):
  name(nm), objtype(ot), size(st) 
{
   width = ascent = descent = 0;
}


// In a near future maybe we use a better fonts renderer
void MathedInset::drawStr(Painter & pain, short type, int siz,
			  int x, int y, string const & s)
{
	string st;
	if (MathIsBinary(type))
		for (string::const_iterator it = s.begin(); it != s.end(); ++it) {
			st += ' ';
			st += *it;
			st += ' ';
		}
	else
		st = s;

	LyXFont const mf = mathed_get_font(type, siz);
	pain.text(x, y, st, mf);
}

