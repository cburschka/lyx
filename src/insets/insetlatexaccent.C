/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "insetlatexaccent.h"
#include "debug.h"
#include "lyxrc.h"
#include "support/lstrings.h"
#include "BufferView.h"
#include "Painter.h"
#include "font.h"
#include "language.h"

using std::ostream;
using std::endl;

/* LatexAccent. Proper handling of accented characters */
/* This part is done by Ivan Schreter, schreter@ccsun.tuke.sk */
/* Later modified by Lars G. Bjønnes, larsbj@lyx.org */

InsetLatexAccent::InsetLatexAccent()
	: candisp(false)
{}


InsetLatexAccent::InsetLatexAccent(string const & str)
	: contents(str)
{
	checkContents();
}


void InsetLatexAccent::checkContents()
        // check, if we know the modifier and can display it ok on screen
{
        candisp = false;

	if (contents.empty() || contents.length() < 2) return;

	// REMOVE IN 0.13
	// Dirty Hack for backward compability. remove in 0.13 (Lgb)
	contents = frontStrip(strip(contents));
	if (!contains(contents, "{") && !contains(contents, "}")) {
		if (contents.length() == 2) {
			string tmp;
			tmp += contents[0];
			tmp += contents[1];
			tmp += "{}";
			contents = tmp;
		} else if (contents.length() == 3) {
			string tmp;
			tmp += contents[0];
			tmp += contents[1];
			tmp += '{';
			tmp += contents[2];
			tmp += '}';
			contents = tmp;
		} else if (contents.length() == 4 && contents[2] == ' ') {
			string tmp;
			tmp += contents[0];
			tmp += contents[1];
			tmp += '{';
			tmp += contents[3];
			tmp += '}';
			contents = tmp;
		} else if  (contents.length() == 4 && contents[2] == '\\'
			    && (contents[3] == 'i' || contents[3] == 'j')) {
			string tmp;
			tmp += contents[0];
			tmp += contents[1];
			tmp += '{';
			tmp += contents[2];
			tmp += contents[3];
			tmp += '}';
			contents = tmp;
		}
	}
        if (contents[0] != '\\') return; // demand that first char is a '\\'

	lyxerr[Debug::KEY] << "Decode: " << contents << endl;

        remdot = false; plusasc = false; plusdesc = false;

        switch (contents[1]) { // second char should be one of these
	case '\'':  // acute
                modtype = ACUTE;    // acute
                plusasc = true;    // at the top of character
                break;
	case '`':   // grave
                modtype = GRAVE;    // grave
                plusasc = true;    // at the top
                break;
	case '=':   // macron
                modtype = MACRON;    // macron
                plusasc = true;    // at the top
                break;
	case '~':   // tilde
                modtype = TILDE;    // tilde
                plusasc = true;    // at the top
                break;
	case 'b':   // underbar
                modtype = UNDERBAR;    // underbar
                plusdesc = true;   // at the bottom
                break;
	case 'c':   // cedilla
                modtype = CEDILLA;    // cedilla
                plusdesc = true;   // at the bottom
                break;
	case 'd':   // underdot
                modtype = UNDERDOT;    // underdot
                plusdesc = true;   // at the bottom
                break;
	case 'r':   // circle
                modtype = CIRCLE;    // circle
                plusasc = true;    // at the top
                break;
	case 't':   // tie
                modtype = TIE;    // tie
                plusasc = true;    // at the top
                break;
	case 'u':   // breve
                modtype = BREVE;    // breve
                plusasc = true;    // at the top
                break;
	case 'v':   // caron
                modtype = CARON;   // caron
                plusasc = true;    // at the top
                break;
	case 'q':   // special caron
                modtype = SPECIAL_CARON;   // special caron
                plusasc = true;    // at the top
                break;
	case 'H':   // hungarian umlaut
                modtype = HUNGARIAN_UMLAUT;   // hungarian umlaut
                plusasc = true;    // at the top
                break;
	case '"':   // umlaut
                modtype = UMLAUT;   // umlaut
                plusasc = true;    // at the top
                break;
	case '.':   // dot
                modtype = DOT;   // dot
                plusasc = true;    // at the top
                break;
	case '^':   // circumflex
                modtype = CIRCUMFLEX;   // circumflex
                plusasc = true;    // at the top
                break;
	case 'k':   // ogonek
		modtype = OGONEK;  // ogonek
		plusdesc = true;
		break;
	case 'i': // dot-less-i
		modtype = DOT_LESS_I;  // dot-less-i
		plusasc = true; // at the top (not really needed)
		remdot = true;
		break;
	case 'j': // dot-less-j
		modtype = DOT_LESS_J; // dot-less-j
		plusasc = true; // at the top (not really needed)
		remdot = true;
		break;
 	case 'l': // lslash
 		modtype = lSLASH;
 		plusasc = true; // at the top (not really needed)
 		break;
 	case 'L': // lslash
 		modtype = LSLASH;
 		plusasc = true; // at the top (not really needed)
 		break;
	default:
		lyxerr[Debug::KEY] << "Default" << endl;
		// unknow accent (or something else)
                return;
        }

	// we demand that third char is a '{' (Lgb)
	if (contents[2] != '{') return;

	// special clause for \i{}, \j{} \l{} and \L{}
	if ((modtype == DOT_LESS_I || modtype == DOT_LESS_J
	     || modtype == lSLASH || modtype == LSLASH)
	    && contents[3] == '}') {
		switch (modtype) {
		case DOT_LESS_I: ic = 'i'; break;
		case DOT_LESS_J: ic = 'j'; break;
		case lSLASH:     ic = 'l'; break;
		case LSLASH:     ic = 'L'; break;
		default:
			// if this happens something is really wrong
			lyxerr << "InsetLaTexAccent: weird error." << endl;
			break;
		}
		//ic = (modtype == DOT_LESS_J ? 'j' : 'i');
		lyxerr[Debug::KEY] << "Contents: [" << contents << "]"
				   << ", ic: " << ic 
				   << ", top: " << plusasc 
				   << ", bot: " << plusdesc 
				   << ", dot: " << remdot 
				   << ", mod: " << modtype << endl;
		// Special case for space
	} else if (contents[3] == '}') {
		ic = ' ';
	} else {
		int i = 3;
		
		// now get the char
		ic = contents[3]; // i will always be 3 here

		// ic should now be a alfa-char or '\\'
		if (ic == '\\') {
			ic = contents[++i]; // will only allow \<foo>{\i} and \<foo>{\j}
			if (ic == 'i' || ic == 'j')
				remdot = true;
			else
				return;
		} else if ((ic == 'i'|| ic == 'j') && contents[4] == '}') {
			// Do a rewrite: \<foo>{i} --> \<foo>{\i}
			string temp = contents;
			temp.erase(3, string::npos);
			temp += '\\';
			temp += char(ic);
			for (string::size_type j = 4;
			    j < contents.length(); ++j)
				temp+= contents[j];
			contents= temp;
			++i;
			remdot = true;
		}    

		// demand a '}' at the end
		if (contents[++i] != '}' && contents[++i]) return;
					   
		// fine, the char is properly decoded now (hopefully)
		lyxerr[Debug::KEY] << "Contents: [" << contents << "]"
				   << ", ic: " << ic
				   << ", top: " << plusasc 
				   << ", bot: " << plusdesc 
				   << ", dot: " << remdot
				   << ", mod: " << modtype << endl;
	}
        candisp = true;
}


