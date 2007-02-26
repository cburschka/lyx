// -*- C++ -*-
/**
 * \file textutils.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Georg Baum
 *
 * Full author contact details are available in file CREDITS.
 */

// FIXME: I can think of a better name for this file ...

#include <config.h>

#include "support/textutils.h"
#include "support/qstring_helpers.h"

namespace lyx {

namespace {
	/// Maximum valid UCS4 code point
	char_type const ucs4_max = 0x10ffff;
}


bool isLetterChar(char_type c)
{
	if (!is_utf16(c)) {
		if (c > ucs4_max)
			// outside the UCS4 range
			return false;
		// assume that all non-utf16 characters are letters
		return true;
	}
	return ucs4_to_qchar(c).isLetter();
}


bool isPrintable(char_type c)
{
	if (!is_utf16(c)) {
		if (c > ucs4_max)
			// outside the UCS4 range
			return false;
		// assume that all non-utf16 characters are printable
		return true;
	}
	return ucs4_to_qchar(c).isPrint();
}


bool isPrintableNonspace(char_type c)
{
	if (!is_utf16(c)) {
		if (c > ucs4_max)
			// outside the UCS4 range
			return false;
		// assume that all non-utf16 characters are printable and
		// no space
		return true;
	}
	QChar const qc = ucs4_to_qchar(c);
	return qc.isPrint() && !qc.isSpace();
}


bool isDigit(char_type c)
{
	if (!is_utf16(c))
		// assume that no non-utf16 character is a digit
		// c outside the UCS4 range is catched as well
		return false;
	return ucs4_to_qchar(c).isDigit();
}

} // namespace lyx
