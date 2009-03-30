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
#include "support/lstrings.h"
#include "support/textutils.h"


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

	docstring::const_iterator cit = str_.begin();
	docstring::const_iterator end = str_.end();

	// We may already be inside an \ensuremath command.
	bool in_forced_mode = os.pendingBrace();

	// We will take care of matching braces.
	os.pendingBrace(false);

	while (cit != end) {
		bool mathmode = in_forced_mode ? os.textMode() : !os.textMode();
		char_type const c = *cit;
		docstring command(1, c);
		try {
			if (c < 0x80 || Encodings::latexMathChar(c, mathmode, os.encoding(), command)) {
				if (os.textMode()) {
					if (in_forced_mode) {
						// we were inside \lyxmathsym
						os << '}';
						os.textMode(false);
						in_forced_mode = false;
					}
					if (c >= 0x80 && os.textMode()) {
						os << "\\ensuremath{";
						os.textMode(false);
						in_forced_mode = true;
					}
				} else if (c < 0x80 && in_forced_mode) {
					// we were inside \ensuremath
					os << '}';
					os.textMode(true);
					in_forced_mode = false;
				}
			} else if (!os.textMode()) {
					if (in_forced_mode) {
						// we were inside \ensuremath
						os << '}';
						in_forced_mode = false;
					} else {
						os << "\\lyxmathsym{";
						in_forced_mode = true;
					}
					os.textMode(true);
			}
			os << command;
			// We may need a space if the command contains a macro
			// and the last char is ASCII.
			if (lyx::support::contains(command, '\\')
			    && isAlphaASCII(command[command.size() - 1]))
				os.pendingSpace(true);
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

	if (in_forced_mode && os.textMode()) {
		// We have to care for closing \lyxmathsym
		os << '}';
		os.textMode(false);
	} else {
		os.pendingBrace(in_forced_mode);
	}
}


} // namespace lyx
