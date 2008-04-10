/**
 * \file VSpace.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Matthias Ettrich
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "VSpace.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "support/gettext.h"
#include "Length.h"
#include "Text.h"
#include "TextMetrics.h" // for defaultRowHeight()

#include "support/convert.h"
#include "support/lstrings.h"

#include "support/assert.h"

#include <cstring>

using namespace std;
using namespace lyx::support;


namespace lyx {

namespace {

/// used to return numeric values in parsing vspace
double number[4] = { 0, 0, 0, 0 };

/// used to return unit types in parsing vspace
Length::UNIT unit[4] = {
	Length::UNIT_NONE,
	Length::UNIT_NONE,
	Length::UNIT_NONE,
	Length::UNIT_NONE
};

/// the current position in the number array
int number_index;
/// the current position in the unit array
int unit_index;

/// skip n characters of input
inline void lyx_advance(string & data, size_t n)
{
	data.erase(0, n);
}


/// return true when the input is at the end
inline bool isEndOfData(string const & data)
{
	return ltrim(data).empty();
}


/**
 * nextToken -  return the next token in the input
 * @param data input string
 * @return a char representing the type of token returned
 *
 * The possible return values are :
 *	+	stretch indicator for glue length
 *	-	shrink indicator for glue length
 *	n	a numeric value (stored in number array)
 *	u	a unit type (stored in unit array)
 *	E	parse error
 */
char nextToken(string & data)
{
	data = ltrim(data);

	if (data.empty())
		return '\0';

	if (data[0] == '+') {
		lyx_advance(data, 1);
		return '+';
	}

	if (prefixIs(data, "plus")) {
		lyx_advance(data, 4);
		return '+';
	}

	if (data[0] == '-') {
		lyx_advance(data, 1);
		return '-';
	}

	if (prefixIs(data, "minus")) {
		lyx_advance(data, 5);
		return '-';
	}

	size_t i = data.find_first_not_of("0123456789.");

	if (i != 0) {
		if (number_index > 3)
			return 'E';

		string buffer;

		// we have found some number
		if (i == string::npos) {
			buffer = data;
			i = data.size() + 1;
		} else {
			buffer = data.substr(0, i);
		}

		lyx_advance(data, i);

		if (isStrDbl(buffer)) {
			number[number_index] = convert<double>(buffer);
			++number_index;
			return 'n';
		}
		return 'E';
	}

	i = data.find_first_not_of("abcdefghijklmnopqrstuvwxyz%");
	if (i != 0) {
		if (unit_index > 3)
			return 'E';

		string buffer;

		// we have found some alphabetical string
		if (i == string::npos) {
			buffer = data;
			i = data.size() + 1;
		} else {
			buffer = data.substr(0, i);
		}

		// possibly we have "mmplus" string or similar
		if (buffer.size() > 5 &&
				(buffer.substr(2, 4) == string("plus") ||
				 buffer.substr(2, 5) == string("minus")))
		{
			lyx_advance(data, 2);
			unit[unit_index] = unitFromString(buffer.substr(0, 2));
		} else {
			lyx_advance(data, i);
			unit[unit_index] = unitFromString(buffer);
		}

		if (unit[unit_index] != Length::UNIT_NONE) {
			++unit_index;
			return 'u';
		}
		return 'E';  // Error
	}
	return 'E';  // Error
}


/// latex representation of a vspace
struct LaTeXLength {
	char const * pattern;
	int  plus_val_index;
	int  minus_val_index;
	int  plus_uni_index;
	int  minus_uni_index;
};


/// the possible formats for a vspace string
LaTeXLength table[] = {
	{ "nu",       0, 0, 0, 0 },
	{ "nu+nu",    2, 0, 2, 0 },
	{ "nu+nu-nu", 2, 3, 2, 3 },
	{ "nu+-nu",   2, 2, 2, 2 },
	{ "nu-nu",    0, 2, 0, 2 },
	{ "nu-nu+nu", 3, 2, 3, 2 },
	{ "nu-+nu",   2, 2, 2, 2 },
	{ "n+nu",     2, 0, 1, 0 },
	{ "n+n-nu",   2, 3, 1, 1 },
	{ "n+-nu",    2, 2, 1, 1 },
	{ "n-nu",     0, 2, 0, 1 },
	{ "n-n+nu",   3, 2, 1, 1 },
	{ "n-+nu",    2, 2, 1, 1 },
	{ "",         0, 0, 0, 0 }   // sentinel, must be empty
};


} // namespace anon

