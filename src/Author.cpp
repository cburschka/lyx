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

#include "support/convert.h"
#include "support/gettext.h"
#include "support/lassert.h"
#include "support/lstrings.h"

#include <algorithm>
#include <istream>

using namespace std;
using namespace lyx::support;

namespace lyx {

static int computeHash(docstring const & name,
	docstring const & email)
{
	string const full_author_string = to_utf8(name + email);
	// Bernstein's hash function
	unsigned int hash = 5381;
	for (unsigned int i = 0; i < full_author_string.length(); ++i)
		hash = ((hash << 5) + hash) + (unsigned int)(full_author_string[i]);
	return int(hash);
}


Author::Author(docstring const & name, docstring const & email)
	: name_(name), email_(email), used_(true),
	  buffer_id_(computeHash(name, email))
{}


Author::Author(int buffer_id)
	: name_(convert<docstring>(buffer_id)), email_(docstring()), used_(false),
	  buffer_id_(buffer_id)
{}


docstring Author::nameAndEmail() const
{
	if (email().empty())
		return name();
	else
		return bformat(_("%1$s[[name]] (%2$s[[email]])"), name(), email());
}


bool Author::valid() const
{
	//this cannot be equal if the buffer_id was produced by the hash function.
	return name_ != convert<docstring>(buffer_id_);
}


bool operator==(Author const & l, Author const & r)
{
	return l.name() == r.name() && l.email() == r.email();
}


ostream & operator<<(ostream & os, Author const & a)
{
	// FIXME UNICODE
	os << a.buffer_id_ << " \"" << to_utf8(a.name_) << "\"";
	if (!a.email_.empty())
		os << " " << to_utf8(a.email_);

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


bool author_smaller(Author const & lhs, Author const & rhs)
{
	return lhs.bufferId() < rhs.bufferId();
}


AuthorList::AuthorList()
{}


int AuthorList::record(Author const & a)
{
	bool const valid = a.valid();
	// If we record an author which equals the current
	// author, we copy the buffer_id, so that it will
	// keep the same id in the file.
	if (valid && !authors_.empty() && a == authors_[0])
		authors_[0].setBufferId(a.bufferId());

	Authors::const_iterator it = authors_.begin();
	Authors::const_iterator const beg = it;
	Authors::const_iterator const end = authors_.end();
	for (; it != end; ++it) {
		if (valid && *it == a)
			return it - beg;
		if (it->bufferId() == a.bufferId()) {
			int id = it - beg;
			if (!it->valid())
				// we need to handle the case of a valid author being registred
				// after an invalid one. For instance, because "buffer-reload"
				// does not clear the buffer's author list.
				record(id, a);
			return id;
		}
	}
	authors_.push_back(a);
	return authors_.size() - 1;
}


void AuthorList::record(int id, Author const & a)
{
	LBUFERR(unsigned(id) < authors_.size());
	authors_[id] = a;
}


void AuthorList::recordCurrentAuthor(Author const & a)
{
	// current author has id 0
	record(0, a);
}


Author const & AuthorList::get(int id) const
{
	LASSERT(id < (int)authors_.size() , return authors_[0]);
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


void AuthorList::sort()
{
	std::sort(authors_.begin(), authors_.end(), author_smaller);
}


ostream & operator<<(ostream & os, AuthorList const & a)
{
	// Copy the authorlist, because we don't want to sort the original
	AuthorList sorted = a;
	sorted.sort();

	AuthorList::Authors::const_iterator a_it = sorted.begin();
	AuthorList::Authors::const_iterator const a_end = sorted.end();

	for (; a_it != a_end; ++a_it) {
		if (a_it->used() && a_it->valid())
			os << "\\author " << *a_it << "\n";	
	}
	return os;
}


} // namespace lyx
