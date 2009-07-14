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

using namespace std;

namespace lyx {


InsetFlex::InsetFlex(Buffer const & buf, string const & layoutName)
	: InsetCollapsable(buf), name_(layoutName)
{
	status_= Collapsed;
}


InsetFlex::InsetFlex(InsetFlex const & in)
	: InsetCollapsable(in), name_(in.name_)
{}


InsetLayout::InsetDecoration InsetFlex::decoration() const
{
	InsetLayout::InsetDecoration const dec = getLayout().decoration();
	return dec == InsetLayout::DEFAULT ? InsetLayout::CONGLOMERATE : dec;
}


void InsetFlex::write(ostream & os) const
{
	os << "Flex " <<
		(name_.empty() ? "undefined" : name_) << "\n";
	InsetCollapsable::write(os);
}


} // namespace lyx
