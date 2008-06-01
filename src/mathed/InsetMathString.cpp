/**
 * \file InsetMathString.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathString.h"
#include "MathStream.h"
#include "MathSupport.h"

#include "Encoding.h"

#include "support/gettext.h"


namespace lyx {

InsetMathString::InsetMathString(docstring const & s)
	: str_(s)
{}


Inset * InsetMathString::clone() const
{
	return new InsetMathString(*this);
}


void InsetMathString::metrics(MetricsInfo & mi, Dimension & dim) const
{
	mathed_string_dim(mi.base.font, str_, dim);
}


void InsetMathString::draw(PainterInfo & pi, int x, int y) const
{
	pi.draw(x, y, str_);
}


void InsetMathString::normalize(NormalStream & os) const
{
	os << "[string " << str_ << ' ' << "mathalpha" << ']';
}


void InsetMathString::maple(MapleStream & os) const
{
	if (/*code_ != LM_TC_VAR ||*/ str_.size() <= 1) {
		os << ' ' << str_ << ' ';
		return;
	}

	// insert '*' between adjacent chars if type is LM_TC_VAR
	os << str_[0];
	for (size_t i = 1; i < str_.size(); ++i)
		os << str_[i];
}


void InsetMathString::mathematica(MathematicaStream & os) const
{
	os << ' ' << str_ << ' ';
}


void InsetMathString::octave(OctaveStream & os) const
{
	if (/*code_ != LM_TC_VAR ||*/ str_.size() <= 1) {
		os << ' ' << str_ << ' ';
		return;
	}

	// insert '*' between adjacent chars if type is LM_TC_VAR
	os << str_[0];
	for (size_t i = 1; i < str_.size(); ++i)
		os << str_[i];
}


void InsetMathString::mathmlize(MathStream & os) const
{
/*
	if (code_ == LM_TC_VAR)
		os << "<mi> " << str_ << " </mi>";
	else if (code_ == LM_TC_CONST)
		os << "<mn> " << str_ << " </mn>";
	else if (code_ == LM_TC_RM || code_ == LM_TC_TEXTRM)
		os << "<mtext> " << str_ <<  " </mtext>";
	else
*/
		os << str_;
}


void InsetMathString::write(WriteStream & os) const
{
	if (!os.latex()) {
		os << str_;
		return;
	}

	// We need the latex macros defined in the unicodesymbols file.
	// As they do not depend on the encoding, simply use the first
	// available encoding.
	Encodings::const_iterator encit = encodings.begin();
	bool can_encode = encit != encodings.end();

	docstring::const_iterator cit = str_.begin();
	docstring::const_iterator end = str_.end();

	bool in_lyxmathsym = false;
	while (cit != end) {
		char_type const c = *cit;
		try {
			if (c < 0x80) {
				if (in_lyxmathsym) {
					os << '}';
					in_lyxmathsym = false;
				}
				os << docstring(1, c);
			} else if (can_encode) {
				if (!in_lyxmathsym) {
					os << "\\lyxmathsym{";
					in_lyxmathsym = true;
				}
				os << encit->latexChar(c, true);
			} else {
				throw EncodingException(c);
			}
		} catch (EncodingException & e) {
			if (os.dryrun()) {
				// FIXME: this is OK for View->Source
				// but math preview will likely fail.
				os << "<" << _("LyX Warning: ")
				   << _("uncodable character") << " '";
				os << docstring(1, e.failed_char);
				os << "'>";
			} else {
				// throw again
				throw(e);
			}
		}
		++cit;
	}
	if (in_lyxmathsym)
		os << '}';
}


} // namespace lyx