const char * stringFromUnit(int unit)
{
	if (unit < 0 || unit > num_units)
		return 0;
	return unit_name[unit];
}


bool isValidGlueLength(string const & data, GlueLength * result)
{
	// This parser is table-driven.  First, it constructs a "pattern"
	// that describes the sequence of tokens in "data".  For example,
	// "n-nu" means: number, minus sign, number, unit.  As we go along,
	// numbers and units are stored into static arrays.  Then, "pattern"
	// is searched in the "table".  If it is found, the associated
	// table entries tell us which number and unit should go where
	// in the Length structure.  Example: if "data" has the "pattern"
	// "nu+nu-nu", the associated table entries are "2, 3, 2, 3".
	// That means, "plus_val" is the second number that was seen
	// in the input, "minus_val" is the third number, and "plus_uni"
	// and "minus_uni" are the second and third units, respectively.
	// ("val" and "uni" are always the first items seen in "data".)
	// This is the most elegant solution I could find -- a straight-
	// forward approach leads to very long, tedious code that would be
	// much harder to understand and maintain. (AS)

	if (data.empty())
		return true;
	string buffer = ltrim(data);

	// To make isValidGlueLength recognize negative values as
	// the first number this little hack is needed:
	int val_sign = 1; // positive as default
	switch (buffer[0]) {
	case '-':
		lyx_advance(buffer, 1);
		val_sign = -1;
		break;
	case '+':
		lyx_advance(buffer, 1);
		break;
	default:
		break;
	}
	// end of hack

	int  pattern_index = 0;
	int  table_index = 0;
	char pattern[20];

	number_index = 1;
	unit_index = 1;  // entries at index 0 are sentinels

	// construct "pattern" from "data"
	while (!isEndOfData(buffer)) {
		if (pattern_index > 20)
			return false;
		pattern[pattern_index] = nextToken(buffer);
		if (pattern[pattern_index] == 'E')
			return false;
		++pattern_index;
	}
	pattern[pattern_index] = '\0';

	// search "pattern" in "table"
	table_index = 0;
	while (strcmp(pattern, table[table_index].pattern)) {
		++table_index;
		if (!*table[table_index].pattern)
			return false;
	}

	// Get the values from the appropriate places.  If an index
	// is zero, the corresponding array value is zero or UNIT_NONE,
	// so we needn't check this.
	if (result) {
		result->len_.value  (number[1] * val_sign);
		result->len_.unit   (unit[1]);
		result->plus_.value (number[table[table_index].plus_val_index]);
		result->plus_.unit  (unit  [table[table_index].plus_uni_index]);
		result->minus_.value(number[table[table_index].minus_val_index]);
		result->minus_.unit (unit  [table[table_index].minus_uni_index]);
	}
	return true;
}


bool isValidLength(string const & data, Length * result)
{
	// This is a trimmed down version of isValidGlueLength.
	// The parser may seem overkill for lengths without
	// glue, but since we already have it, using it is
	// easier than writing something from scratch.
	if (data.empty())
		return true;

	string   buffer = data;
	int      pattern_index = 0;
	char     pattern[3];

	// To make isValidLength recognize negative values
	// this little hack is needed:
	int val_sign = 1; // positive as default
	switch (buffer[0]) {
	case '-':
		lyx_advance(buffer, 1);
		val_sign = -1;
		break;
	case '+':
		lyx_advance(buffer, 1);
		// fall through
	default:
		// no action
		break;
	}
	// end of hack

	number_index = unit_index = 1;  // entries at index 0 are sentinels

	// construct "pattern" from "data"
	while (!isEndOfData(buffer)) {
		if (pattern_index > 2)
			return false;
		pattern[pattern_index] = nextToken(buffer);
		if (pattern[pattern_index] == 'E')
			return false;
		++pattern_index;
	}
	pattern[pattern_index] = '\0';

	// only the most basic pattern is accepted here
	if (strcmp(pattern, "nu") != 0)
		return false;

	// It _was_ a correct length string.
	// Store away the values we found.
	if (result) {
		result->val_  = number[1] * val_sign;
		result->unit_ = unit[1];
	}
	return true;
}


//
//  VSpace class
//

VSpace::VSpace()
	: kind_(DEFSKIP), len_(), keep_(false)
{}