int InsetLatexAccent::ascent(BufferView *, LyXFont const & font) const
{
	// This function is a bit too simplistix and is just a
	// "try to make a fit for all accents" approach, to
	// make it better we need to know what kind of accent is
	// used and add to max based on that.
	int max;
	if (candisp) {
		if (ic == ' ')
			max = lyxfont::ascent('a', font);
		else
			max = lyxfont::ascent(ic, font);
		if (plusasc) 
			max += (lyxfont::maxAscent(font) + 3) / 3;
	} else
		max = lyxfont::maxAscent(font) + 4;
	return max;
}


int InsetLatexAccent::descent(BufferView *, LyXFont const & font) const
{
	int max;
	if (candisp) {
		if (ic == ' ') 
			max = lyxfont::descent('a', font);
                else
                	max = lyxfont::descent(ic, font);
                if (plusdesc)
                	max += 3;
	} else
		max = lyxfont::maxDescent(font) + 4;
	return max;
}


int InsetLatexAccent::width(BufferView *, LyXFont const & font) const
{
	if (candisp)
		return lyxfont::width(ic, font);
        else
                return lyxfont::width(contents, font) + 4;
}


int InsetLatexAccent::lbearing(LyXFont const & font) const
{
	return lyxfont::lbearing(ic, font);
}


