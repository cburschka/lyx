// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *	    LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *	    Copyright 1995-2000 The LyX Team
 *
 *	    This file Copyright 1999-2000
 *	    Allan Rae
 *======================================================
 */

#ifndef PRINTERPARAMS_H
#define PRINTERPARAMS_H

#ifndef LYXRC_H
#error You must include lyxrc.h before PrinterParams.h
#endif

#ifdef ENABLE_ASSERTIONS
#include "support/LAssert.h"
extern bool containsOnly(string const &, char const *);
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
	/// We allow printing of even pages in a range and so on.
	enum WhichPages{
		///
		ALL,
		///
		ODD,
		///
		EVEN
	};
	///
	WhichPages which_pages;
	/** Print a page range. Both from_page and to_page used to be strings
	    because they're actually easier to work with that way.  I've
	    switched to_page to be an int.  However, from_page will remain a
	    string because I want the from_page field to be able to be used as
	    a page range "1,3-5" and so on.
	    I've modified the invariant test to match. ARRae 20000518
	 */
	string from_page;
	///
	int to_page;
	///
	bool reverse_order;
	///
	bool unsorted_copies;
	///
  	int count_copies;
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
			if (!from_page.empty()) {
				// Assert(from_page == number or empty)
				lyx::Assert(containsOnly(from_page,
							 "1234567890"));
			}
			if (to_page) {
				// Assert(to_page == empty
				//        or number iff from_page set)
				lyx::Assert(!from_page.empty());
			}
			switch (target) {
			case PRINTER:
//  				Assert(!printer_name.empty());
				break;
			case FILE:
				lyx::Assert(!file_name.empty());
				break;
			default:
				lyx::Assert(false);
				break;
			}
			switch (which_pages) {
			case ALL:
			case ODD:
			case EVEN:
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
		      WhichPages const wp = ALL,
		      string const & from = string(),
		      int const & to = 0,
		      bool const reversed = false,
		      bool const unsorted = false,
		      int const & num_copies = 1)
		: target(t),
		  printer_name(pname),
		  file_name(fname),
		  which_pages(wp),
		  from_page(from),
		  to_page(to),
		  reverse_order(reversed),
		  unsorted_copies(unsorted),
		  count_copies(num_copies)
		{
			testInvariant();
		}
	///
	PrinterParams(PrinterParams const & pp)
		: target(pp.target),
		  printer_name(pp.printer_name),
		  file_name(pp.file_name),
		  which_pages(pp.which_pages),
		  from_page(pp.from_page),
		  to_page(pp.to_page),
		  reverse_order(pp.reverse_order),
		  unsorted_copies(pp.unsorted_copies),
		  count_copies(pp.count_copies)
		{
			testInvariant();
		}
};

#endif
