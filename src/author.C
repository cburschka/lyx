/**
 * \file author.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "author.h"

#include "support/lstrings.h"

#include <boost/assert.hpp>

#include "support/std_istream.h"

using lyx::support::token;
using lyx::support::trim;


bool operator==(Author const & l, Author const & r)
{
	return l.name() == r.name() && l.email() == r.email();
}


std::ostream & operator<<(std::ostream & os, Author const & a)
{
	os << "\"" << a.name() << "\" " << a.email();
	return os;
}

std::istream & operator>>(std::istream & is, Author & a)
{
	string s;
	getline(is, s);
	a.name_ = trim(token(s, '\"', 1));
	a.email_ = trim(token(s, '\"', 2));
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
	BOOST_ASSERT(unsigned(id) < authors_.size());

	authors_[id] = a;
}


Author const & AuthorList::get(int id)
{
	Authors::const_iterator it(authors_.find(id));
	BOOST_ASSERT(it != authors_.end());
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
