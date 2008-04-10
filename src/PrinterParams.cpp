/**
 * \file PrinterParams.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Allan Rae
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "PrinterParams.h"

#include "support/lstrings.h"

#include "support/assert.h"


using namespace std;

namespace lyx {


PrinterParams::PrinterParams(Target t,
			     string const & pname,
			     string const & fname,
			     bool all,
			     unsigned int from,
			     unsigned int to,
			     bool odd,
			     bool even,
			     unsigned int copies,
			     bool sorted,
			     bool reverse)
	: target(t),
	  printer_name(pname),
	  file_name(fname),
	  all_pages(all),
	  from_page(from),
	  to_page(to),
	  odd_pages(odd),
	  even_pages(even),
	  count_copies(copies),
	  sorted_copies(sorted),
	  reverse_order(reverse)
{
	testInvariant();
}


PrinterParams::PrinterParams(PrinterParams const & pp)
	: target(pp.target),
	  printer_name(pp.printer_name),
	  file_name(pp.file_name),
	  all_pages(pp.all_pages),
	  from_page(pp.from_page),
	  to_page(pp.to_page),
	  odd_pages(pp.odd_pages),
	  even_pages(pp.even_pages),
	  count_copies(pp.count_copies),
	  sorted_copies(pp.sorted_copies),
	  reverse_order(pp.reverse_order)
{
	testInvariant();
}


void PrinterParams::testInvariant() const
{
#ifdef ENABLE_ASSERTIONS
	switch (target) {
	case PRINTER:
		//LASSERT(!printer_name.empty(), /**/);
		break;
	case FILE:
		LASSERT(!file_name.empty(), /**/);
		break;
	default:
		LASSERT(false, /**/);
		break;
	}
#endif
}


} // namespace lyx
