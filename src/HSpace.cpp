/**
 * \file HSpace.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 * \author Uwe Stöhr
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "HSpace.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "support/gettext.h"
#include "Length.h"
#include "Text.h"

#include "support/lstrings.h"

#include "support/lassert.h"

using namespace std;
using namespace lyx::support;


namespace lyx {


HSpace::HSpace()
	: kind_(DEFAULT), len_()
{}


HSpace::HSpace(HSpaceKind k)
	: kind_(k), len_()
{}


HSpace::HSpace(Length const & l)
	: kind_(LENGTH), len_(l)
{}


HSpace::HSpace(GlueLength const & l)
	: kind_(LENGTH), len_(l)
{}


HSpace::HSpace(string const & data)
	: kind_(DEFAULT), len_()
{
	if (data.empty())
		return;

	string input = rtrim(data);

	if (prefixIs(input, "default"))
		kind_ = DEFAULT;
	else if (isValidGlueLength(input, &len_))
		kind_ = LENGTH;
}


bool HSpace::operator==(HSpace const & other) const
{
	if (kind_ != other.kind_)
		return false;
	if (len_ != other.len_)
		return false;
	return true;
}


string const HSpace::asLyXCommand() const
{
	string result;
	switch (kind_) {
	case DEFAULT:
		result = "default";
		break;
	case LENGTH:
		result = len_.asString();
		break;
	}
	return result;
}


string const HSpace::asLatexCommand() const
{
	switch (kind_) {
	case DEFAULT:
		return string();
	case LENGTH:
		return len_.asLatexString();
	default:
		LATTEST(false);
		// fall through in release mode
	}
	return string();
}


docstring const HSpace::asGUIName() const
{
	docstring result;
	switch (kind_) {
	case DEFAULT:
		result = _("Default");
		break;
	case LENGTH:
		result = from_ascii(len_.asString());
		break;
	}
	return result;
}


string HSpace::asHTMLLength() const 
{
	string result;
	switch (kind_) {
	case DEFAULT:
		// 30pt are LaTeX's default
		result = "30pt";
		break;
	case LENGTH: {
		Length tmp = len_.len();
		if (tmp.value() > 0)
			result = tmp.asHTMLString();
		break;
		}
	}
	return result;
}


int HSpace::inPixels(BufferView const & bv) const
{
	switch (kind_) {
	case DEFAULT:
		// FIXME: replace by correct length
		return bv.buffer().params().getIndentation().inPixels(bv);
	case LENGTH:
		return len_.len().inPixels(bv.workWidth());
	default:
		LATTEST(false);
		// fall through in release mode
	}
	return 0;
}


} // namespace lyx
