/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-1999 The LyX Team.
 *
 * ======================================================*/

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "insetlatexaccent.h"
#include "debug.h"
#include "lyxrc.h"
#include "lyxdraw.h"
#include "support/lstrings.h"

extern LyXRC * lyxrc;

/* LatexAccent. Proper handling of accented characters */
/* This part is done by Ivan Schreter, schreter@ccsun.tuke.sk */
/* Later modified by Lars G. Bjonnes, larsbj@ifi.uio.no */

InsetLatexAccent::InsetLatexAccent()
{
	candisp = false;
}


InsetLatexAccent::InsetLatexAccent(InsetLatexAccent const & other)
    :contents(other.contents),
     candisp(other.candisp),
     modtype(other.modtype),
     remdot(other.remdot),
     plusasc(other.plusasc),
     plusdesc(other.plusdesc),
     ic(other.ic)
{}


InsetLatexAccent::InsetLatexAccent(string const & string)
	: contents(string)
{
	checkContents();
}


InsetLatexAccent::~InsetLatexAccent()
{
}


void InsetLatexAccent::checkContents()
        // check, if we know the modifier and can display it ok on screen
{
        candisp = false;

	if (contents.empty() || contents.length()<2) return;

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
		} else if (contents.length()==4 && contents[2] == ' ') {
			string tmp;
			tmp += contents[0];
			tmp += contents[1];
			tmp += '{';
			tmp += contents[3];
			tmp += '}';
			contents = tmp;
		} else if  (contents.length()==4 && contents[2] == '\\'
			    && (contents[3]== 'i' || contents[3]== 'j')) {
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

	lyxerr.debug() << "Decode: " << contents << endl;

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
 	        printf ("Default\n");
		// unknow accent (or something else)
                return;
        }

	// we demand that third char is a '{' (Lgb)
	if (contents[2] != '{') return;

	// special clause for \i{}, \j{} \l{} and \L{}
	if ((modtype == DOT_LESS_I || modtype == DOT_LESS_J
		|| modtype == lSLASH || modtype == LSLASH)
	    && contents[3] == '}' ) {
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
		lyxerr.debug() << "Contents: [" << contents << "]"
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
		} else if ( (ic=='i'|| ic=='j') && contents[4]=='}') {
		    // Do a rewrite: \<foo>{i} --> \<foo>{\i}
		    string temp=contents;
		    temp.erase(3, string::npos);
		    temp+='\\';
			temp+=char(ic);
		    for(string::size_type j = 4; j < contents.length(); ++j)
				temp+=contents[j];
		    contents=temp;
		    i++;
			remdot=true;
		}    

		// demand a '}' at the end
		if (contents[++i] != '}' && contents[++i]) return;
					   
		// fine, the char is properly decoded now (hopefully)
		lyxerr.debug() << "Contents: [" << contents << "]"
			       << ", ic: " << ic
			       << ", top: " << plusasc 
			       << ", bot: " << plusdesc 
			       << ", dot: " << remdot
			       << ", mod: " << modtype << endl;
	}
        candisp = true;
}


int InsetLatexAccent::Ascent(LyXFont const &font) const
{
	int max;
	if (candisp) {
		if (ic == ' ')
			max = font.ascent('a');
		else
			max = font.ascent(ic);
		if (plusasc) 
			max += (font.maxAscent()+3) / 3;
	} else
		max = font.maxAscent()+4;
	return max;
}


int InsetLatexAccent::Descent(LyXFont const &font) const
{
	int max;
	if (candisp) {
		if (ic==' ') 
			max = font.descent('a');
                else 
                	max = font.descent(ic);
                if (plusdesc)
                	max += 3;
	} else
		max = font.maxDescent() + 4;
	return max;
}


int InsetLatexAccent::Width(LyXFont const &font) const
{
	if (candisp)
		return font.textWidth(&ic, 1);
        else
                return font.stringWidth(contents) + 4;
}



bool InsetLatexAccent::DisplayISO8859_9(LyXFont font,
			    LyXScreen &scr,
			    int baseline, 
			    float &x)
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
		scr.drawText(font, &ch, 1, baseline, int(x));
		x += Width (font);
		return true;
	}
	else
		return false;
}


