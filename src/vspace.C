/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation "vspace.h"
#endif

#include <stdio.h>

#include "lyx_main.h"
#include "buffer.h"
#include "vspace.h"
#include "lyxrc.h"
#include "lyxtext.h"
#include "BufferView.h"
#include "support/lstrings.h"


namespace {

/*  length units
 */

int const num_units = LyXLength::UNIT_NONE;

// I am not sure if "mu" should be possible to select (Lgb)
char const * unit_name[num_units] = { "sp", "pt", "bp", "dd",
				      "mm", "pc", "cc", "cm",
				      "in", "ex", "em", "mu",
				      "%",  "c%", "p%", "l%" }; 


/*  The following static items form a simple scanner for
 *  length strings, used by isValid[Glue]Length.  See comments there.
 */
float           number[4] = { 0, 0, 0, 0 };
LyXLength::UNIT unit[4]   = { LyXLength::UNIT_NONE,
			      LyXLength::UNIT_NONE,
			      LyXLength::UNIT_NONE,
			      LyXLength::UNIT_NONE };
int number_index;
int unit_index;


inline
void lyx_advance(string & data, unsigned int n)
{
	data.erase(0, n);
}


inline
bool isEndOfData(string const & data)
{
	return frontStrip(data).empty();
}


char nextToken(string & data)
{
	data = frontStrip(data);
	if (data.empty())
		return '\0';
	else if (data[0] == '+') {
		lyx_advance(data, 1);
		return '+';
	} else if (prefixIs(data, "plus")) {
		lyx_advance(data, 4);
		return '+';
	} else if (data[0] == '-') {
		lyx_advance(data, 1);
		return '-';
	} else if (prefixIs(data, "minus")) {
		lyx_advance(data, 5);
		return '-';
	} else {
		string::size_type i = data.find_first_not_of("0123456789.");

		if (i != 0) {
			if (number_index > 3) return 'E';

			string buffer;
		
			// we have found some number
			if (i == string::npos) {
				buffer = data;
				i = data.size() + 1;
			} else
				buffer = data.substr(0, i);

			lyx_advance(data, i);

			if (isStrDbl(buffer)) {
				number[number_index] = strToDbl(buffer);
				++number_index;
				return 'n';
			} else return 'E';
		}
		
		i = data.find_first_not_of("abcdefghijklmnopqrstuvwxyz%");
		if (i != 0) {
			if (unit_index > 3) return 'E';

			string buffer;
		
			// we have found some alphabetical string
			if (i == string::npos) {
				buffer = data;
				i = data.size() + 1;
			} else
				buffer = data.substr(0, i);

			// possibly we have "mmplus" string or similar
			if (buffer.size() > 5 && (buffer.substr(2,4) == string("plus") || buffer.substr(2,5) == string("minus"))) {
				lyx_advance(data, 2);
				unit[unit_index] = unitFromString(buffer.substr(0, 2));
			} else {
				lyx_advance(data, i);
				unit[unit_index] = unitFromString(buffer);
			}

			if (unit[unit_index] != LyXLength::UNIT_NONE) {
				++unit_index;
				return 'u';
			} else return 'E';  // Error
		}
		return 'E';  // Error
	}
}


struct LaTeXLength {
	char const * pattern;
	int   plus_val_index, minus_val_index,
		plus_uni_index, minus_uni_index;
};


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
    if (unit < 0 || unit >= num_units)
	return 0;
    return unit_name[unit];
}


LyXLength::UNIT unitFromString (string const & data)
{
	int i = 0;
	while ((i < num_units) && (data != unit_name[i])) ++i;
	return static_cast<LyXLength::UNIT>(i);
}


