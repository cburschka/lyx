// -*- C++ -*-

#ifndef ERRORLIST_H
#define ERRORLIST_H

/**
 * \file errorlist.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alfredo Braunstein
 *
 * Full author contact details are available in file CREDITS
 */

#include "LString.h"

#include <vector>

class Buffer;
class TeXErrors;

/// A class to hold an error item
struct ErrorItem {
	string error;
	string description;
	int par_id;
	int pos_start;
	int pos_end;
	ErrorItem(string const &, string const &, 
		  int, int, int);
	ErrorItem();
};

class ErrorList : private std::vector<ErrorItem>
{
public:	
	ErrorList() : std::vector<ErrorItem> () {};
	ErrorList(Buffer const & buf, TeXErrors const &);

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
