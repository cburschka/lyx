// -*- C++ -*-
/* This file is part of
 * ======================================================
 *
 *	    LyX, The Document Processor
 *
 *	    Copyright 1995 Matthias Ettrich
 *	    Copyright 1995-2001 The LyX Team
 *
 *	    This file Copyright 1999-2001
 *	    Allan Rae
 *======================================================
 */

#ifndef PRINTERPARAMS_H
#define PRINTERPARAMS_H

#include "lyxrc.h"

#ifdef ENABLE_ASSERTIONS
#include "support/lstrings.h"
#include "support/LAssert.h"
#endif


/**
  This struct contains (or should contain) all the parameters required for
  printing a buffer.  Some work still needs to be done on this struct and
  printing handling in general to make it nice and full-featured.
  The main things I'd like to add now is the ability to print a read-only
  document with different orientation, papersize or single/duplex state
  than the document's settings. ARRae 20000423
*/
struct PrinterParams {
	///
	enum Target {
		///
		PRINTER,
		///
		FILE
	};
	///
	Target target;
	///
	string printer_name;
	///
	string file_name;
	///
	bool all_pages;
	/** Print a page range. Both from_page and to_page used to be strings
	    because they're actually easier to work with that way.  I've
	    switched to_page to be an int.  However, from_page will remain a
	    string because I want the from_page field to be able to be used as
	    a page range "1,3-5" and so on.
	    I've modified the invariant test to match. ARRae 20000518
	 */
	unsigned int from_page;
	///
	unsigned int to_page;
	///
	bool odd_pages;
	///
	bool even_pages;
	///
	unsigned int count_copies;
	///
	bool sorted_copies;
	///
	bool reverse_order;
	// The settings below should allow us to print any read-only doc in
	// whatever size/orientation we want it -- overriding the documents
	// settings.
	// Override the documents orientation
	// bool orientation;
	// Print n pages per physical sheet
	// unsigned int nup;
	// Override document settings for duplex.
	// bool duplex;

	/** Test that all the fields contain valid entries.  It's unlikely
	    that the internal code will get this wrong (at least for the
	    xforms code anyway) however new ports and external scripts
	    might drive the wrong values in.
	 */
	void testInvariant() const
		{
#ifdef ENABLE_ASSERTIONS
			switch (target) {
			case PRINTER:
				//Assert(!printer_name.empty());
				break;
			case FILE:
				lyx::Assert(!file_name.empty());
				break;
			default:
				lyx::Assert(false);
				break;
			}
#endif
		}

	///
	PrinterParams(Target const & t = PRINTER,
		      string const & pname = lyxrc.printer,
		      string const & fname = string(),
		      bool const all = true,
		      unsigned int const & from = 1,
		      unsigned int const & to = 1,
		      bool const odd = true,
		      bool const even = true,
		      unsigned int const & copies = 1,
		      bool const sorted = false,
		      bool const reverse = false)
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
	///
	PrinterParams(PrinterParams const & pp)
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
};

#endif