bool isValidGlueLength (string const & data, LyXGlueLength * result)
{
	// This parser is table-driven.  First, it constructs a "pattern"
        // that describes the sequence of tokens in "data".  For example,
	// "n-nu" means: number, minus sign, number, unit.  As we go along,
        // numbers and units are stored into static arrays.  Then, "pattern"
        // is searched in the "table".  If it is found, the associated
        // table entries tell us which number and unit should go where
        // in the LyXLength structure.  Example: if "data" has the "pattern"
        // "nu+nu-nu", the associated table entries are "2, 3, 2, 3".  
	// That means, "plus_val" is the second number that was seen
	// in the input, "minus_val" is the third number, and "plus_uni"
	// and "minus_uni" are the second and third units, respectively.
	// ("val" and "uni" are always the first items seen in "data".)
        //     This is the most elegant solution I could find -- a straight-
        // forward approach leads to very long, tedious code that would be
        // much harder to understand and maintain. (AS)

	if (data.empty())
		return true;
	string buffer = frontStrip(data);

	// To make isValidGlueLength recognize negative values as
	// the first number this little hack is needed:
	short val_sign = 1; // positive as default
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
	
	int       pattern_index = 0, table_index = 0;
	char      pattern[20];

	number_index = unit_index = 1;  // entries at index 0 are sentinels

	// construct "pattern" from "data"
	while (!isEndOfData (buffer)) {
		if (pattern_index > 20) return false;
		pattern[pattern_index] = nextToken (buffer);
		if (pattern[pattern_index] == 'E') return false;
		++pattern_index;
	}
	pattern[pattern_index] = '\0';

	// search "pattern" in "table"
	table_index = 0;
	while (compare(pattern, table[table_index].pattern)) {
		++table_index;
		if (!*table[table_index].pattern) return false;
	}
	
	// Get the values from the appropriate places.  If an index
	// is zero, the corresponding array value is zero or UNIT_NONE,
	// so we needn't check this.
	if (result) {
		result->val = number[1] * val_sign;
		result->uni = unit[1];
		result->plus_val  = number[table[table_index].plus_val_index];
		result->minus_val = number[table[table_index].minus_val_index];
		result->plus_uni  = unit  [table[table_index].plus_uni_index];
		result->minus_uni = unit  [table[table_index].minus_uni_index];
	}
	return true;
}


bool isValidLength(string const & data, LyXLength * result)
{
	/// This is a trimmed down version of isValidGlueLength.
	/// The parser may seem overkill for lengths without 
	/// glue, but since we already have it, using it is
	/// easier than writing something from scratch.
	if (data.empty())
		return true;

        string   buffer(data);
	int       pattern_index = 0;
	char      pattern[3];

	// To make isValidLength recognize negative values
	// this little hack is needed:
	short val_sign = 1; // positive as default
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
	while (!isEndOfData (buffer)) {
		if (pattern_index > 2) return false;
		pattern[pattern_index] = nextToken (buffer);
		if (pattern[pattern_index] == 'E') return false;
		++pattern_index;
	}
	pattern[pattern_index] = '\0';

	// only the most basic pattern is accepted here
	if (compare(pattern, "nu") != 0) return false;		
	
	// It _was_ a correct length string.  
	// Store away the values we found.
	if (result) {
		result->val = number[1] * val_sign;
		result->uni = unit[1];
	}
	return true;
}


/// LyXLength class

LyXLength::LyXLength(string const & data)
{
	LyXLength tmp;
	
	if (!isValidLength (data, &tmp))
		return; // should raise an exception
	else {
		val = tmp.val;
		uni = tmp.uni;
	}
}


string const LyXLength::asString() const
{
	ostringstream buffer;
	buffer << val << unit_name[uni]; // setw?
	return buffer.str().c_str();
}


string const LyXLength::asLatexString() const
{
	ostringstream buffer;
	switch(uni) {
	case PW:
	case PE:
	    buffer << abs(static_cast<int>(val/100)) << "." << abs(static_cast<int>(val)%100) << "\\columnwidth";
	    break;
	case PP:
	    buffer << "." << abs(static_cast<int>(val/100)) << "." << abs(static_cast<int>(val)%100) << "\\pagewidth";
	    break;
	case PL:
	    buffer << "." << abs(static_cast<int>(val/100)) << "." << abs(static_cast<int>(val)%100) << "\\linewidth";
	    break;
	default:
	    buffer << val << unit_name[uni]; // setw?
	    break;
	}
	return buffer.str().c_str();
}


/*  LyXGlueLength class
 */

LyXGlueLength::LyXGlueLength (string const & data)
{
	LyXGlueLength tmp(0.0, PT);

	if (!isValidGlueLength (data, &tmp))
		return; // should raise an exception
	else {
		val = tmp.val;
		uni = tmp.uni;
		plus_val = tmp.plus_val;
		plus_uni = tmp.plus_uni;
		minus_val = tmp.minus_val;
		minus_uni = tmp.minus_uni;
	}
}


