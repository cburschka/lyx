/**
 * \file Author.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Author.h"

#include "support/lstrings.h"

#include "support/lassert.h"

#include <algorithm>
#include <istream>

using namespace std;
using namespace lyx::support;

namespace lyx {


bool operator==(Author const & l, Author const & r)
{
	return l.name() == r.name() && l.email() == r.email();
}


ostream & operator<<(ostream & os, Author const & a)
{
	// FIXME UNICODE
	os << a.buffer_id() << " \"" << to_utf8(a.name())
			<< "\" " << to_utf8(a.email());
		
	return os;
}

istream & operator>>(istream & is, Author & a)
{
	string s;
	is >> a.buffer_id_;
	getline(is, s);
	// FIXME UNICODE
	a.name_ = from_utf8(trim(token(s, '\"', 1)));
	a.email_ = from_utf8(trim(token(s, '\"', 2)));
	return is;
}


bool author_smaller(Author const & lhs, Author const & rhs) {
	return lhs.buffer_id() < rhs.buffer_id();
}


AuthorList::AuthorList()
	: last_id_(0)
{
}


int AuthorList::record(Author const & a)
{
	Authors::const_iterator it(authors_.begin());
	Authors::const_iterator itend(authors_.end());

	for (int i = 0;  it != itend; ++it, ++i) {
		if (*it == a) {
			if (it->buffer_id() == 0)
				// The current author is internally represented as 
				// author 0, but it appears he has already an id.
				it->setBufferId(a.buffer_id());
			return i;
		}
	}
	authors_.push_back(a);
	return last_id_++;
}


void AuthorList::record(int id, Author const & a)
{
	LASSERT(unsigned(id) < authors_.size(), /**/);

	authors_[id] = a;
}


Author const & AuthorList::get(int id) const
{
	LASSERT(id < (int)authors_.size() , /**/);
	return authors_[id];
}


AuthorList::Authors::const_iterator AuthorList::begin() const
{
	return authors_.begin();
}


AuthorList::Authors::const_iterator AuthorList::end() const
{
	return authors_.end();
}


void AuthorList::sort() {
	std::sort(authors_.begin(), authors_.end(), author_smaller);
}


ostream & operator<<(ostream & os, AuthorList const & a) {
	// Copy the authorlist, because we don't want to sort the original
	AuthorList sorted = a;
	sorted.sort();

	AuthorList::Authors::const_iterator a_it = sorted.begin();
	AuthorList::Authors::const_iterator a_end = sorted.end();

	// Find the buffer id for the current author (internal id 0),
	// if he doesn't have a buffer_id yet.
	if (sorted.get(0).buffer_id() == 0) {
		unsigned int cur_id = 1;
		for (; a_it != a_end; ++a_it) {
			if (a_it->buffer_id() == cur_id)
				++cur_id;
			else if (a_it->buffer_id() > cur_id) {
				break;
			}
		}
		// Set the id in both the original authorlist, 
		// as in the copy.
		a.get(0).setBufferId(cur_id);
		sorted.get(0).setBufferId(cur_id);
		sorted.sort();
	}
	
	for (a_it = sorted.begin(); a_it != a_end; ++a_it) {
		if (a_it->used())
			os << "\\author " << *a_it << "\n";	
	}
	return os;
}


} // namespace lyx
