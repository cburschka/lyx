// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-2000 The LyX Team.
 *
 *======================================================*/

#include <config.h>
#include FORMS_H_LOCATION

#ifdef __GNUG_
#pragma implementation
#endif

#include <algorithm> // max
#include <cmath> // floor
#include <fstream> // ofstream
#include "Color.h"
#include "lyxlex.h"

using std::max;
using std::min;
using std::ofstream;

static int const nohue = -1;

RGBColor::RGBColor(HSVColor const & hsv)
{
	double h = hsv.h;
	double const s = hsv.s;
	double const v = hsv.v;
	
	double rd, gd, bd;
	
	if (h == nohue || s == 0.0) {
		rd = gd = bd = v;
	} else {
		if (h == 360.0) h = 0.0;
		h /= 60.0;

		int const j = max(0, static_cast<int>(::floor(h)));
		//if( j < 0 ) j = 0;

		double const f = h - j;
		double const p = v * (1.0 - s);
		double const q = v * (1.0 - (s * f));
		double const t = v * (1.0 - (s * (1.0 - f)));

		switch (j) {
		case 0:
			rd = v;
			gd = t;
			bd = p;
			break;
		case 1:
			rd = q;
			gd = v;
			bd = p;
			break;
		case 2:
			rd = p;
			gd = v;
			bd = t;
			break;
		case 3:
			rd = p;
			gd = q;
			bd = v;
			break;
		case 4:
			rd = t;
			gd = p;
			bd = v;
			break;
		case 5:
			rd = v;
			gd = p;
			bd = q;
			break;
		default:
			rd = v;
			gd = t;
			bd = p;
			break;  // should never happen.
		}
	}

	r = static_cast<int>( ::floor((rd * 255.0) + 0.5) );
	g = static_cast<int>( ::floor((gd * 255.0) + 0.5) );
	b = static_cast<int>( ::floor((bd * 255.0) + 0.5) );
}


HSVColor::HSVColor(RGBColor const & rgb)
{
	// r, g, b lie in the range 0-1, not 0-255.
	double const r = rgb.r / 255.0;
	double const g = rgb.g / 255.0;
	double const b = rgb.b / 255.0;

	double const maxval = max( max( r, g ), b );
	double const minval = max( min( r, g ), b );

	v = maxval;

	double const diff = maxval - minval;
	if (maxval != 0.0)
		s = diff / maxval;
	else
		s = 0.0;

	h = nohue;
	if (s != 0.0) {
		double const rc = (maxval - r) / diff;
		double const gc = (maxval - g) / diff;
		double const bc = (maxval - b) / diff;

		if (r == maxval)
			h = bc - gc;
		else if (g == maxval)
			h = 2.0 + rc - bc;
		else if (b == maxval)
			h = 4.0 + gc - rc;

		h *= 60.0;
		if (h < 0)
			h += 360;
	}
}


// sorted by hand to prevent LyXLex from complaining on read().
static
keyword_item xformTags[] = {
//	{ "\\gui_active_tab", FL_LIGHTER_COL1 },
	{ "\\gui_background", FL_COL1 },
	{ "\\gui_buttonbottom", FL_BOTTOM_BCOL },
	{ "\\gui_buttonleft", FL_LEFT_BCOL },
	{ "\\gui_buttonright", FL_RIGHT_BCOL },
	{ "\\gui_buttontop", FL_TOP_BCOL },
	{ "\\gui_inactive", FL_INACTIVE },
	{ "\\gui_push_button", FL_YELLOW },
	{ "\\gui_selected", FL_MCOL },	
	{ "\\gui_text", FL_BLACK }
};


static const int xformCount = sizeof(xformTags) / sizeof(keyword_item);


bool XformColor::read(string const & filename)
{
	LyXLex lexrc(xformTags, xformCount);
	if (!lexrc.setFile(filename))
		return false;

	while (lexrc.IsOK()) {
		int const le = lexrc.lex();

		switch (le) {
		case LyXLex::LEX_UNDEF:
			lexrc.printError("Unknown tag `$$Token'");
			continue; 
		case LyXLex::LEX_FEOF:
			continue;
		default: break;
		}

		RGBColor col;

		if (!lexrc.next()) break;
		col.r = lexrc.GetInteger();

		if (!lexrc.next()) break;
		col.g = lexrc.GetInteger();

		if (!lexrc.next()) break;
		col.b = lexrc.GetInteger();

		fl_mapcolor(le, col.r, col.g, col.b);
	}
	
	return true;
}


bool XformColor::write(string const & filename)
{
	ofstream os(filename.c_str());
	if (!os)
		return false;

	os << "### This file is part of\n"
	   << "### ========================================================\n"
	   << "###          LyX, The Document Processor\n"
	   << "###\n"
	   << "###          Copyright 1995 Matthias Ettrich\n"
	   << "###          Copyright 1995-2000 The LyX Team.\n"
	   << "###\n"
	   << "### ========================================================\n"
	   << "\n"
	   << "# This file is written by LyX, if you want to make your own\n"
	   << "# modifications you should do them from inside LyX and save\n"
	   << "\n";

	for (int i = 0; i < xformCount; ++i) {
		string const tag  = xformTags[i].tag;
		int const colorID = xformTags[i].code;
		RGBColor color;

		fl_getmcolor(colorID, &color.r, &color.g, &color.b);

		os << tag << " "
		   << color.r << " " << color.g << " " << color.b << "\n";
	}

	return true;
}
