// -*- C++ -*-
/**
 * \file Author.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef AUTHOR_H
#define AUTHOR_H

#include "support/docstring.h"

#include <map>
#include <iosfwd>


namespace lyx {


class Author {
public:
	Author() {}

	Author(docstring const & name, docstring const & email)
		: name_(name), email_(email), used_(true) {}

	docstring const name() const {
		return name_;
	}

	docstring const email() const {
		return email_;
	}

	void used(bool u) const {
		used_ = u;
	}

	bool used() const {
		return used_;
	}

	friend std::istream & operator>>(std::istream & os, Author & a);

private:
	docstring name_;

	docstring email_;

	mutable bool used_;
};


class AuthorList {
public:
	AuthorList();

	int record(Author const & a);

	void record(int id, Author const & a);

	Author const & get(int id) const;

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


} // namespace lyx

#endif // AUTHOR_H