int InsetLatexAccent::rbearing(LyXFont const & font) const
{
	return lyxfont::rbearing(ic, font);
}


bool InsetLatexAccent::displayISO8859_9(BufferView * bv, LyXFont const & font,
					int baseline, 
					float & x) const
{
	unsigned char tmpic = ic;
	
	switch (modtype) {
	case CEDILLA:
	{
		if (ic == 'c') tmpic = 0xe7;
		if (ic == 'C') tmpic = 0xc7;
		if (ic == 's') tmpic = 0xfe;
		if (ic == 'S') tmpic = 0xde;
		break;
	}
	case BREVE:
	{	if (ic == 'g') tmpic = 0xf0;
	if (ic == 'G') tmpic = 0xd0;
	break;
	}
	case UMLAUT:
	{
		if (ic == 'o') tmpic = 0xf6;
		if (ic == 'O') tmpic = 0xd6;
		if (ic == 'u') tmpic = 0xfc;
		if (ic == 'U') tmpic = 0xdc;
		break;
	}
	case DOT: 	 if (ic == 'I') tmpic = 0xdd; break;
	case DOT_LESS_I: tmpic = 0xfd; break;
	default:	 return false;
	}
	if (tmpic != ic) {
		char ch = char(tmpic);
		bv->painter().text(int(x), baseline, ch, font);
		x += width(bv, font);
		return true;
	}
	else
		return false;
}