string const LyXGlueLength::asString() const
{
	ostringstream buffer;

	if (plus_val != 0.0)
		if (minus_val != 0.0)
			if ((uni == plus_uni) && (uni == minus_uni))
				if (plus_val == minus_val)
					buffer << val << "+-"
					       << plus_val << unit_name[uni];
				else
					buffer << val
					       << '+' << plus_val
					       << '-' << minus_val
					       << unit_name[uni];
			else
				if (plus_uni == minus_uni
				    && plus_val == minus_val)
					buffer << val << unit_name[uni]
					       << "+-" << plus_val
					       << unit_name[plus_uni];
	
				else
					buffer << val << unit_name[uni]
					       << '+' << plus_val
					       << unit_name[plus_uni]
					       << '-' << minus_val
					       << unit_name[minus_uni];
		else 
			if (uni == plus_uni)
				buffer << val << '+' << plus_val
				       << unit_name[uni];
			else
				buffer << val << unit_name[uni]
				       << '+' << plus_val
				       << unit_name[plus_uni];
	
	else
		if (minus_val != 0.0)
			if (uni == minus_uni)
				buffer << val << '-' << minus_val
				       << unit_name[uni];
	
			else
				buffer << val << unit_name[uni]
				       << '-' << minus_val
				       << unit_name[minus_uni];
		else
			buffer << val << unit_name[uni];

	return buffer.str().c_str();
}


string const LyXGlueLength::asLatexString() const
{
	ostringstream buffer;

	if (plus_val != 0.0)
		if (minus_val != 0.0)
			buffer << val << unit_name[uni]
			       << " plus "
			       << plus_val << unit_name[plus_uni]
			       << " minus "
			       << minus_val << unit_name[minus_uni];
		else
			buffer << val << unit_name[uni]
			       << " plus "
			       << plus_val << unit_name[plus_uni];
	else
		if (minus_val != 0.0)
			buffer << val << unit_name[uni]
			       << " minus "
			       << minus_val << unit_name[minus_uni];
		else
			buffer << val << unit_name[uni];

	return buffer.str().c_str();
}


/*  VSpace class
 */

VSpace::VSpace (string const & data)
	: kin (NONE), len (0.0, LyXLength::PT) 
{
	kp = false;
	if (data.empty())
		return;
	float   value;
	string input  = strip(data);

	int length = input.length();

	if (length > 1 && input[length-1] == '*') {
		kp = true;
		input.erase(length - 1);
	}

	if      (prefixIs (input, "defskip"))   kin = DEFSKIP;
	else if (prefixIs (input, "smallskip")) kin = SMALLSKIP;
	else if (prefixIs (input, "medskip"))   kin = MEDSKIP;
	else if (prefixIs (input, "bigskip"))   kin = BIGSKIP;
	else if (prefixIs (input, "vfill"))     kin = VFILL;
	else if (isValidGlueLength (input, &len))
		kin = LENGTH;
	else if (sscanf(input.c_str(), "%f", &value) == 1) {
		// This last one is for reading old .lyx files
		// without units in added_space_top/bottom.
		// Let unit default to centimeters here.
		kin = LENGTH;
		len = LyXGlueLength (value, LyXLength::CM);
	}
}


bool VSpace::operator==(VSpace const & other) const
{
	if (this->kin != other.kin)
		return false;
 
	if (this->kin != LENGTH)
		return this->kp == other.kp;
 
	if (!(this->len == other.len))
		return false;
 
	return this->kp == other.kp;
}


string const VSpace::asLyXCommand() const
{
        string result;

	switch (kin) {
	case NONE:      break;
	case DEFSKIP:   result = "defskip";      break;
	case SMALLSKIP: result = "smallskip";    break;
	case MEDSKIP:   result = "medskip";      break;
	case BIGSKIP:   result = "bigskip";      break;
	case VFILL:     result = "vfill";        break;
	case LENGTH:    result = len.asString(); break;
	}
	if (kp && (kin != NONE) && (kin != DEFSKIP))
		return result += '*';
	else
		return result;
}


