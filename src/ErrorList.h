// -*- C++ -*-
/**
 * \file ErrorList.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alfredo Braunstein
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef ERRORLIST_H
#define ERRORLIST_H

#include "TexRow.h"

#include "support/strfwd.h"

#include <vector>


namespace lyx {

class Buffer;

/// A class to hold an error item
class ErrorItem {
public:
	typedef TexRow::TextEntry TextEntry;
	docstring error;
	docstring description;
	// To generalise into RowEntries
	TextEntry start;
	TextEntry end;
	Buffer const * buffer;
	// With a start position and an end position
	ErrorItem(docstring const & error, docstring const & description,
	          TextEntry start, TextEntry end, Buffer const * buf = 0);
	// Error outside the document body
	ErrorItem(docstring const & error, docstring const & description,
	          Buffer const * buf = 0);
	ErrorItem();
};


class ErrorList : private std::vector<ErrorItem>
{
public:
	ErrorList() : std::vector<ErrorItem> () {}

	using std::vector<ErrorItem>::push_back;
	using std::vector<ErrorItem>::end;
	using std::vector<ErrorItem>::begin;
	using std::vector<ErrorItem>::operator[];
	using std::vector<ErrorItem>::size;
	using std::vector<ErrorItem>::clear;
	using std::vector<ErrorItem>::empty;
	using std::vector<ErrorItem>::const_iterator;
};


} // namespace lyx

#endif
