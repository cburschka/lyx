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
#pragma implementation
#endif

#include "lyxlength.h"
#include "lengthcommon.h"
#include "lyxrc.h"

#include "Lsstream.h"

#include <cstdlib>


LyXLength::LyXLength()
	: val_(0), unit_(LyXLength::PT)
{}


LyXLength::LyXLength(double v, LyXLength::UNIT u)
	: val_(v), unit_(u)
{}


LyXLength::LyXLength(string const & data)
	: val_(0), unit_(LyXLength::PT)
{
	LyXLength tmp;
	
	if (!isValidLength (data, &tmp))
		return; // should raise an exception

	val_  = tmp.val_;
	unit_ = tmp.unit_;
}


string const LyXLength::asString() const
{
	ostringstream buffer;
	buffer << val_ << unit_name[unit_]; // setw?
	return buffer.str().c_str();
}


string const LyXLength::asLatexString() const
{
	ostringstream buffer;
	switch (unit_) {
	case PW:
	    buffer << abs(static_cast<int>(val_/100)) << "."
		   << abs(static_cast<int>(val_)%100) << "\\textwidth";
	    break;
	case PE:
	    buffer << abs(static_cast<int>(val_/100)) << "."
		   << abs(static_cast<int>(val_)%100) << "\\columnwidth";
	    break;
	case PP:
	    buffer << abs(static_cast<int>(val_/100)) << "."
		   << abs(static_cast<int>(val_)%100) << "\\paperwidth";
	    break;
	case PL:
	    buffer << abs(static_cast<int>(val_/100)) << "."
		   << abs(static_cast<int>(val_)%100) << "\\linewidth";
	    break;
	default:
	    buffer << val_ << unit_name[unit_]; // setw?
	    break;
	}
	return buffer.str().c_str();
}


double LyXLength::value() const
{
	return val_;
}


LyXLength::UNIT LyXLength::unit() const
{
	return unit_;
}


void LyXLength::value(double v)
{
	val_ = v;
}


void LyXLength::unit(LyXLength::UNIT u)
{
	unit_ = u;
}


bool LyXLength::zero() const 
{
	return val_ == 0.0;
}


int LyXLength::inPixels(int default_width, int default_height) const
{
	// Zoom factor specified by user in percent
	double const zoom = lyxrc.zoom / 100.0; // [percent]

	// DPI setting for monitor: pixels/inch
	double const dpi = lyxrc.dpi; // screen resolution [pixels/inch]

	// Pixel values are scaled so that the ratio
	// between lengths and font sizes on the screen
	// is the same as on paper.

	// we don't care about sign of value, we
	// display negative space with text too
	double result = 0.0;
	int val_sign = val_ < 0.0 ? -1 : 1;
		
	switch (unit_) {
	case LyXLength::SP:
		// Scaled point: sp = 1/65536 pt
		result = zoom * dpi * val_
			/ (72.27 * 65536); // 4736286.7
		break;
	case LyXLength::PT:
		// Point: 1 pt = 1/72.27 inch
		result = zoom * dpi * val_
			/ 72.27; // 72.27
		break;
	case LyXLength::BP:
		// Big point: 1 bp = 1/72 inch
		result = zoom * dpi * val_
			/ 72; // 72
		break;
	case LyXLength::DD:
		// Didot: 1157dd = 1238 pt?
		result = zoom * dpi * val_
			/ (72.27 / (0.376 * 2.845)); // 67.559735
		break;
	case LyXLength::MM:
		// Millimeter: 1 mm = 1/25.4 inch
		result = zoom * dpi * val_
			/ 25.4; // 25.4
		break;
	case LyXLength::PC:
		// Pica: 1 pc = 12 pt
		result = zoom * dpi * val_
			/ (72.27 / 12); // 6.0225
		break;
	case LyXLength::CC:
		// Cicero: 1 cc = 12 dd
		result = zoom * dpi * val_
			/ (72.27 / (12 * 0.376 * 2.845)); // 5.6299779
		break;
	case LyXLength::CM:
		// Centimeter: 1 cm = 1/2.54 inch
		result = zoom * dpi * val_
			/ 2.54; // 2.54
		break;
	case LyXLength::IN:
		// Inch
		result = zoom * dpi * val_;
		break;
	case LyXLength::EX:
		// Ex: The height of an "x"
		result = zoom * val_ * default_height / 2; // what to / width?
		break;
	case LyXLength::EM: // what to / width?
		// Em: The width of an "m"
		result = zoom * val_ * default_height / 2; // Why 2?
		break;
	case LyXLength::MU: // This is probably only allowed in
		// math mode
		result = zoom * val_ * default_height;
		break;
	case LyXLength::PW: // Always % of workarea
	case LyXLength::PE:
	case LyXLength::PP:
	case LyXLength::PL:
		result = val_ * default_width / 100;
		break;
	case LyXLength::UNIT_NONE:
		result = 0;  // this cannot happen
		break;
	}
	return static_cast<int>(result * val_sign + 0.5);
}


bool operator==(LyXLength const & l1, LyXLength const & l2)
{
	return l1.value() == l2.value() && l1.unit() == l2.unit();
}


bool operator!=(LyXLength const & l1, LyXLength const & l2)
{
	return !(l1 == l2);
}

