/**
 * \file Bullet.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Allan Rae
 *
 * Full author contact details are available in file CREDITS.
 */

/* Completes the implementation of the Bullet class
 * It defines the various LaTeX commands etc. required to
 * generate the bullets in the bullet-panel's.
 */

#include <config.h>

#include "Bullet.h"

#include "support/lassert.h"

using namespace std;

namespace lyx {


/** The four LaTeX itemize environment default bullets
 */
extern
Bullet const ITEMIZE_DEFAULTS[4] = { Bullet(0, 8),//"\\(\\bullet\\)"
				     Bullet(0, 0),//"\\normalfont\\bfseries{--}"
				     Bullet(0, 6),//"\\(\\ast\\)"
				     Bullet(0, 10) };//"\\(\\cdot\\)"

// will need these later if still using full text as below
// \usepackage{latexsym,pifont,amssymb}
// and wasysym when that panel is created


Bullet::Bullet(int f, int c, int s)
	: font(f), character(c), size(s), user_text(0)
{
	if (f < MIN || f >= FONTMAX)
		font = MIN;
	if (c < MIN || c >= CHARMAX)
		character = MIN;
	if (s < MIN || s >= SIZEMAX)
		size = MIN;
	generateText();
	testInvariant();
}



Bullet::Bullet(docstring const & t)
	: font(MIN), character(MIN), size(MIN), user_text(1), text(t), label(t)
{
	testInvariant();
}


void Bullet::setCharacter(int c)
{
	if (c < MIN || c >= CHARMAX)
		character = MIN;
	else
		character = c;
	user_text = 0;
	testInvariant();
}


void Bullet::setFont(int f)
{
	if (f < MIN || f >= FONTMAX)
		font = MIN;
	else
		font = f;
	user_text = 0;
	testInvariant();
}


void Bullet::setSize(int s)
{
	if (s < MIN || s >= SIZEMAX)
		size = MIN;
	else
		size = s;
	user_text = 0;
	testInvariant();
}


void Bullet::setText(docstring const & t)
{
	font = character = size = MIN;
	user_text = 1;
	text = t;
	label = t;
	testInvariant();
}


int Bullet::getCharacter() const
{
	return character;
}


int Bullet::getFont() const
{
	return font;
}


int Bullet::getSize() const
{
	return size;
}


FontSize Bullet::getFontSize() const
{
	if (size >= 0)
		return bulletFontSize(size);
	else
		return INHERIT_SIZE;
}


Bullet & Bullet::operator=(Bullet const & b)
{
	b.testInvariant();
	font = b.font;
	character = b.character;
	size = b.size;
	user_text = b.user_text;
	text = b.text;
	label = b.label;
	this->testInvariant();
	return *this;
}


docstring const & Bullet::getText() const
{
	if (user_text == 0)
		generateText();
	return text;
}


docstring const & Bullet::getLabel() const
{
	if (user_text == 0)
		generateText();
	return label;
}


bool operator==(const Bullet & b1, const Bullet & b2)
{
	bool result = false;

	if (b1.user_text && b2.user_text) {
		/* both have valid text */
		if (b1.text == b2.text)
			result = true;
	} else if (b1.character == b2.character && b1.font == b2.font &&
			 b1.size == b2.size) {
		result = true;
	}
	return result;
}


/*--------------------Private Member Functions-------------------*/


void Bullet::generateText() const
{
	// Assumption:
	// user hasn't defined their own text and/or I haven't generated
	// the text for the current font/character settings yet
	// thus the calling member function should say:
	//    if (user_text == 0) {
	//       generateText();
	//    }
	// Since a function call is more expensive than a conditional
	// this is more efficient. Besides this function is internal to
	// the class so it's only the class author that has access --
	// external users thus can't make mistakes.

	if ((font >= 0) && (character >= 0)) {
		text = bulletEntry(font, character);
		label = bulletLabel(font, character);
		if (size >= 0)
			text = bulletSize(size) + text;
		user_text = -1;
		// text is now defined and doesn't need to be recalculated
		// unless font/character or text is modified
	}
}


docstring const Bullet::bulletSize(int s)
{
	// use a parameter rather than hard code `size' in here
	// in case some future function may want to retrieve
	// an arbitrary entry.
	// See additional comments in bulletEntry() below.

	static char const * BulletSize[SIZEMAX] = {
		"\\tiny",  "\\scriptsize", "\\footnotesize", "\\small", "\\normalsize",
		"\\large", "\\Large",      "\\LARGE",        "\\huge",  "\\Huge"
	};

	return from_ascii(BulletSize[s]);
}


docstring const Bullet::bulletEntry(int f, int c)
{
	// Despite how this may at first appear the static local variables
	// are only initialized once..
	// This is a work-around to avoid the "Static Initialization Problem"
	// and should work for all compilers. See "C++ FAQs" by Cline and Lomow,
	// Addison-Wesley, 1994, FAQ-180 pp169-171 for an explanation.
	// Doing things this way also makes it possible to generate `text' at
	// the time of construction.  It also encapsulates the conversion
	// of font, character and size entries to text.

	// The single 2-dim array had to be changed to multiple 1-dim arrays
	// to get around a compiler bug in an earler version of gcc (< 2.7.2.1)
	// static string const BulletPanels[FONTMAX][CHARMAX] = {
	static char const * BulletPanel0[CHARMAX] = {
		/* standard */
		"\\normalfont\\bfseries{--}", "\\(\\vdash\\)",
		"\\(\\dashv\\)", "\\(\\flat\\)", "\\(\\natural\\)",
		"\\(\\sharp\\)", "\\(\\ast\\)", "\\(\\star\\)",
		"\\(\\bullet\\)", "\\(\\circ\\)", "\\(\\cdot\\)",
		"\\(\\dagger\\)", "\\(\\bigtriangleup\\)",
		"\\(\\bigtriangledown\\)", "\\(\\triangleleft\\)",
		"\\(\\triangleright\\)", "\\(\\lhd\\)", "\\(\\rhd\\)",
		"\\(\\oplus\\)", "\\(\\ominus\\)", "\\(\\otimes\\)",
		"\\(\\oslash\\)", "\\(\\odot\\)", "\\(\\spadesuit\\)",
		"\\(\\diamond\\)", "\\(\\Diamond\\)", "\\(\\Box\\)",
		"\\(\\diamondsuit\\)", "\\(\\heartsuit\\)",
		"\\(\\clubsuit\\)", "\\(\\rightarrow\\)", "\\(\\leadsto\\)",
		"\\(\\rightharpoonup\\)", "\\(\\rightharpoondown\\)",
		"\\(\\Rightarrow\\)", "\\(\\succ\\)"
	};
	static char const * BulletPanel1[CHARMAX] = {
		/* amssymb */
		"\\(\\Rrightarrow\\)", "\\(\\rightarrowtail\\)",
		"\\(\\twoheadrightarrow\\)", "\\(\\rightsquigarrow\\)",
		"\\(\\looparrowright\\)", "\\(\\multimap\\)",
		"\\(\\boxtimes\\)", "\\(\\boxplus\\)", "\\(\\boxminus\\)",
		"\\(\\boxdot\\)", "\\(\\divideontimes\\)", "\\(\\Vvdash\\)",
		"\\(\\lessdot\\)", "\\(\\gtrdot\\)", "\\(\\maltese\\)",
		"\\(\\bigstar\\)", "\\(\\checkmark\\)", "\\(\\Vdash\\)",
		"\\(\\backsim\\)", "\\(\\thicksim\\)",
		"\\(\\centerdot\\)", "\\(\\circleddash\\)",
		"\\(\\circledast\\)", "\\(\\circledcirc\\)",
		"\\(\\vartriangleleft\\)", "\\(\\vartriangleright\\)",
		"\\(\\vartriangle\\)", "\\(\\triangledown\\)",
		"\\(\\lozenge\\)", "\\(\\square\\)", "\\(\\blacktriangleleft\\)",
		"\\(\\blacktriangleright\\)", "\\(\\blacktriangle\\)",
		"\\(\\blacktriangledown\\)", "\\(\\blacklozenge\\)",
		"\\(\\blacksquare\\)"
	};
	static char const * BulletPanel2[CHARMAX] = {
		/* psnfss1 */
		"\\ding{108}", "\\ding{109}",
		"\\ding{119}", "\\Pisymbol{psy}{197}",
		"\\Pisymbol{psy}{196}", "\\Pisymbol{psy}{183}",
		"\\ding{71}", "\\ding{70}",
		"\\ding{118}", "\\ding{117}",
		"\\Pisymbol{psy}{224}", "\\Pisymbol{psy}{215}",
		"\\ding{111}", "\\ding{112}",
		"\\ding{113}", "\\ding{114}",
		"\\Pisymbol{psy}{68}", "\\Pisymbol{psy}{209}",
		"\\ding{120}", "\\ding{121}",
		"\\ding{122}", "\\ding{110}",
		"\\ding{115}", "\\ding{116}",
		"\\Pisymbol{psy}{42}", "\\ding{67}",
		"\\ding{66}", "\\ding{82}",
		"\\ding{81}", "\\ding{228}",
		"\\ding{162}", "\\ding{163}",
		"\\ding{166}", "\\ding{167}",
		"\\ding{226}", "\\ding{227}"
	};
	static char const * BulletPanel3[CHARMAX] = {
		/* psnfss2 */
		"\\ding{37}", "\\ding{38}",
		"\\ding{34}", "\\ding{36}",
		"\\ding{39}", "\\ding{40}",
		"\\ding{41}", "\\ding{42}",
		"\\ding{43}", "\\ding{44}",
		"\\ding{45}", "\\ding{47}",
		"\\ding{53}", "\\ding{54}",
		"\\ding{59}", "\\ding{57}",
		"\\ding{62}", "\\ding{61}",
		"\\ding{55}", "\\ding{56}",
		"\\ding{58}", "\\ding{60}",
		"\\ding{63}", "\\ding{64}",
		"\\ding{51}", "\\ding{52}",
		"\\Pisymbol{psy}{170}", "\\Pisymbol{psy}{167}",
		"\\Pisymbol{psy}{168}", "\\Pisymbol{psy}{169}",
		"\\ding{164}", "\\ding{165}",
		"\\ding{171}", "\\ding{168}",
		"\\ding{169}", "\\ding{170}"
	};
	static char const * BulletPanel4[CHARMAX] = {
		/* psnfss3 */
		"\\ding{65}", "\\ding{76}",
		"\\ding{75}", "\\ding{72}",
		"\\ding{80}", "\\ding{74}",
		"\\ding{78}", "\\ding{77}",
		"\\ding{79}", "\\ding{85}",
		"\\ding{90}", "\\ding{98}",
		"\\ding{83}", "\\ding{84}",
		"\\ding{86}", "\\ding{87}",
		"\\ding{88}", "\\ding{89}",
		"\\ding{92}", "\\ding{91}",
		"\\ding{93}", "\\ding{105}",
		"\\ding{94}", "\\ding{99}",
		"\\ding{103}", "\\ding{104}",
		"\\ding{106}", "\\ding{107}",
		"\\ding{68}", "\\ding{69}",
		"\\ding{100}", "\\ding{101}",
		"\\ding{102}", "\\ding{96}",
		"\\ding{95}", "\\ding{97}"
	};
	static char const * BulletPanel5[CHARMAX] = {
		/* psnfss4 */
		"\\ding{223}", "\\ding{224}",
		"\\ding{225}", "\\ding{232}",
		"\\ding{229}", "\\ding{230}",
		"\\ding{238}", "\\ding{237}",
		"\\ding{236}", "\\ding{235}",
		"\\ding{234}", "\\ding{233}",
		"\\ding{239}", "\\ding{241}",
		"\\ding{250}", "\\ding{251}",
		"\\ding{49}", "\\ding{50}",
		"\\ding{217}", "\\ding{245}",
		"\\ding{243}", "\\ding{248}",
		"\\ding{252}", "\\ding{253}",
		"\\ding{219}", "\\ding{213}",
		"\\ding{221}", "\\ding{222}",
		"\\ding{220}", "\\ding{212}",
		"\\Pisymbol{psy}{174}", "\\Pisymbol{psy}{222}",
		"\\ding{254}", "\\ding{242}",
		"\\ding{231}", "\\Pisymbol{psy}{45}"
	};  /* string const BulletPanels[][] */

	static char const ** BulletPanels[FONTMAX] = {
		BulletPanel0, BulletPanel1,
		BulletPanel2, BulletPanel3,
		BulletPanel4, BulletPanel5
	};

	return from_ascii(BulletPanels[f][c]);
}


FontSize Bullet::bulletFontSize(int s)
{
	// see comment at bulletSize
	static FontSize BulletFontSize[SIZEMAX] = {
		TINY_SIZE, SCRIPT_SIZE, FOOTNOTE_SIZE, SMALL_SIZE, NORMAL_SIZE,
		LARGE_SIZE, LARGER_SIZE, LARGEST_SIZE, HUGE_SIZE, HUGER_SIZE
	};

	return BulletFontSize[s];
}


docstring const Bullet::bulletLabel(int f, int c)
{
	// see comment at bulletEntry
	static int UnicodeBulletPanel0[CHARMAX] = {
		/* standard */
		0x02013, 0x22A2,
		0x022A3, 0x0266D, 0x0266E,
		0x0266F, 0x02217, 0x022C6,
		0x02022, 0x02218, 0x022C5,
		0x02020, 0x025B3,
		0x025BD, 0x025C3,
		0x025B9, 0x025C1, 0x025B7,
		0x02295, 0x02296, 0x02297,
		0x02298, 0x02299, 0x02660,
		0x022C4, 0x025C7,  /* \square */ 0x025FB,
		0x02662, 0x02661,
		0x02663, 0x02192, 0x02933,
		0x021C0, 0x021C1,
		0x021D2, 0x0227B
	};
	static int UnicodeBulletPanel1[CHARMAX] = {
		/* amssymb */
		0x021DB, 0x021A3,
		0x021A0, 0x021DD,
		0x021AC, 0x022B8,
		0x022A0, 0x0229E, 0x0229F,
		0x022A1, 0x022C7, 0x022AA,
		0x022D6, 0x022D7, 0x02720,
		0x02605, 0x02713, 0x022A9,
		0x0223D, 0x0223C,
		0x02B1D, 0x0229D,
		0x0229B, 0x025CE,
		0x022B2, 0x022B3,
		0x025B3, 0x025BD,
		0x025CA, 0x025FB, 0x025C0,
		0x025B6, 0x025B4,
		0x025BE, 0x029EB,
		0x025FC
	};
	static int UnicodeBulletPanel2[CHARMAX] = {
		/* psnfss1 */
		0x025CF, 0x0274D,
		0x025D7, 0x02295,
		0x02297, 0x02022,
		0x02727, 0x02726,
		0x02756, 0x025C6,
		0x025CA, 0x022C5,
		0x02751, 0x02752,
		0x0274F, 0x02750,
		0x02206, 0x02207,
		0x02758, 0x02759,
		0x0275A, 0x025A0,
		0x025B2, 0x025BC,
		0x02217, 0x02723,
		0x02722, 0x02732,
		0x02731, 0x027A4,
		0x02762, 0x02763,
		0x02766, 0x02767,
		0x027A2, 0x027A3
	};
	static int UnicodeBulletPanel3[CHARMAX] = {
		/* psnfss2 */
		0x0260E, 0x02706,
		0x02702, 0x02704,
		0x02707, 0x02708,
		0x02709, 0x0261B,
		0x0261E, 0x0270C,
		0x0270D, 0x0270F,
		0x02715, 0x02716,
		0x0271B, 0x02719,
		0x0271E, 0x0271D,
		0x02717, 0x02718,
		0x0271A, 0x0271C,
		0x0271F, 0x02720,
		0x02713, 0x02714,
		0x02660, 0x02663,
		0x02666, 0x02665,
		0x02764, 0x02765,
		0x02660, 0x02663,
		0x02666, 0x02665
	};
	static int UnicodeBulletPanel4[CHARMAX] = {
		/* psnfss3 */
		0x02721, 0x0272C,
		0x0272B, 0x02B51,
		0x02730, 0x0272A,
		0x0272E, 0x0272D,
		0x0272F, 0x02735,
		0x0273A, 0x02742,
		0x02733, 0x02734,
		0x02736, 0x02737,
		0x02738, 0x02739,
		0x0273C, 0x0273B,
		0x0273D, 0x02749,
		0x0273E, 0x02743,
		0x02747, 0x02748,
		0x0274A, 0x0274B,
		0x02724, 0x02725,
		0x02744, 0x02745,
		0x02746, 0x02740,
		0x0273F, 0x02741
	};
	static int UnicodeBulletPanel5[CHARMAX] = {
		/* psnfss4 */
		0x0279F, 0x027A0,
		0x027A1, 0x027A8,
		0x027A5, 0x027A6,
		0x027AE, 0x027AD,
		0x027AC, 0x027AB,
		0x027AA, 0x027A9,
		0x027AF, 0x027B1,
		0x027BA, 0x027BB,
		0x02711, 0x02712,
		0x02799, 0x027B5,
		0x027B3, 0x027B8,
		0x027BC, 0x027BD,
		0x0279B, 0x02192,
		0x0279D, 0x0279E,
		0x0279C, 0x02794,
		0x02192, 0x021D2,
		0x027BE, 0x027B2,
		0x027A7, 0x02212
	};  /* string const BulletPanels[][] */

	static int * UnicodeBulletPanels[FONTMAX] = {
		UnicodeBulletPanel0, UnicodeBulletPanel1,
		UnicodeBulletPanel2, UnicodeBulletPanel3,
		UnicodeBulletPanel4, UnicodeBulletPanel5
	};

	return docstring(1, char_type(UnicodeBulletPanels[f][c]));
}


void Bullet::testInvariant() const
{
#ifdef ENABLE_ASSERTIONS
	LATTEST(font >= MIN);
	LATTEST(font < FONTMAX);
	LATTEST(character >= MIN);
	LATTEST(character < CHARMAX);
	LATTEST(size >= MIN);
	LATTEST(size < SIZEMAX);
	LATTEST(user_text >= -1);
	LATTEST(user_text <= 1);
	// now some relational/operational tests
	if (user_text == 1) {
		LATTEST(font == -1 && (character == -1 && size == -1));
		//        LATTEST(!text.empty()); // this isn't necessarily an error
	}
	//      else if (user_text == -1) {
	//        LATTEST(!text.empty()); // this also isn't necessarily an error
	//      }
	//      else {
	//        // user_text == 0
	//        LATTEST(text.empty()); // not usually true
	//      }
#endif
}


} // namespace lyx
