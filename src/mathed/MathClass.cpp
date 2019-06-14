/**
 * \file MathClass.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "MathClass.h"
#include "MathSupport.h"

#include "MetricsInfo.h"
#include "FontInfo.h"

#include "support/debug.h"
#include "support/docstring.h"
#include "support/lassert.h"

#include <ostream>

using namespace std;

namespace lyx {


docstring const class_to_string(MathClass const mc)
{
	string s;
	switch (mc) {
	case MC_ORD:
		s = "mathord";
		break;
	case MC_OP:
		s = "mathop";
		break;
	case MC_BIN:
		s = "mathbin";
		break;
	case MC_REL:
		s = "mathrel";
		break;
	case MC_OPEN:
		s = "mathopen";
		break;
	case MC_CLOSE:
		s = "mathclose";
		break;
	case MC_PUNCT:
		s = "mathpunct";
		break;
	case MC_INNER:
		s = "mathinner";
		break;
	case MC_UNKNOWN:
		LATTEST(false);
		s = "mathord";
	}
	return from_ascii(s);
}


MathClass string_to_class(docstring const &s)
{
	if (s  == "mathop")
		return MC_OP;
	else if (s  == "mathbin")
		return MC_BIN;
	else if (s  == "mathrel")
		return MC_REL;
	else if (s  == "mathopen")
		return MC_OPEN;
	else if (s  == "mathclose")
		return MC_CLOSE;
	else if (s  == "mathpunct")
		return MC_PUNCT;
	else if (s  == "mathinner")
		return MC_INNER;
	else  if (s  == "mathord")
		return MC_ORD;
	else
		return MC_UNKNOWN;
}


/*
 * The TeXbook presents in Appendix G a set of 22 rules (!) explaining
 * how to typeset mathematic formulas. Of interest here are rules 5
 * and 6:

 * 5. If the current item is a Bin atom, and if this was the first
 *    atom in the list, or if the most recent previous atom was Bin,
 *    Op, Rel, Open, or Punct, change the current Bin to Ord [and
 *    continue with Rule 14. Otherwise continue with Rule 17]
 *
 * 6. If the current item is a Rel or Close or Punct atom, and if the
 *    most recent previous atom was Bin, change that previous Bin to
 *    Ord. [Continue with Rule 17.]
 */
void update_class(MathClass & mc, MathClass const prev, MathClass const next)
{
	if (mc == MC_BIN
		&& (prev == MC_BIN || prev == MC_OP || prev == MC_OPEN
			|| prev == MC_PUNCT || prev == MC_REL
			|| next == MC_CLOSE || next == MC_PUNCT || next == MC_REL))
		mc = MC_ORD;
}


/*
 * This table of spacing between two classes can be found p. 170 of
 * The TeXbook.
 *
 * The line is the class of the first class, and the column the second
 * class. The number encodes the spacing between the two atoms, as
 * follows
 *
 * + 0: no spacing
 * + 1: thin mu skip
 * + 2: med mu skip
 * + 3: thick mu skip
 * + 9: should never happen
 * + negative value: either 0 if the atom is in script or scriptscript mode,
 *   or the spacing given by the absolute value.
 */
int pair_spc[MC_UNKNOWN][MC_UNKNOWN] = {
//	 ORD    OP   BIN   REL  OPEN CLOSE PUNCT INNER
	{  0,    1,   -2,   -3,    0,    0,    0,   -1}, // ORD
	{  1,    1,    9,   -3,    0,    0,    0,   -1}, // OP
	{ -2,   -2,    9,    9,   -2,    9,    9,   -2}, // BIN
	{ -3,   -3,    9,    0,   -3,    0,    0,   -3}, // REL
	{  0,    0,    9,    0,    0,    0,    0,    0}, // OPEN
	{  0,    1,   -2,   -3,    0,    0,    0,   -1}, // CLOSE
	{ -1,   -1,    9,   -1,   -1,   -1,   -1,   -1}, // PUNCT
	{ -1,    1,   -2,   -3,   -1,    0,   -1,   -1}, // INNER
};


int class_spacing(MathClass const mc1, MathClass const mc2,
                  MetricsBase const & mb)
{
	int spc_code = pair_spc[mc1][mc2];
	//lyxerr << class_to_string(mc1) << "+" << class_to_string(mc2)
	//	   << "=" << spc_code << " @" << mb.style << endl;
	if (spc_code < 0) {
		switch (mb.font.style()) {
		case LM_ST_DISPLAY:
		case LM_ST_TEXT:
		case LM_ST_IGNORE:
		case LM_ST_INHERIT:
			spc_code = abs(spc_code);
			break;
		case LM_ST_SCRIPT:
		case LM_ST_SCRIPTSCRIPT:
			spc_code = 0;
		}
	}

	int spc = 0;
	switch(spc_code) {
	case 0:
		break;
	case 1:
		spc = mathed_thinmuskip(mb.font);
		break;
	case 2:
		spc = mathed_medmuskip(mb.font);
		break;
	case 3:
		spc = mathed_thickmuskip(mb.font);
		break;
	default:
		LYXERR0("Impossible pair of classes: (" << mc1 << ", " << mc2 << ")");
		LATTEST(false);
	}
	return spc;
}

} // namespace lyx
