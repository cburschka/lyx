/**
 * \file InsetFlex.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Martin Vermeer
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetFlex.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "Lexer.h"

#include "support/gettext.h"

#include <ostream>

//#include "support/debug.h"
using namespace std;

namespace lyx {


InsetFlex::InsetFlex(Buffer const & buf, string const & layoutName)
	: InsetCollapsable(buf), name_(layoutName)
{
	// again, because now the name is initialized
	setLayout(buf.params().documentClassPtr());
	status_= Collapsed;
}


InsetFlex::InsetFlex(InsetFlex const & in)
	: InsetCollapsable(in), name_(in.name_)
{}


docstring InsetFlex::editMessage() const
{
	return _("Opened Flex Inset");
}


void InsetFlex::write(ostream & os) const
{
	os << "Flex " <<
		(name_.empty() ? "undefined" : name_) << "\n";
	InsetCollapsable::write(os);
}


} // namespace lyx