string const VSpace::asLatexCommand(BufferParams const & params) const
{
	switch (kin) {
	case NONE:      return string();
	case DEFSKIP:   
		return params.getDefSkip().asLatexCommand(params);
	case SMALLSKIP: return kp ? "\\vspace*{\\smallskipamount}"
				: "\\smallskip{}";
	case MEDSKIP:   return kp ? "\\vspace*{\\medskipamount}"
				: "\\medskip{}";
	case BIGSKIP:   return kp ? "\\vspace*{\\bigskipamount}"
				: "\\bigskip{}";
	case VFILL:     return kp ? "\\vspace*{\\fill}"
				: "\\vfill{}";
	case LENGTH:    return kp ? "\\vspace*{" + len.asLatexString() + '}'
				: "\\vspace{" + len.asLatexString() + '}';
	}
	return string();  // should never be reached
}


int VSpace::inPixels(BufferView * bv) const
{
	// Height of a normal line in pixels (zoom factor considered)
	int height = bv->text->defaultHeight(); // [pixels]
	int skip = 0;
	int width = bv->workWidth();

	if (kin == DEFSKIP)
	    skip = bv->buffer()->params.getDefSkip().inPixels(bv);

	return inPixels(height, skip, width);
}

int VSpace::inPixels(int default_height, int default_skip, int default_width) const
{
	// Height of a normal line in pixels (zoom factor considered)
	int height = default_height; // [pixels]
	
	// Zoom factor specified by user in percent
	float const zoom = lyxrc.zoom / 100.0; // [percent]

	// DPI setting for monitor: pixels/inch
	float const dpi = lyxrc.dpi; // screen resolution [pixels/inch]

	// We want the result in pixels
	float result, value;

	switch (kin) {
	case NONE: return 0;

	case DEFSKIP:
		return default_skip;

		// This is how the skips are normally defined by
		// LateX.  But there should be some way to change
		// this per document.
	case SMALLSKIP: return height / 4;
	case MEDSKIP:   return height / 2;
	case BIGSKIP:   return height;
	case VFILL:     return 3 * height;
		// leave space for the vfill symbol
	case LENGTH:
		// Pixel values are scaled so that the ratio
		// between lengths and font sizes on the screen
		// is the same as on paper.

		// we don't care about sign of value, we
		// can't display negative anyway
		result = 0.0;
		value = len.value();
		short val_sign = value < 0.0 ? -1 : 1;
		
		switch (len.unit()) {
		case LyXLength::SP:
			// Scaled point: sp = 1/65536 pt
			result = zoom * dpi * value
				/ (72.27 * 65536); // 4736286.7
			break;
		case LyXLength::PT:
			// Point: 1 pt = 1/72.27 inch
			result = zoom * dpi * value
				/ 72.27; // 72.27
			break;
		case LyXLength::BP:
			// Big point: 1 bp = 1/72 inch
			result = zoom * dpi * value
				/ 72; // 72
			break;
		case LyXLength::DD:
			// Didot: 1157dd = 1238 pt?
			result = zoom * dpi * value
				/ (72.27 / (0.376 * 2.845)); // 67.559735
			break;
		case LyXLength::MM:
			// Millimeter: 1 mm = 1/25.4 inch
			result = zoom * dpi * value
				/ 25.4; // 25.4
			break;
		case LyXLength::PC:
			// Pica: 1 pc = 12 pt
			result = zoom * dpi * value
				/ (72.27 / 12); // 6.0225
			break;
		case LyXLength::CC:
			// Cicero: 1 cc = 12 dd
			result = zoom * dpi * value
				/ (72.27 / (12 * 0.376 * 2.845)); // 5.6299779
			break;
		case LyXLength::CM:
			// Centimeter: 1 cm = 1/2.54 inch
			result = zoom * dpi * value
				/ 2.54; // 2.54
			break;
		case LyXLength::IN:
			// Inch
			result = zoom * dpi * value;
			break;
		case LyXLength::EX:
			// Ex: The height of an "x"
			result = zoom * value * height / 2; // what to / width?
			break;
		case LyXLength::EM: // what to / width?
			// Em: The width of an "m"
			result = zoom * value * height / 2; // Why 2?
			break;
		case LyXLength::MU: // This is probably only allowed in
			// math mode
			result = zoom * value * height;
			break;
		case LyXLength::PW: // Always % of workarea
		case LyXLength::PE:
		case LyXLength::PP:
		case LyXLength::PL:
			result = value * default_width / 100;
			break;
		case LyXLength::UNIT_NONE:
			result = 0;  // this cannot happen
			break;  
		}
		return int (result * val_sign + 0.5);
	}
	return 0; // never reached
}
