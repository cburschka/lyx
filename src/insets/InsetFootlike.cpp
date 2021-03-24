/**
 * \file InsetFootlike.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetFootlike.h"

#include "Buffer.h"
#include "BufferView.h"
#include "BufferParams.h"
#include "Font.h"
#include "MetricsInfo.h"

#include "support/lstrings.h"

#include <iostream>

using namespace std;

namespace lyx {

using support::token;

InsetFootlike::InsetFootlike(Buffer * buf)
	: InsetCollapsible(buf)
{}


void InsetFootlike::write(ostream & os) const
{
	// The layoutName may contain a "InTitle" qualifier
	os << to_utf8(token(layoutName(), char_type(':'), 0)) << "\n";
	InsetCollapsible::write(os);
}


bool InsetFootlike::insetAllowed(InsetCode code) const
{
	if (code == FOOT_CODE || code == MARGIN_CODE || code == FLOAT_CODE)
		return false;
	return InsetCollapsible::insetAllowed(code);
}


} // namespace lyx
