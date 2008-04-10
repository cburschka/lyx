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

#include "support/assert.h"

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
	os << "\"" << to_utf8(a.name()) << "\" " << to_utf8(a.email());
	return os;
}

istream & operator>>(istream & is, Author & a)
{
	string s;
	getline(is, s);
	// FIXME UNICODE
	a.name_ = from_utf8(trim(token(s, '\"', 1)));
	a.email_ = from_utf8(trim(token(s, '\"', 2)));
	return is;
}


AuthorList::AuthorList()
	: last_id_(0)
{
}


int AuthorList::record(Author const & a)
{
	Authors::const_iterator it(authors_.begin());
	Authors::const_iterator itend(authors_.end());

	for (;  it != itend; ++it) {
		if (it->second == a)
			return it->first;
	}

	authors_[last_id_++] = a;
	return last_id_ - 1;
}


void AuthorList::record(int id, Author const & a)
{
	LASSERT(unsigned(id) < authors_.size(), /**/);

	authors_[id] = a;
}


Author const & AuthorList::get(int id) const
{
	Authors::const_iterator it(authors_.find(id));
	LASSERT(it != authors_.end(), /**/);
	return it->second;
}


AuthorList::Authors::const_iterator AuthorList::begin() const
{
	return authors_.begin();
}


AuthorList::Authors::const_iterator AuthorList::end() const
{
	return authors_.end();
}


} // namespace lyx
