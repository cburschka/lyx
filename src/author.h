/**
 * \file author.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef AUTHOR_H
#define AUTHOR_H

#include <map>
#include <iosfwd>

#include "LString.h"

class Author {
public:
	Author() {}

	Author(string const & name, string const & email)
		: name_(name), email_(email) {}

	string const name() const {
		return name_;
	}

	string const email() const {
		return email_;
	}

	friend  std::istream & operator>>(std::istream & os, Author & a);

private:
	string name_;

	string email_;
};


class AuthorList {
public:
	AuthorList();

	int record(Author const & a);

	void record(int id, Author const & a);

	Author const & get(int id);

	typedef std::map<int, Author> Authors;

	Authors::const_iterator begin() const;

	Authors::const_iterator end() const;

private:
	int last_id_;

	Authors authors_;
};

bool operator==(Author const & l, Author const & r);

std::ostream & operator<<(std::ostream & os, Author const & a);

std::istream & operator>>(std::istream & os, Author & a);

#endif // AUTHOR_H