void InsetLatexAccent::draw(BufferView * bv, LyXFont const & font0,
			    int baseline, float & x, bool) const
{
	Painter & pain = bv->painter();

	if (lyxrc.font_norm_type == LyXRC::ISO_8859_9)
		if (displayISO8859_9(bv, font0, baseline, x))	
			return;
	
	/* draw it! */ 
	// All the manually drawn accents in this function could use an
	// overhaul. Different ways of drawing (what metrics to use)
	// should also be considered.

	LyXFont font(font0);
	if (lyxrc.font_norm_type == LyXRC::ISO_10646_1)
		font.setLanguage(english_language);
	
	if (candisp) {
		int asc = ascent(bv, font);
		int desc = descent(bv, font);
		int wid = width(bv, font);
		float x2 = x + (rbearing(font) - lbearing(font)) / 2.0;
		float hg;
		int y;
		if (plusasc) {
			// mark at the top
			hg = lyxfont::maxDescent(font);
			y = baseline - asc;

			if (font.shape() == LyXFont::ITALIC_SHAPE)
				x2 += (4.0 * hg) / 5.0; // italic
		} else {
			// at the bottom
			hg = desc;
			y = baseline;
		}

		float hg35 = float(hg * 3.0) / 5.0;

		// display with proper accent mark
		// first the letter
		pain.text(int(x), baseline, ic, font);

		if (remdot) {
			int tmpvar = baseline - lyxfont::ascent('i', font);
			float tmpx = 0;
			if (font.shape() == LyXFont::ITALIC_SHAPE)
				tmpx += (8.0 * hg) / 10.0; // italic
			lyxerr[Debug::KEY] << "Removing dot." << endl;
			// remove the dot first
			pain.fillRectangle(int(x + tmpx), tmpvar, wid,
					   lyxfont::ascent('i', font) -
					   lyxfont::ascent('x', font) - 1,
					   backgroundColor());
			// the five lines below is a simple hack to
			// make the display of accent 'i' and 'j'
			// better. It makes the accent be written
			// closer to the top of the dot-less 'i' or 'j'.
			char tmpic = ic; // store the ic when we
			ic = 'x';        // calculates the ascent of
			asc = ascent(bv, font); // the dot-less version (here: 'x')
			ic = tmpic;      // set the orig ic back
			y = baseline - asc; // update to new y coord.
		}
		// now the rest - draw within (x, y, x+wid, y+hg)
		switch (modtype) {
		case ACUTE:     // acute 0xB4
 		{
			pain.text(int(x2 - (lyxfont::rbearing(0xB4, font) - lyxfont::lbearing(0xB4, font)) / 2),
				  baseline - lyxfont::ascent(ic, font) - lyxfont::descent(0xB4, font) - (lyxfont::ascent(0xB4, font) + lyxfont::descent(0xB4, font)) / 2,
				  char(0xB4), font);
			break;
		}
		case GRAVE:     // grave 0x60
		{
			pain.text(int(x2 - (lyxfont::rbearing(0x60, font) - lyxfont::lbearing(0x60, font)) / 2),
				  int(baseline - lyxfont::ascent(ic, font) - lyxfont::descent(0x60, font) - (lyxfont::ascent(0x60, font) + lyxfont::descent(0x60, font)) / 2.0),
				  char(0x60), font);
			break;
		}
		case MACRON:     // macron
		{
			pain.text(int(x2 - (lyxfont::rbearing(0xAF, font) - lyxfont::lbearing(0xAF, font)) / 2),
				  baseline - lyxfont::ascent(ic, font) - lyxfont::descent(0xAF, font) - (lyxfont::ascent(0xAF, font) + lyxfont::descent(0xAF, font)),
				  char(0xAF), font);
			break;
		}
		case TILDE:     // tilde
		{
			pain.text(int(x2 - (lyxfont::rbearing('~', font) - lyxfont::lbearing('~', font)) / 2),
				  baseline - lyxfont::ascent(ic, font) - lyxfont::descent('~', font) - (lyxfont::ascent('~', font) + lyxfont::descent('~', font)) / 2,
				  '~', font);
			break;
		}
		case UNDERBAR:     // underbar 0x5F
		{
			pain.text(int(x2 - (lyxfont::rbearing(0x5F, font) - lyxfont::lbearing(0x5F, font)) / 2), baseline,
				  char(0x5F), font);
			break;
		}
		case CEDILLA:     // cedilla
		{
			pain.text(int(x2 - (lyxfont::rbearing(0xB8, font) - lyxfont::lbearing(0xB8, font)) / 2), baseline,
				  char(0xB8), font);
			
			break;
		}
		case UNDERDOT:     // underdot
		{
			pain.text(int(x2 - (lyxfont::rbearing('.', font) - lyxfont::lbearing('.', font)) / 2.0),
				  int(baseline + 3.0 / 2.0 * (lyxfont::ascent('.', font) + lyxfont::descent('.', font))),
				  '.', font);
			break;
		}

		case DOT:    // dot
		{
			pain.text(int(x2 - (lyxfont::rbearing('.', font) - lyxfont::lbearing('.', font)) / 2.0),
				  baseline - lyxfont::ascent(ic, font) - lyxfont::descent('.', font) - (lyxfont::ascent('.', font) + lyxfont::descent('.', font)) / 2,
				  '.', font);
			break;
		}

		case CIRCLE:     // circle
		{
			LyXFont tmpf(font);
			tmpf.decSize().decSize();
			pain.text(int(x2 - (lyxfont::rbearing(0xB0, tmpf) - lyxfont::lbearing(0xB0, tmpf)) / 2.0),
				  int(baseline - lyxfont::ascent(ic, font) - lyxfont::descent(0xB0, tmpf) - (lyxfont::ascent(0xB0, tmpf) + lyxfont::descent(0xB0, tmpf)) / 3.0),
				  char(0xB0), tmpf);
			break;
		}
		case TIE:     // tie
		{
			pain.arc(int(x2 + hg35), int(y + hg / 2.0),
				 int(2 * hg), int(hg), 0, 360 * 32);
			break;
		}
		case BREVE:     // breve
		{
			pain.arc(int(x2 - (hg / 2.0)), y,
				 int(hg), int(hg), 0, -360*32);
			break;
		}
		case CARON:    // caron
		{
			int xp[3], yp[3];
			
			xp[0] = int(x2 - hg35); yp[0] = int(y + hg35);
			xp[1] = int(x2);        yp[1] = int(y + hg);
			xp[2] = int(x2 + hg35); yp[2] = int(y + hg35);
			pain.lines(xp, yp, 3);
			break;
		}
		case SPECIAL_CARON:    // special caron
		{
			switch (ic) {
			case 'L': wid = int(4.0 * wid / 5.0); break;
			case 't': y -= int(hg35 / 2.0); break;
			}
			int xp[3], yp[3];
			xp[0] = int(x + wid);
			yp[0] = int(y + hg35 + hg);
			
			xp[1] = int(x + wid + (hg35 / 2.0));
			yp[1] = int(y + hg + (hg35 / 2.0));
			
			xp[2] = int(x + wid + (hg35 / 2.0));
			yp[2] = y + int(hg);

			pain.lines(xp, yp, 3);
			break;
		}
		case HUNGARIAN_UMLAUT:    // hung. umlaut
		{
			pain.text(int(x2 - (lyxfont::rbearing('´', font) - lyxfont::lbearing('´', font))),
				  baseline - lyxfont::ascent(ic, font) - lyxfont::descent('´', font) - (lyxfont::ascent('´', font) + lyxfont::descent('´', font)) / 2,
				  '´', font);
			pain.text(int(x2),
				  baseline - lyxfont::ascent(ic, font) - lyxfont::descent('´', font) - (lyxfont::ascent('´', font) + lyxfont::descent('´', font)) / 2,
				  '´', font);
			break;
		}
		case UMLAUT:    // umlaut
		{
			pain.text(int(x2 - (lyxfont::rbearing('¨', font) - lyxfont::lbearing('¨', font)) / 2),
				  baseline - lyxfont::ascent(ic, font) - lyxfont::descent('¨', font) - ( lyxfont::ascent('¨', font) + lyxfont::descent('¨', font)) / 2,
				  '¨', font);
			break;
		}
		case CIRCUMFLEX:    // circumflex
		{
			LyXFont tmpf(font);
			tmpf.decSize().decSize().decSize();
			pain.text(int(x2 - (lyxfont::rbearing(0x5E, tmpf) - lyxfont::lbearing(0x5E, tmpf)) / 2),
				  int(baseline - lyxfont::ascent(ic, font) - lyxfont::descent(0x5E, tmpf) - (lyxfont::ascent(0x5E, tmpf) + lyxfont::descent(0x5E, tmpf)) / 3.0),
				  char(0x5E), tmpf);
			break;
		}
		case OGONEK:    // ogonek
		{
			// this does probably not look like an ogonek, so
			// it should certainly be refined
			int xp[4], yp[4];
			
			xp[0] = int(x2);
			yp[0] = y;
			
			xp[1] = int(x2);
			yp[1] = y + int(hg35);
			
			xp[2] = int(x2 - hg35);
			yp[2] = y + int(hg / 2.0);
			
			xp[3] = int(x2 + hg / 4.0);
			yp[3] = y + int(hg);

			pain.lines(xp, yp, 4);
			break;
		}
		case lSLASH:
		case LSLASH:
		{
			int xp[2], yp[2];
			
 			xp[0] = int(x);
			yp[0] = y + int(3.0 * hg);
			
 			xp[1] = int(x + float(wid) * 0.75);
			yp[1] = y + int(hg);
			
 			pain.lines(xp, yp, 2);
 			break;
		}
		case DOT_LESS_I: // dotless-i
		case DOT_LESS_J: // dotless-j
		{
			// nothing to do for these
			break;
		}
		}
	} else {
		pain.fillRectangle(int(x + 1),
				   baseline - ascent(bv, font) + 1,
				   width(bv, font) - 2,
				   ascent(bv, font)
				   + descent(bv, font) - 2,
				   backgroundColor());
		pain.rectangle(int(x + 1), baseline - ascent(bv, font) + 1,
			       width(bv, font) - 2,
			       ascent(bv, font) + descent(bv, font) - 2);
		pain.text(int(x + 2), baseline, contents, font);
	}
	x +=  width(bv, font);
}


