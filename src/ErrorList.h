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

#include "support/docstring.h"
#include "support/types.h"

#include <vector>
#include <string>


namespace lyx {

class Buffer;

/// A class to hold an error item
class ErrorItem {
public:
	docstring error;
	docstring description;
	int par_id;
	pos_type pos_start;
	pos_type pos_end;
	ErrorItem(docstring const & error, docstring const & description,
		  int parid, pos_type posstart, pos_type posend);
	ErrorItem();
};


class ErrorList : private std::vector<ErrorItem>
{
public:
	ErrorList() : std::vector<ErrorItem> () {};

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
