/**
 * \file insetlatexaccent.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "insetlatexaccent.h"

#include "debug.h"
#include "language.h"
#include "LColor.h"
#include "lyxlex.h"
#include "lyxrc.h"
#include "metricsinfo.h"

#include "frontends/font_metrics.h"
#include "frontends/Painter.h"

#include "support/lstrings.h"

using lyx::support::contains;
using lyx::support::trim;

using std::endl;
using std::string;
using std::auto_ptr;
using std::ostream;


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


auto_ptr<InsetBase> InsetLatexAccent::clone() const
{
	return auto_ptr<InsetBase>(new InsetLatexAccent(contents));
}


void InsetLatexAccent::checkContents()
	// check, if we know the modifier and can display it ok on screen
{
	candisp = false;

	if (contents.empty() || contents.length() < 2) {
		lyxerr[Debug::KEY] << "Cannot decode: " << contents << endl;
		return;
	}

	// REMOVE IN 0.13
	// Dirty Hack for backward compability. remove in 0.13 (Lgb)
	contents = trim(contents);
	if (!contains(contents, '{') && !contains(contents, '}')) {
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
	if (contents[0] != '\\') { // demand that first char is a '\\'
		lyxerr[Debug::KEY] << "Cannot decode: " << contents << endl;
		return;
	}

	lyxerr[Debug::KEY] << "Decode: " << contents << endl;

	remdot = false;
	plusasc = false;
	plusdesc = false;

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
		lyxerr[Debug::KEY] << "Contents: [" << contents << ']'
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
		lyxerr[Debug::KEY] << "Contents: [" << contents << ']'
				   << ", ic: " << ic
				   << ", top: " << plusasc
				   << ", bot: " << plusdesc
				   << ", dot: " << remdot
				   << ", mod: " << modtype << endl;
	}
	candisp = true;
}


void InsetLatexAccent::metrics(MetricsInfo & mi, Dimension & dim) const
{
	LyXFont & font = mi.base.font;
	// This function is a bit too simplistic and is just a
	// "try to make a fit for all accents" approach, to
	// make it better we need to know what kind of accent is
	// used and add to max based on that.
	if (candisp) {
		if (ic == ' ')
			dim.asc = font_metrics::ascent('a', font);
		else
			dim.asc = font_metrics::ascent(ic, font);
		if (plusasc)
			dim.asc += (font_metrics::maxAscent(font) + 3) / 3;

		if (ic == ' ')
			dim.des = font_metrics::descent('a', font);
		else
			dim.des = font_metrics::descent(ic, font);
		if (plusdesc)
			dim.des += 3;

		dim.wid = font_metrics::width(ic, font);
	} else {
		dim.asc = font_metrics::maxAscent(font) + 4;
		dim.des = font_metrics::maxDescent(font) + 4;
		dim.wid = font_metrics::width(contents, font) + 4;
	}
	dim_ = dim;
}


int InsetLatexAccent::lbearing(LyXFont const & font) const
{
	return font_metrics::lbearing(ic, font);
}


int InsetLatexAccent::rbearing(LyXFont const & font) const
{
	return font_metrics::rbearing(ic, font);
}


bool InsetLatexAccent::displayISO8859_9(PainterInfo & pi, int x, int y) const
{
	unsigned char tmpic = ic;

	switch (modtype) {

	case CEDILLA: {
		if (ic == 'c') tmpic = 0xe7;
		if (ic == 'C') tmpic = 0xc7;
		if (ic == 's') tmpic = 0xfe;
		if (ic == 'S') tmpic = 0xde;
		break;
	}

	case BREVE: {
		if (ic == 'g') tmpic = 0xf0;
		if (ic == 'G') tmpic = 0xd0;
		break;
	}

	case UMLAUT: {
		if (ic == 'o') tmpic = 0xf6;
		if (ic == 'O') tmpic = 0xd6;
		if (ic == 'u') tmpic = 0xfc;
		if (ic == 'U') tmpic = 0xdc;
		break;
	}

	case DOT:
		if (ic == 'I') tmpic = 0xdd;
		break;

	case DOT_LESS_I:
		tmpic = 0xfd;
		break;

	default:
		return false;
	}

	if (tmpic == ic)
		return false;

	pi.pain.text(x, y, char(tmpic), pi.base.font);
	return true;
}


void InsetLatexAccent::drawAccent(PainterInfo const & pi, int x, int y,
	char accent) const
{
	LyXFont const & font = pi.base.font;
	x -= font_metrics::center(accent, font);
	y -= font_metrics::ascent(ic, font);
	y -= font_metrics::descent(accent, font);
	y -= font_metrics::height(accent, font) / 2;
	pi.pain.text(x, y, accent, font);
}


void InsetLatexAccent::draw(PainterInfo & pi, int x, int baseline) const
{
	if (lyxrc.font_norm_type == LyXRC::ISO_8859_9)
		if (displayISO8859_9(pi, x, baseline))
			return;

	// All the manually drawn accents in this function could use an
	// overhaul. Different ways of drawing (what metrics to use)
	// should also be considered.

	LyXFont font = pi.base.font;
	if (lyxrc.font_norm_type == LyXRC::ISO_10646_1)
		font.setLanguage(english_language);

	if (candisp) {
		int x2 = int(x + (rbearing(font) - lbearing(font)) / 2);
		int hg;
		int y;
		if (plusasc) {
			// mark at the top
			hg = font_metrics::maxDescent(font);
			y = baseline - dim_.asc;
			if (font.shape() == LyXFont::ITALIC_SHAPE)
				x2 += int(0.8 * hg); // italic
		} else {
			// at the bottom
			hg = dim_.des;
			y = baseline;
		}

		double hg35 = hg * 0.6;

		// display with proper accent mark
		// first the letter
		pi.pain.text(x, baseline, ic, font);

		if (remdot) {
			int tmpvar = baseline - font_metrics::ascent('i', font);
			int tmpx = 0;
			if (font.shape() == LyXFont::ITALIC_SHAPE)
				tmpx += int(0.8 * hg); // italic
			lyxerr[Debug::KEY] << "Removing dot." << endl;
			// remove the dot first
			pi.pain.fillRectangle(x + tmpx, tmpvar, dim_.wid,
					   font_metrics::ascent('i', pi.base.font) -
					   font_metrics::ascent('x', pi.base.font) - 1,
					   backgroundColor());
			// the five lines below is a simple hack to
			// make the display of accent 'i' and 'j'
			// better. It makes the accent be written
			// closer to the top of the dot-less 'i' or 'j'.
			char tmpic = ic; // store the ic when we
			ic = 'x';        // calculates the ascent of
#warning metrics?
			int asc = ascent(); // the dot-less version (here: 'x')
			ic = tmpic;      // set the orig ic back
			y = baseline - asc; // update to new y coord.
		}

		// now the rest - draw within (x, y, x + wid, y + hg)
		switch (modtype) {
		case ACUTE:
			drawAccent(pi, x2, baseline, char(0xB4));
			break;

		case GRAVE:
			drawAccent(pi, x2, baseline, char(0x60));
			break;

		case MACRON:
			drawAccent(pi, x2, baseline, char(0xAF));
			break;

		case TILDE:
			drawAccent(pi, x2, baseline, '~');
			break;

		case UNDERBAR:     // underbar 0x5F
			pi.pain.text(x2 - font_metrics::center(0x5F, font), baseline,
				  char(0x5F), font);
			break;

		case CEDILLA:
			pi.pain.text(x2  - font_metrics::center(0xB8, font), baseline,
				  char(0xB8), font);
			break;

		case UNDERDOT:
			pi.pain.text(x2  - font_metrics::center('.', font),
				  int(baseline + 1.5 * font_metrics::height('.', font)),
				  '.', font);
			break;

		case DOT:
			drawAccent(pi, x2, baseline, '.');
			break;

		case CIRCLE:
			drawAccent(pi, x2, baseline, char(0xB0));
			break;

		case TIE:
			pi.pain.arc(int(x2 + hg35), y + hg / 2, 2 * hg, hg, 0, 360 * 32,
				    LColor::foreground);
			break;

		case BREVE:
			pi.pain.arc(int(x2 - hg / 2), y, hg, hg, 0, -360*32,
				    LColor::foreground);
			break;

		case CARON: {
			int xp[3], yp[3];
			xp[0] = int(x2 - hg35);    yp[0] = int(y + hg35);
			xp[1] = int(x2);           yp[1] = int(y + hg);
			xp[2] = int(x2 + hg35);    yp[2] = int(y + hg35);
			pi.pain.lines(xp, yp, 3, LColor::foreground);
			break;
		}

		case SPECIAL_CARON: {
			switch (ic) {
				case 'L': dim_.wid = int(4.0 * dim_.wid / 5.0); break;
				case 't': y -= int(hg35 / 2.0); break;
			}
			int xp[3], yp[3];
			xp[0] = int(x + dim_.wid);
			yp[0] = int(y + hg35 + hg);

			xp[1] = int(x + dim_.wid + (hg35 / 2.0));
			yp[1] = int(y + hg + (hg35 / 2.0));

			xp[2] = int(x + dim_.wid + (hg35 / 2.0));
			yp[2] = y + int(hg);

			pi.pain.lines(xp, yp, 3, LColor::foreground);
			break;
		}

		case HUNGARIAN_UMLAUT:
			drawAccent(pi, x2 - font_metrics::center('´', font), baseline, '´');
			drawAccent(pi, x2 + font_metrics::center('´', font), baseline, '´');
			break;

		case UMLAUT:
			drawAccent(pi, x2, baseline, '"');
			break;

		case CIRCUMFLEX:
			drawAccent(pi, x2, baseline, 0x5E);
			break;

		case OGONEK: {
			// this does probably not look like an ogonek, so
			// it should certainly be refined
			int xp[4], yp[4];

			xp[0] = x2;
			yp[0] = y;

			xp[1] = x2;
			yp[1] = y + int(hg35);

			xp[2] = int(x2 - hg35);
			yp[2] = y + hg / 2;

			xp[3] = x2 + hg / 4;
			yp[3] = y + int(hg);

			pi.pain.lines(xp, yp, 4, LColor::foreground);
			break;
		}

		case lSLASH:
		case LSLASH: {
			int xp[2], yp[2];

			xp[0] = x;
			yp[0] = y + int(3 * hg);

			xp[1] = int(x + dim_.wid * 0.75);
			yp[1] = y + int(hg);

			pi.pain.lines(xp, yp, 2, LColor::foreground);
			break;
		}

		case DOT_LESS_I: // dotless-i
		case DOT_LESS_J: // dotless-j
			// nothing to do for these
			break;
		}

	} else {
		pi.pain.fillRectangle(x + 1,
				      baseline - dim_.asc + 1, dim_.wid - 2,
				      dim_.asc + dim_.des - 2,
				      backgroundColor());
		pi.pain.rectangle(x + 1, baseline - dim_.asc + 1,
				  dim_.wid - 2, dim_.asc + dim_.des - 2,
				  LColor::foreground);
		pi.pain.text(x + 2, baseline, contents, font);
	}
}


void InsetLatexAccent::write(Buffer const &, ostream & os) const
{
	os << "\\i " << contents << "\n";
}


void InsetLatexAccent::read(Buffer const &, LyXLex & lex)
{
	lex.eatLine();
	contents = lex.getString();
	checkContents();
}


int InsetLatexAccent::latex(Buffer const &, ostream & os,
			    OutputParams const &) const
{
	os << contents;
	return 0;
}


int InsetLatexAccent::plaintext(Buffer const &, ostream & os,
			    OutputParams const &) const
{
	os << contents;
	return 0;
}


int InsetLatexAccent::linuxdoc(Buffer const &, ostream & os,
			       OutputParams const &) const
{
	os << contents;
	return 0;
}


int InsetLatexAccent::docbook(Buffer const &, ostream & os,
			      OutputParams const &) const
{
	os << contents;
	return 0;
}


bool InsetLatexAccent::directWrite() const
{
	return true;
}


InsetOld::Code InsetLatexAccent::lyxCode() const
{
	return InsetOld::ACCENT_CODE;
}


ostream & operator<<(ostream & o, InsetLatexAccent::ACCENT_TYPES at)
{
	return o << int(at);
}
