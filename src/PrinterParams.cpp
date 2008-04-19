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

#include "LyXRC.h"

#include "support/assert.h"
#include "support/lstrings.h"


namespace lyx {

PrinterParams::PrinterParams() 
{
	target = PRINTER;
	printer_name = lyxrc.printer;
	file_name = std::string();
	all_pages = true;
	from_page = 1;
	to_page = 0;
	odd_pages = true;
	even_pages = true;
	count_copies = 1;
	sorted_copies = false;
	reverse_order = false;

	testInvariant();
}


void PrinterParams::testInvariant() const
{
	switch (target) {
	case PRINTER:
		LASSERT(!printer_name.empty(), /**/);
		break;
	case FILE:
		LASSERT(!file_name.empty(), /**/);
		break;
	default:
		LASSERT(false, /**/);
		break;
	}
}


} // namespace lyx
