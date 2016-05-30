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

#include "support/docstring.h"
#include "support/lassert.h"

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


} // namespace lyx
