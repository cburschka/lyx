// -*- C++ -*-
/**
 * \file errorlist.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alfredo Braunstein
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef ERRORLIST_H
#define ERRORLIST_H

#include "support/types.h"

#include <vector>
#include <string>

class Buffer;

/// A class to hold an error item
struct ErrorItem {
	std::string error;
	std::string description;
	int par_id;
	lyx::pos_type pos_start;
	lyx::pos_type pos_end;
	ErrorItem(std::string const & error, std::string const & description,
		  int parid, lyx::pos_type posstart, lyx::pos_type posend);
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



#endif
