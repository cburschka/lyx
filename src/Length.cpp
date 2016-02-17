/**
 * \file Length.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Matthias Ettrich
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author Angus Leeming
 * \author John Levon
 * \author Dekel Tsur
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Length.h"
#include "LyXRC.h"
#include "MetricsInfo.h"

#include "frontends/FontMetrics.h"

#include "support/docstream.h"
#include "support/lstrings.h"
#include "support/lyxlib.h"

#include <sstream>
#include <iomanip>

using namespace std;
using namespace lyx::support;

namespace lyx {


/////////////////////////////////////////////////////////////////////
//
// Length
//
/////////////////////////////////////////////////////////////////////

Length::Length()
	: val_(0), unit_(Length::UNIT_NONE)
{}


Length::Length(double v, Length::UNIT u)
	: val_(v), unit_(u)
{}


Length::Length(string const & data)
	: val_(0), unit_(Length::PT)
{
	Length tmp;

	if (!isValidLength(data, &tmp))
		return; // should raise an exception

	val_  = tmp.val_;
	unit_ = tmp.unit_;
}


string const Length::asString() const
{
	ostringstream os;
	if (unit_ != UNIT_NONE)
		os << formatFPNumber(val_) << unit_name[unit_]; // setw?
	return os.str();
}


docstring const Length::asDocstring() const
{
	odocstringstream os;
	if (unit_ != UNIT_NONE)
		os << from_ascii(formatFPNumber(val_))
		   << from_ascii(unit_name[unit_]); // setw?
	return os.str();
}


string const Length::asLatexString() const
{
	ostringstream os;
	// Do not allow scientific notation (e.g. 1.2e+03), since this is not valid
	// LaTeX (bug 9416)
	switch (unit_) {
	case PTW:
		os << formatFPNumber(val_ / 100.0) << "\\textwidth";
		break;
	case PCW:
		os << formatFPNumber(val_ / 100.0) << "\\columnwidth";
		break;
	case PPW:
		os << formatFPNumber(val_ / 100.0) << "\\paperwidth";
		break;
	case PLW:
		os << formatFPNumber(val_ / 100.0) << "\\linewidth";
		break;
	case PTH:
		os << formatFPNumber(val_ / 100.0) << "\\textheight";
		break;
	case PPH:
		os << formatFPNumber(val_ / 100.0) << "\\paperheight";
		break;
	case UNIT_NONE:
		break;
	default:
		os << formatFPNumber(val_) << unit_name[unit_];
	  break;
	}
	return os.str();
}


string const Length::asHTMLString() const
{
	ostringstream os;
	switch (unit_) {
	case PT:
 	case BP:
	case DD:
		// close enough
		os << formatFPNumber(val_) << "pt";
		break;
 	case MM:
	case CM:
	case PC:
	case IN:
	case EX:
	case EM:
		os << formatFPNumber(val_) << unit_name[unit_];
		break;
	case CC:
		os << formatFPNumber(val_ / 12.0) << "pt";
		break;
 	case MU:
		os << formatFPNumber(val_ / 18.0) << "em";
		break;
	case PTW:
	case PPW:
	case PLW:
	case PCW:
	case PTH:
	case PPH:
		// what it's a percentage of probably won't make sense for HTML,
		// so we'll assume people have chosen these appropriately
		os << formatFPNumber(val_) << '%';
		break;
	case SP:
	case UNIT_NONE:
		break;
	}
	return os.str();
}


double Length::value() const
{
	return val_;
}


Length::UNIT Length::unit() const
{
	return unit_;
}


void Length::value(double v)
{
	val_ = v;
}


void Length::unit(Length::UNIT u)
{
	unit_ = u;
}


bool Length::zero() const
{
	return val_ == 0.0;
}


bool Length::empty() const
{
	return unit_ == Length::UNIT_NONE;
}


int Length::inPixels(int text_width, int em_width_base) const
{
	// Zoom factor specified by user in percent
	double const zoom = lyxrc.zoom / 100.0; // [percent]

	// DPI setting for monitor: pixels/inch
	double const dpi = lyxrc.dpi; // screen resolution [pixels/inch]

	double const em_width_in = (em_width_base > 0)
		? em_width_base / (zoom * dpi)
		: 10.0/72.27;
	// A different estimate for em_width is
	// theFontMetrics(FontInfo(sane_font)).em()
	// but this estimate might not be more accurate as the screen font
	// is different then the latex font.

	// Pixel values are scaled so that the ratio
	// between lengths and font sizes on the screen
	// is the same as on paper.

	double const text_width_in = text_width / (zoom * dpi);
	double const result = zoom * dpi * inInch(text_width_in, em_width_in);
	return support::iround(result);
}


double Length::inInch(double text_width, double em_width) const
{
	double result = 0.0;

	switch (unit_) {
	case Length::SP:
		// Scaled point: sp = 1/65536 pt
		result = val_ / (72.27 * 65536); // 4736286.7
		break;
	case Length::PT:
		// Point: 1 pt = 1/72.27 inch
		result = val_ / 72.27; // 72.27
		break;
	case Length::BP:
		// Big point: 1 bp = 1/72 inch
		result = val_ / 72; // 72
		break;
	case Length::DD:
		// Didot: 1157dd = 1238 pt?
		result = val_ / (72.27 / (0.376 * 2.845)); // 67.559735
		break;
	case Length::MM:
		// Millimeter: 1 mm = 1/25.4 inch
		result = val_ / 25.4; // 25.4
		break;
	case Length::PC:
		// Pica: 1 pc = 12 pt
		result = val_ / (72.27 / 12); // 6.0225
		break;
	case Length::CC:
		// Cicero: 1 cc = 12 dd
		result = val_
			/ (72.27 / (12 * 0.376 * 2.845)); // 5.6299779
		break;
	case Length::CM:
		// Centimeter: 1 cm = 1/2.54 inch
		result = val_ / 2.54; // 2.54
		break;
	case Length::IN:
		// Inch
		result = val_;
		break;
	case Length::EX:
		// Ex: The height of an "x"
		// 0.4305 is the ration between 1ex and 1em in cmr10
		result = val_ * em_width * 0.4305;
		break;
	case Length::EM:
		// Em: The width of an "m"
		result = val_ * em_width;
		break;
	case Length::MU:
		// math unit = 1/18em
		result = val_ * em_width / 18;
		break;
	case Length::PCW: // Always % of workarea
	case Length::PTW:
	case Length::PLW:
		result = val_ * text_width / 100;
		break;
	case Length::PPW:
		// paperwidth/textwidth is 1.7 for A4 paper with default margins
		result = val_ * text_width * 1.7 / 100;
		break;
	case Length::PTH:
		result = val_ * text_width * 1.787 / 100;
		break;
	case Length::PPH:
		result = val_ * text_width * 2.2 / 100;
		break;
	case Length::UNIT_NONE:
		result = 0;  // this cannot happen
		break;
	}
	return result;
}


int Length::inPixels(MetricsBase const & base) const
{
	return inPixels(base.textwidth, theFontMetrics(base.font).em());
}


int Length::inBP() const
{
	// return any Length value as a one with
	// the PostScript point, called bp (big points)

	double const text_width_in = 210.0 / 2.54; // assume A4
	double const em_width_in = 10.0 / 72.27;
	double result = 72.0 * inInch(text_width_in, em_width_in);
	return support::iround(result);
}


Length::UNIT Length::defaultUnit()
{
	return lyxrc.default_length_unit;
}



bool operator==(Length const & l1, Length const & l2)
{
	return l1.value() == l2.value() && l1.unit() == l2.unit();
}


bool operator!=(Length const & l1, Length const & l2)
{
	return !(l1 == l2);
}


/////////////////////////////////////////////////////////////////////
//
// GlueLength
//
/////////////////////////////////////////////////////////////////////


GlueLength::GlueLength(Length const & len)
	: len_(len)
{}


GlueLength::GlueLength(Length const & len, Length const & plus,
		Length const & minus)
	: len_(len), plus_(plus), minus_(minus)
{}


GlueLength::GlueLength(string const & data)
{
	isValidGlueLength(data, this);
}


string const GlueLength::asString() const
{
	if (len_.empty())
		return string();

	ostringstream buffer;

	buffer << formatFPNumber(len_.value());

	if (plus_.zero() && minus_.zero()) {
		buffer << unit_name[len_.unit()];
		return buffer.str();
	}

	// just len and plus
	if (minus_.zero()) {
		if (len_.unit() != plus_.unit())
			buffer << unit_name[len_.unit()];
		buffer << '+' << formatFPNumber(plus_.value());
		buffer << unit_name[plus_.unit()];
		return buffer.str();
	}

	// just len and minus
	if (plus_.zero()) {
		if (len_.unit() != minus_.unit())
			buffer << unit_name[len_.unit()];
		buffer << '-' << formatFPNumber(minus_.value());
		buffer << unit_name[minus_.unit()];
		return buffer.str();
	}

	// ok, len, plus AND minus

	// len+-
	if (minus_ == plus_) {
		if (len_.unit() != minus_.unit())
			buffer << unit_name[len_.unit()];
		buffer << "+-" << formatFPNumber(minus_.value());
		buffer << unit_name[minus_.unit()];
		return buffer.str();
	}

	// this is so rare a case, why bother minimising units ?

	buffer << unit_name[len_.unit()];
	buffer << '+' << formatFPNumber(plus_.value()) << unit_name[plus_.unit()];
	buffer << '-' << formatFPNumber(minus_.value()) << unit_name[minus_.unit()];

	return buffer.str();
}


string const GlueLength::asLatexString() const
{
	ostringstream buffer;
	// use Length::asLatexString() to handle also the percent lengths
	buffer << len_.Length::asLatexString();
	if (!plus_.zero())
		buffer << " plus " << plus_.Length::asLatexString();
	if (!minus_.zero())
		buffer << " minus " << minus_.Length::asLatexString();
	return buffer.str();
}


Length const & GlueLength::len() const
{
	return len_;
}


Length const & GlueLength::plus() const
{
	return plus_;
}


Length const & GlueLength::minus() const
{
	return minus_;
}


bool operator==(GlueLength const & l1, GlueLength const & l2)
{
	return l1.len() == l2.len()
		 && l1.plus() == l2.plus()
		 && l1.minus() == l2.minus();
}


bool operator!=(GlueLength const & l1, GlueLength const & l2)
{
	return !(l1 == l2);
}


} // namespace lyx
