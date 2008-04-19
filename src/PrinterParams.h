// -*- C++ -*-
/**
 * \file PrinterParams.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Allan Rae
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef PRINTERPARAMS_H
#define PRINTERPARAMS_H

#include <string>


namespace lyx {

/**
  This class contains (or should contain) all the parameters required for
  printing a buffer.  Some work still needs to be done on this class and
  printing handling in general to make it nice and full-featured.
  The main things I'd like to add now is the ability to print a read-only
  document with different orientation, papersize or single/duplex state
  than the document's settings. ARRae 20000423
*/
class PrinterParams
{
public:
	///
	PrinterParams();

	///
	enum Target {
		///
		PRINTER,
		///
		FILE
	};

	/** Test that all the fields contain valid entries.  It's unlikely
	    that the internal code will get this wrong however new ports
	    and external scripts might drive the wrong values in.
	 */
	void testInvariant() const;

public:
	///
	Target target;
	///
	std::string printer_name;
	///
	std::string file_name;
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

};


} // namespace lyx

#endif