VSpace::VSpace(VSpaceKind k)
	: kind_(k), len_(), keep_(false)
{}


VSpace::VSpace(Length const & l)
	: kind_(LENGTH), len_(l), keep_(false)
{}


VSpace::VSpace(GlueLength const & l)
	: kind_(LENGTH), len_(l), keep_(false)
{}


VSpace::VSpace(string const & data)
	: kind_(DEFSKIP), len_(), keep_(false)
{
	if (data.empty())
		return;

	string input = rtrim(data);

	size_t const length = input.length();

	if (length > 1 && input[length - 1] == '*') {
		keep_ = true;
		input.erase(length - 1);
	}

	if (prefixIs(input, "defskip"))
		kind_ = DEFSKIP;
	else if (prefixIs(input, "smallskip"))
		kind_ = SMALLSKIP;
	else if (prefixIs(input, "medskip"))
		kind_ = MEDSKIP;
	else if (prefixIs(input, "bigskip"))
		kind_ = BIGSKIP;
	else if (prefixIs(input, "vfill"))
		kind_ = VFILL;
	else if (isValidGlueLength(input, &len_))
		kind_ = LENGTH;
	else if (isStrDbl(input)) {
		// This last one is for reading old .lyx files
		// without units in added_space_top/bottom.
		// Let unit default to centimeters here.
		kind_ = LENGTH;
		len_  = GlueLength(Length(convert<double>(input), Length::CM));
	}
}


bool VSpace::operator==(VSpace const & other) const
{
	if (kind_ != other.kind_)
		return false;

	if (kind_ != LENGTH)
		return this->keep_ == other.keep_;

	if (len_ != other.len_)
		return false;

	return keep_ == other.keep_;
}


string const VSpace::asLyXCommand() const
{
	string result;
	switch (kind_) {
	case DEFSKIP:   result = "defskip";      break;
	case SMALLSKIP: result = "smallskip";    break;
	case MEDSKIP:   result = "medskip";      break;
	case BIGSKIP:   result = "bigskip";      break;
	case VFILL:     result = "vfill";        break;
	case LENGTH:    result = len_.asString(); break;
	}
	if (keep_)
		result += '*';
	return result;
}


string const VSpace::asLatexCommand(BufferParams const & params) const
{
	switch (kind_) {
	case DEFSKIP:
		return params.getDefSkip().asLatexCommand(params);

	case SMALLSKIP:
		return keep_ ? "\\vspace*{\\smallskipamount}" : "\\smallskip{}";

	case MEDSKIP:
		return keep_ ? "\\vspace*{\\medskipamount}" : "\\medskip{}";

	case BIGSKIP:
		return keep_ ? "\\vspace*{\\bigskipamount}" : "\\bigskip{}";

	case VFILL:
		return keep_ ? "\\vspace*{\\fill}" : "\\vfill{}";

	case LENGTH:
		return keep_ ? "\\vspace*{" + len_.asLatexString() + '}'
			: "\\vspace{" + len_.asLatexString() + '}';

	default:
		LASSERT(false, /**/);
		return string();
	}
}


docstring const VSpace::asGUIName() const
{
	docstring result;
	switch (kind_) {
	case DEFSKIP:
		result = _("Default skip");
		break;
	case SMALLSKIP:
		result = _("Small skip");
		break;
	case MEDSKIP:
		result = _("Medium skip");
		break;
	case BIGSKIP:
		result = _("Big skip");
		break;
	case VFILL:
		result = _("Vertical fill");
		break;
	case LENGTH:
		result = from_ascii(len_.asString());
		break;
	}
	if (keep_)
		result += ", " + _("protected");
	return result;
}


int VSpace::inPixels(BufferView const & bv) const
{
	// Height of a normal line in pixels (zoom factor considered)
	int const default_height = defaultRowHeight();

	switch (kind_) {

	case DEFSKIP:
		return bv.buffer().params().getDefSkip().inPixels(bv);

	// This is how the skips are normally defined by LateX.
	// But there should be some way to change this per document.
	case SMALLSKIP:
		return default_height / 4;

	case MEDSKIP:
		return default_height / 2;

	case BIGSKIP:
		return default_height;

	case VFILL:
		// leave space for the vfill symbol
		return 3 * default_height;

	case LENGTH:
		return len_.len().inPixels(bv.workWidth());

	default:
		LASSERT(false, /**/);
		return 0;
	}
}


} // namespace lyx