void InsetLatexAccent::Draw(LyXFont font,
			    LyXScreen &scr,
			    int baseline, 
			    float &x)
{
	if (lyxrc->font_norm=="iso8859-9")
		if (DisplayISO8859_9 (font, scr, baseline, x))	
			return;
	
	/* draw it! */ 
  
	if (candisp) {
		int asc = Ascent(font);
		int desc = Descent(font);
		int wid = Width(font);
		float x2 = x+(float(wid)/2);
		float hg35;
		int hg, y;
		if (plusasc) {
			// mark at the top
			hg = font.maxDescent();
			y = baseline - asc;

			if (font.shape() == LyXFont::ITALIC_SHAPE) x2 += (4*hg)/5; // italic
		} else {
			// at the bottom
			hg = desc;
			y = baseline;
		}

		hg35 = float(hg*3)/5;

		// display with proper accent mark
		// first the letter
		scr.drawText(font, &ic, 1, baseline, int(x));

		GC pgc = GetAccentGC(font, (hg+3)/5);

		if (remdot) {
			int tmpvar = baseline - font.ascent('i');
			float tmpx = 0;
			if (font.shape() == LyXFont::ITALIC_SHAPE) tmpx += (8*hg)/10; // italic
			lyxerr.debug() << "Removing dot." << endl;
			// remove the dot first
			scr.fillRectangle(gc_clear, int(x + tmpx),
					  tmpvar, wid,
					  font.ascent('i') -
					  font.ascent('x')-1);
			
		}
		// now the rest - draw within (x,y, x+wid, y+hg)
		switch (modtype) {
		case ACUTE:     // acute
 		{
			scr.drawLine(pgc, int(x2), int(y+hg35),
				     int(x2+hg35), y);
			break;
		}
		case GRAVE:     // grave
		{
			scr.drawLine(pgc,int(x2), int(y+hg35),
				     int(x2-hg35), y); 
			break;
		}
		case MACRON:     // macron
		{
			scr.drawLine(pgc,
				     int(x2-(3*wid/7)), int(y+(hg/2) + hg35),
				     int(x2+(3*wid/7)), int(y+(hg/2) + hg35));
			break;
		}
		case TILDE:     // tilde
		{
			if (hg35 > 2) --hg35;
			x2 += (hg35/2);
			XPoint p[4];
			p[0].x = int(x2 - 2*hg35); p[0].y = int(y + hg);
			p[1].x = int(x2 - hg35);   p[1].y = int(y + hg35);
			p[2].x = int(x2);          p[2].y = int(y + hg);
			p[3].x = int(x2 + hg35);   p[3].y = int(y + hg35);
			scr.drawLines(pgc, p, 4);
			break;
		}
		case UNDERBAR:     // underbar
		{
			scr.drawLine(pgc,
				     int(x2-(3*wid/7)), y+(hg/2),
				     int(x2+(3*wid/7)), y+(hg/2));
			break;
		}
		case CEDILLA:     // cedilla
		{
			XPoint p[4];
			p[0].x = int(x2);          p[0].y = y;
			p[1].x = int(x2);          p[1].y = y + (hg/3);
			p[2].x = int(x2 + (hg/3)); p[2].y = y + (hg/2);
			p[3].x = int(x2 - (hg/4)); p[3].y = y + hg;
			scr.drawLines(pgc, p, 4);
			break;
		}
		case UNDERDOT:     // underdot
		case DOT:    // dot
		{
			scr.drawArc(pgc,int(x2), y+(hg/2),
				    1, 1, 0, 360*64); 
			break;
		}
		case CIRCLE:     // circle
		{
			scr.drawArc(pgc, int(x2-(hg/2)), y, hg, hg, 0,
				    360*64);
			break;
		}
		case TIE:     // tie
		{
			scr.drawArc(pgc,
				    int(x2), y+(hg/4),
				    hg, hg, 0, 11519);
			break;
		}
		case BREVE:     // breve
		{
			scr.drawArc(pgc,
				    int(x2-(hg/2)), y-(hg/4),
				    hg, hg, 0, -11519);
			break;
		}
		case CARON:    // caron
		{
			XPoint p[3];
			p[0].x = int(x2 - hg35); p[0].y = y;
			p[1].x = int(x2);        p[1].y = int(y+hg35);
			p[2].x = int(x2 + hg35); p[2].y = y;
			scr.drawLines(pgc, p, 3);
			break;
		}
		case SPECIAL_CARON:    // special caron
		{
			switch (ic) {
			case 'L': wid = 4*wid/5; break;
			case 't': y -= int(hg35/2); break;
			}
			XPoint p[3];
			p[0].x = int(x+wid);          p[0].y = int(y+hg35+hg);
			p[1].x = int(x+wid+(hg35/2)); p[1].y = int(y+ hg+(hg35/2));
			p[2].x = int(x+wid+(hg35/2)); p[2].y = y + hg;
			scr.drawLines(pgc, p, 3);
			break;
		}
		case HUNGARIAN_UMLAUT:    // hung. umlaut
		{
			XSegment s[2];
			s[0].x1= int(x2-(hg/2));      s[0].y1 = int(y + hg35);
			s[0].x2= int(x2+hg35-(hg/2)); s[0].y2 = y;
			s[1].x1= int(x2+(hg/2));      s[1].y1 = int(y + hg35);
			s[1].x2= int(x2+hg35+(hg/2)); s[1].y2 = y;

			scr.drawSegments(pgc, s, 2);
			break;
		}
		case UMLAUT:    // umlaut
		{
			int tmpadd = y;
			tmpadd += (remdot) ? ((19*hg)/10) : ((20*hg)/27); // if (remdot) -> i or j
			int rad = ((hg*4)/8);
			if (rad <= 1) {
				scr.drawPoint(pgc, int(x2-((4*hg)/7)), tmpadd);
				scr.drawPoint(pgc, int(x2+((4*hg)/7)), tmpadd);
			} else {
				scr.drawArc(pgc, int(x2-((2*hg)/4)), tmpadd,
					    rad-1, rad-1, 0, 360*64);
				scr.drawArc(pgc, int(x2+((2*hg)/4)), tmpadd,
					    rad-1, rad-1, 0, 360*64);
			}
			break;
		}
		case CIRCUMFLEX:    // circumflex
		{
			XPoint p[3];
			p[0].x = int(x2 - hg35); p[0].y = y + hg;
			p[1].x = int(x2);        p[1].y = int(y + hg35);
			p[2].x = int(x2 + hg35); p[2].y = y + hg;
			scr.drawLines(pgc, p, 3);
			break;
		}
		case OGONEK:    // ogonek
		{
			// this does probably not look like an ogonek, so
			// it should certainly be refined
			XPoint p[4];
			p[0].x = int(x2);          p[0].y = y;
			p[1].x = int(x2);          p[1].y = y + (hg/3);
			p[2].x = int(x2 - (hg/3)); p[2].y = y + (hg/2);
			p[3].x = int(x2 + (hg/4)); p[3].y = y + hg;
			scr.drawLines(pgc, p, 4);
			break;
		}
		case lSLASH:
		case LSLASH:
		{
 			XPoint p[2];
 			p[0].x = int(x);                p[0].y = y+3*hg;
 			p[1].x = int(x+float(wid)*.75); p[1].y = y+hg;
 			scr.drawLines(pgc, p, 2);
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
		//scr.drawFilledRectangle(int(x+1), baseline - Ascent(font)+1,
		//			Width(font)-2,
		//			Ascent(font)+Descent(font)-2,
		//			FL_GRAY80);
		scr.fillRectangle(gc_lighted,
				  int(x+1), baseline - Ascent(font)+1,
				  Width(font)-2,
				  Ascent(font)+Descent(font)-2);
		
		//scr.drawRectangle(int(x), baseline - Ascent(font),
		//		  Width(font)-1,
		//		  Ascent(font)+Descent(font)-1,
		//		  FL_GRAY80);
		scr.drawRectangle(gc_lighted,
				  int(x), baseline - Ascent(font),
				  Width(font)-1,
				  Ascent(font)+Descent(font)-1);
		scr.drawString(font, contents, baseline, int(x+2));
	}
	x +=  Width(font);
}


void InsetLatexAccent::Write(FILE *file)
{
	fprintf(file, "\\i %s\n", contents.c_str());
}


void InsetLatexAccent::Read(LyXLex &lex)
{
	lex.EatLine();
	contents = lex.GetString();
	checkContents();
}


int InsetLatexAccent::Latex(FILE *file, signed char /*fragile*/)
{
	fprintf(file, "%s", contents.c_str());
	return 0;
}


int InsetLatexAccent::Latex(string &file, signed char /*fragile*/)
{
	file += contents;
	return 0;
}


int InsetLatexAccent::Linuxdoc(string &file)
{
	file += contents;
	return 0;
}


int InsetLatexAccent::DocBook(string &file)
{
	file += contents;
	return 0;
}


bool InsetLatexAccent::Deletable() const
{
	return true;
}


bool InsetLatexAccent::DirectWrite() const
{
	return true;
}


Inset* InsetLatexAccent::Clone()
{
	InsetLatexAccent *result = new InsetLatexAccent(contents);
	return result;
}


Inset::Code InsetLatexAccent::LyxCode() const
{
	return Inset::ACCENT_CODE;
}


bool InsetLatexAccent::IsEqual(Inset* other)
{
	if (other && other->LyxCode() == Inset::ACCENT_CODE){
		InsetLatexAccent* otheraccent = (InsetLatexAccent*) other;
		return (contents == otheraccent->contents);
	}
	return false;
}

ostream & operator<<(ostream & o, InsetLatexAccent::ACCENT_TYPES at)
{
	return o << int(at);
}
