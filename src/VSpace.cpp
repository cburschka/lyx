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
#include "TextMetrics.h" // for defaultRowHeight()

#include "support/convert.h"
#include "support/Length.h"
#include "support/lstrings.h"

#include "support/lassert.h"

using namespace std;
using namespace lyx::support;


namespace lyx {

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
	else if (prefixIs(input, "halfline"))
		kind_ = HALFLINE;
	else if (prefixIs(input, "fullline"))
		kind_ = FULLLINE;
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
	case DEFSKIP:
		result = "defskip";
		break;
	case SMALLSKIP:
		result = "smallskip";
		break;
	case MEDSKIP:
		result = "medskip";
		break;
	case BIGSKIP:
		result = "bigskip";
		break;
	case HALFLINE:
		result = "halfline";
		break;
	case FULLLINE:
		result = "fullline";
		break;
	case VFILL:
		result = "vfill";
		break;
	case LENGTH:
		result = len_.asString();
		break;
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
	
	case HALFLINE:
		return keep_ ? "\\vspace*{.5\\baselineskip}" : "\\vspace{.5\\baselineskip}";

	case FULLLINE:
		return keep_ ? "\\vspace*{\\baselineskip}" : "\\vspace{\\baselineskip}";

	case VFILL:
		return keep_ ? "\\vspace*{\\fill}" : "\\vfill{}";

	case LENGTH:
		return keep_ ? "\\vspace*{" + len_.asLatexString() + '}'
			: "\\vspace{" + len_.asLatexString() + '}';

	default:
		LATTEST(false);
		// fall through in release mode
	}
	return string();
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
	case HALFLINE:
		result = _("Half line height");
		break;
	case FULLLINE:
		result = _("Line height");
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


string VSpace::asHTMLLength() const
{
	string result;
	switch (kind_) {
		case DEFSKIP:
			result = "2ex";
			break;
		case SMALLSKIP:
			result = "1ex";
			break;
		case MEDSKIP:
			result = "3ex";
			break;
		case BIGSKIP:
			result = "5ex";
			break;
		case HALFLINE:
			result = "0.6em";
			break;
		case FULLLINE:
			result = "1.2em";
			break;
		case LENGTH: {
			Length tmp = len_.len();
			if (tmp.value() > 0)
				result = tmp.asHTMLString();
		}
		case VFILL:
			break;
	}
	return result;
}


int VSpace::inPixels(BufferView const & bv) const
{
	// Height of a normal line in pixels (zoom factor considered)
	int const default_height = defaultRowHeight();

	switch (kind_) {

	case DEFSKIP:
		return bv.buffer().params().getDefSkip().inPixels(bv);

	// This is how the skips are normally defined by LaTeX.
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

	case HALFLINE:
		return default_height / 2;

	case FULLLINE:
		return default_height;

	case LENGTH:
		return bv.inPixels(len_.len());

	default:
		LATTEST(false);
		// fall through in release mode
	}
	return 0;
}


} // namespace lyx
