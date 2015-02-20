/**
 * \file lengthcommon.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Matthias Ettrich
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Length.h"

#include "support/convert.h"
#include "support/gettext.h"
#include "support/lassert.h"
#include "support/lstrings.h"

using namespace std;
using namespace lyx::support;


namespace lyx {

// I am not sure if "mu" should be possible to select (Lgb)

// the latex units
char const * const unit_name[] = {
	"bp", "cc", "cm", "dd", "em", "ex", "in", "mm", "mu",
	"pc", "pt", "sp",
	"text%",  "col%", "page%", "line%",
	"theight%", "pheight%", "" };

int const num_units = int(sizeof(unit_name) / sizeof(unit_name[0]) - 1);

// the LyX gui units
char const * const unit_name_gui[] = {
	N_("bp"), N_("cc[[unit of measure]]"), N_("cm"), N_("dd"), N_("em"),
	N_("ex"), N_("in[[unit of measure]]"), N_("mm"), N_("mu[[unit of measure]]"), N_("pc"),
	N_("pt"), N_("sp"), N_("Text Width %"),
	N_("Column Width %"), N_("Page Width %"), N_("Line Width %"),
	N_("Text Height %"), N_("Page Height %"), "" };


Length::UNIT unitFromString(string const & data)
{
	int i = 0;
	while (i < num_units && data != unit_name[i])
		++i;
	return static_cast<Length::UNIT>(i);
}


namespace {

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
 * @param number_index the current position in the number array
 * @param unit_index the current position in the unit array
 * @return a char representing the type of token returned
 *
 * The possible return values are :
 *	+	stretch indicator for glue length
 *	-	shrink indicator for glue length
 *	n	a numeric value (stored in number array)
 *	u	a unit type (stored in unit array)
 *	E	parse error
 */
char nextToken(string & data, double * number, int & number_index,
               Length::UNIT * unit, int & unit_index)
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

	if (data.empty()) {
		if (result)
			*result = GlueLength();
		return true;
	}
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

	// used to return numeric values in parsing vspace
	double number[4] = { 0, 0, 0, 0 };
	// used to return unit types in parsing vspace
	Length::UNIT unit[4] = {Length::UNIT_NONE, Length::UNIT_NONE,
	                        Length::UNIT_NONE, Length::UNIT_NONE};
	int number_index = 1; // entries at index 0 are sentinels
	int unit_index = 1;   // entries at index 0 are sentinels

	// construct "pattern" from "data"
	size_t const pattern_max_size = 20;
	string pattern;
	while (!isEndOfData(buffer)) {
		if (pattern.size() > pattern_max_size)
			return false;
		char const c = nextToken(buffer, number, number_index, unit,
				unit_index);
		if (c == 'E')
			return false;
		pattern.push_back(c);
	}

	// search "pattern" in "table"
	size_t table_index = 0;
	while (pattern != table[table_index].pattern) {
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
	if (data.empty()) {
		if (result)
			*result = Length();
		return true;
	}

	string buffer = data;

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

	// used to return numeric values in parsing vspace
	double number[4] = { 0, 0, 0, 0 };
	// used to return unit types in parsing vspace
	Length::UNIT unit[4] = {Length::UNIT_NONE, Length::UNIT_NONE,
	                        Length::UNIT_NONE, Length::UNIT_NONE};
	int number_index = 1; // entries at index 0 are sentinels
	int unit_index = 1;   // entries at index 0 are sentinels

	// construct "pattern" from "data"
	string pattern;
	while (!isEndOfData(buffer)) {
		if (pattern.size() > 2)
			return false;
		char const token = nextToken(buffer, number,
				number_index, unit, unit_index);
		if (token == 'E')
			return false;
		pattern += token;
	}

	// only the most basic pattern is accepted here
	if (pattern != "nu")
		return false;

	// It _was_ a correct length string.
	// Store away the values we found.
	if (result) {
		result->val_  = number[1] * val_sign;
		result->unit_ = unit[1];
	}
	return true;
}

} // namespace lyx
