/*
 *  File:        math_draw.C
 *  Purpose:     Interaction and drawing for mathed
 *  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx> 
 *  Created:     January 1996
 *  Description: Math drawing and interaction for a WYSIWYG math editor.
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
#include "math_cursor.h"
#include "math_parser.h"
#include "debug.h"
#include "lyxfont.h"
#include "Painter.h"

using std::endl;

extern LyXFont const mathed_get_font(short type, int size);
extern int mathed_char_width(short type, int style, byte c);
extern int mathed_string_width(short type, int style, string const &);
extern int mathed_string_height(short, int, string const &, int &, int &);
extern int mathed_char_height(short, int, byte, int &, int &);






