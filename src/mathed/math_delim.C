 /* 
 *  File:        math_delim.C
 *  Purpose:     Draw delimiters and decorations
 *  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx> 
 *  Created:     January 1996
 *  Description: Vectorial fonts for simple and resizable objets.
 *
 *  Dependencies: Xlib, XForms
 *
 *  Copyright: 1996, Alejandro Aguilar Sierra
 *
 *   Version: 0.8beta, Mathed & Lyx project.
 *
 *   You are free to use and modify this code under the terms of
 *   the GNU General Public Licence version 2 or later.
 */

#include <config.h>

#include FORMS_H_LOCATION
#include <algorithm>
#include "symbol_def.h"
#include "math_inset.h"
#include "LColor.h"
#include "Painter.h"
#include "math_deliminset.h"
#include "mathed/support.h"

using std::sort;
using std::lower_bound;
using std::endl;

/* 
 * Internal struct of a drawing: code n x1 y1 ... xn yn, where code is:
 * 0 = end, 1 = line, 2 = polyline, 3 = square line, 4= square polyline
 */



//inline
//int odd(int x) { return ((x) & 1); }

//typedef float matriz_data[2][2];

//const matriz_data MATIDEN= { {1, 0}, {0, 1}};

//extern int mathed_char_width(short type, int style, byte c);
//extern int mathed_char_height(short, int, byte, int &, int &);

//#define mateq(m1, m2)  memcpy(m1, m2, sizeof(matriz_data))










// If we had exceptions we could return a reference in stead and not
// have to check for a null pointer in mathed_draw_deco