void InsetLatexAccent::write(Buffer const *, ostream & os) const
{
	os << "\\i " << contents << "\n";
}


void InsetLatexAccent::read(Buffer const *, LyXLex & lex)
{
	lex.EatLine();
	contents = lex.GetString();
	checkContents();
}


int InsetLatexAccent::latex(Buffer const *, ostream & os,
			    bool /*fragile*/, bool/*fs*/) const
{
	os << contents;
	return 0;
}


int InsetLatexAccent::ascii(Buffer const *, ostream & os, int) const
{
	os << contents;
	return 0;
}


int InsetLatexAccent::linuxdoc(Buffer const *, ostream & os) const
{
	os << contents;
	return 0;
}


int InsetLatexAccent::docBook(Buffer const *, ostream & os) const
{
	os << contents;
	return 0;
}


bool InsetLatexAccent::deletable() const
{
	return true;
}


bool InsetLatexAccent::directWrite() const
{
	return true;
}


Inset * InsetLatexAccent::clone(Buffer const &, bool) const
{
	return new InsetLatexAccent(contents);
}


Inset::Code InsetLatexAccent::lyxCode() const
{
	return Inset::ACCENT_CODE;
}


ostream & operator<<(ostream & o, InsetLatexAccent::ACCENT_TYPES at)
{
	return o << int(at);
}
